
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <libintl.h>
#define _(String) gettext (String)

#include <gtk/gtk.h>

#include "config.h"
#include "setupdb.h"
#include "setup-xml.h"
#include "uninstall.h"
#include "uninstall_ui.h"

#define UNINSTALL_GLADE "uninstall.gtk3.ui"

static GtkBuilder *uninstall_builder = NULL;

static int uninstall_cancelled = 0;

/* GTK utility function */
void gtk_button_set_sensitive(GtkWidget *button, gboolean sensitive)
{
    // Set widget sensitivity
    gtk_widget_set_sensitive(button, sensitive);

    // Simulate a mouse crossing event if button becomes sensitive
    if (sensitive) {
        int x, y;
        gboolean retval;
        GdkEventCrossing crossing;
        GtkAllocation allocation;

        // Get pointer position relative to the widget and its allocation
        gtk_widget_get_allocation(button, &allocation);
        gdk_window_get_device_position(gtk_widget_get_window(button), NULL, &x, &y, NULL);

        // Check if the pointer is inside the widget area
        if (x >= 0 && y >= 0 && x <= allocation.width && y <= allocation.height) {
            memset(&crossing, 0, sizeof(crossing));
            crossing.type = GDK_ENTER_NOTIFY;
            crossing.window = gtk_widget_get_window(button);
            crossing.detail = GDK_NOTIFY_VIRTUAL;
            crossing.mode = GDK_CROSSING_NORMAL;

            // Emit the enter_notify_event signal
            g_signal_emit_by_name(button, "enter_notify_event", &crossing, &retval);
        }
    }
}

static int dialog_done;

#define BUTTON_OK    1
#define BUTTON_ABORT 2

static void prompt_okbutton_slot( GtkWidget* widget, gpointer func_data)
{
    dialog_done = 1;
}

static void prompt_nobutton_slot( GtkWidget* widget, gpointer func_data)
{
    dialog_done = 2;
}

static int display_message(const char *txt, int buttons)
{
    GtkWidget *dialog, *label, *ok_button, *abort_button;
       
    /* Create the widgets */
	dialog_done = 0;
    
    dialog = gtk_dialog_new();
    label = gtk_label_new (txt);
	if ( buttons & BUTTON_OK ) {
		ok_button = gtk_button_new_with_label(_("OK"));
		gtk_signal_connect_object (G_OBJECT (ok_button), "clicked", G_CALLBACK (prompt_okbutton_slot), G_OBJECT(dialog));
                GtkWidget *action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
                gtk_container_add(GTK_CONTAINER(action_area), ok_button);
	}
	if ( buttons & BUTTON_ABORT ) {
		abort_button = gtk_button_new_with_label(_("Abort"));
		gtk_signal_connect_object (G_OBJECT (abort_button), "clicked",G_CALLBACK (prompt_nobutton_slot), G_OBJECT(dialog));
                GtkWidget *action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
                gtk_container_add(GTK_CONTAINER(action_area), dialog);
	}

    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    
	gtk_signal_connect_object(G_OBJECT(dialog), "delete-event",
							  G_CALLBACK(prompt_nobutton_slot), G_OBJECT(dialog));
    
    /* Add the label, and show everything we've added to the dialog. */
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), label);

    gtk_window_set_title(GTK_WINDOW(dialog), _("Message"));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);

    while ( ! dialog_done ) {
        gtk_main_iteration();
    }
    gtk_widget_destroy(dialog);
	return  dialog_done == 1;
}

/* List of open products that need closing */
struct product_list {
    product_t *product;
    struct product_list *next;
} *product_list = NULL;

static void add_product(product_t *product)
{
    struct product_list *entry;

    entry = (struct product_list *)malloc(sizeof *entry);
    if ( entry ) {
        entry->product = product;
        entry->next = product_list;
        product_list = entry;
    }
}

static void remove_product(product_t *product)
{
    struct product_list *prev, *entry;

    prev = NULL;
    for ( entry = product_list; entry; entry = entry->next ) {
        if ( entry->product == product ) {
            if ( prev ) {
                prev->next = entry->next;
            } else {
                product_list = entry->next;
            }
            free(entry);
            break;
        }
    }
}

static void close_products(int status)
{
    struct product_list *freeable;

    while ( product_list ) {
        freeable = product_list;
        product_list = product_list->next;
        loki_closeproduct(freeable->product);
        free(freeable);
    }
}

/* List of components and associated widgets */
typedef struct {
    product_t *product;
    product_info_t *info;
    product_component_t *component;
    size_t size;
    struct component_button {
        GtkWidget *widget;
        struct component_button *next;
    } *buttons;
} component_list;

static component_list *create_component_list(product_t *product,
                                             product_info_t *info,
                                             product_component_t *component)
{
    component_list *list;

    list = (component_list *)malloc(sizeof *list);
    if ( list ) {
        list->product = product;
        list->info = info;
        list->component = component;
        list->size = loki_getsize_component(component);
        list->buttons = NULL;
    }
    return(list);
}

static void add_component_list(component_list *list, GtkWidget *widget)
{
    struct component_button *entry;

    if ( list ) {
        entry = (struct component_button *)malloc(sizeof *entry);
        if ( entry ) {
            entry->widget = widget;
            entry->next = list->buttons;
            list->buttons = entry;
        }
    }
}

static size_t calculate_recovered_space(void)
{
    GtkWidget *widget;
    GList *list, *poopy, *clist;
    GtkWidget *button;
    component_list *component;
    gboolean ready;
    size_t size;

    ready = FALSE;
    size = 0;

    // Retrieve uninstall_vbox using GtkBuilder
    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_vbox"));
    list = gtk_container_get_children(GTK_CONTAINER(widget));
    
    while (list) {
        widget = GTK_WIDGET(list->data);
        poopy = gtk_container_get_children(GTK_CONTAINER(widget));
        widget = GTK_WIDGET(poopy->data);
        clist = gtk_container_get_children(GTK_CONTAINER(widget));
        
        while (clist) {
            button = GTK_WIDGET(clist->data);
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                component = g_object_get_data(G_OBJECT(button), "data");
                size += component->size / 1024;
                ready = TRUE;
            }
            clist = clist->next;
        }
        list = list->next;
    }

    // Update the recovered_space_label
    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "recovered_space_label"));
    if (widget) {
        char text[128];
        sprintf(text, _("%zu MB"), size / 1024);  // %zu for size_t
        gtk_label_set_text(GTK_LABEL(widget), text);
    }

    // Update the uninstall_button
    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_button"));
    if (widget) {
        gtk_widget_set_sensitive(widget, ready);
    }

    return size;
}

void main_quit_slot(GtkWidget* w, gpointer data)
{
    uninstall_cancelled = 1;
    gtk_main_quit();
}

void main_signal_abort(int status)
{
    main_quit_slot(NULL, NULL);
}

void cancel_uninstall_slot(GtkWidget *w, gpointer data)
{
    uninstall_cancelled = 1;
}

static void set_status_text(const char *text)
{
    GtkWidget *widget;

    // Retrieve the label for status update
    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_status_label"));
    if (widget) {
        gtk_label_set_text(GTK_LABEL(widget), text);
    }
}

void perform_uninstall_slot(GtkWidget* w, gpointer data)
{
    GtkWidget *notebook;
    GtkWidget *progress;
    GtkWidget *widget;
    GList *list, *poopy, *clist;
    GtkWidget *button;
    component_list *component;
    size_t size, total;
    char text[1024];
    const char *message;

    /* Set through environment to hide questions, and assume Yes */
    int show_messages;
    const char *env;

    show_messages = 1;

    env = getenv("SETUP_NO_PROMPT");
    if (env && atoi(env))
        show_messages = 0;

    /* First switch to the next notebook page */
    notebook = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_notebook"));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);

    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "finished_button"));
    if (widget) {
        gtk_widget_set_sensitive(widget, FALSE);
    }

    /* Now uninstall all the selected components */
    progress = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_progress"));
    size = 0;
    total = calculate_recovered_space();

    widget = GTK_WIDGET(gtk_builder_get_object(uninstall_builder, "uninstall_vbox"));
    list = gtk_container_get_children(GTK_CONTAINER(widget));

    while (list && !uninstall_cancelled) {
        widget = GTK_WIDGET(list->data);
        poopy = gtk_container_get_children(GTK_CONTAINER(widget));
        widget = GTK_WIDGET(poopy->data);
        
        /* First do the addon components */
        clist = gtk_container_get_children(GTK_CONTAINER(widget));
        while (clist && !uninstall_cancelled) {
            button = GTK_WIDGET(clist->data);
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                component = g_object_get_data(G_OBJECT(button), "data");
                if (loki_isdefault_component(component->component)) {
                    clist = clist->next;
                    continue;
                }

               /* Put up the status */
                snprintf(text, sizeof(text), "%s: %s",
                        component->info->description,
                        loki_getname_component(component->component));
                set_status_text(text);

                /* See if the user wants to cancel the uninstall */
                while( gtk_events_pending() ) {
                    gtk_main_iteration();
                }
				
				/* Display an optional message to the user */
				message = loki_getmessage_component(component->component);
				if (show_messages && message && !display_message(message, BUTTON_OK|BUTTON_ABORT) ) {
                    clist = clist->next;
					uninstall_cancelled = 1;
					break;
				}

                /* Remove the component */
                if ( ! uninstall_component(component->component, component->info) ) {
					uninstall_cancelled = 2;
					snprintf(text, sizeof(text), _("Uninstallation of component %s has failed!\n"
												   "The whole uninstallation may be incomplete.\n"),
							 loki_getname_component(component->component));
					display_message(text, BUTTON_ABORT);
					break;
				}

                /* Update the progress bar */
                if ( total && progress ) {
                    size += component->size/1024;
                    gtk_progress_set_percentage(GTK_PROGRESS(progress),
                                                (float)size/total);
                }
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);

                /* See if the user wants to cancel the uninstall */
                while( gtk_events_pending() ) {
                    gtk_main_iteration();
                }
            }
            clist = clist->next;
        }
        /* Now do the primary components */
        clist = gtk_container_children(GTK_CONTAINER(widget));
        while ( clist && ! uninstall_cancelled ) {
            button = GTK_WIDGET(clist->data);
            if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) ) {
                component = gtk_object_get_data(G_OBJECT(button), "data");
                if ( ! loki_isdefault_component(component->component) ) {
                    clist = clist->next;
                    continue;
                }
                /* Put up the status */
                strncpy(text, component->info->description, sizeof(text));
                set_status_text(text);

                /* See if the user wants to cancel the uninstall */
                while( gtk_events_pending() ) {
                    gtk_main_iteration();
                }

				/* Display an optional message to the user */
				message = loki_getmessage_component(component->component);
				if ( message && !display_message(message, BUTTON_OK|BUTTON_ABORT) ) {
                    clist = clist->next;
					uninstall_cancelled = 1;
					break;
				}

                /* Remove the component */
                if ( ! perform_uninstall(component->product, component->info, 0) ) {
					uninstall_cancelled = 2;
					snprintf(text, sizeof(text), _("Uninstallation of product %s has failed!\n"
												   "Aborting the rest of the uninstallation.\n"),
							 component->info->description);
					display_message(text, BUTTON_ABORT);
					break;
				}
                remove_product(component->product);

                /* Update the progress bar */
                if ( total && progress ) {
                    size += component->size/1024;
                    gtk_progress_set_percentage(GTK_PROGRESS(progress),
                                                (float)size/total);
                }

                /* See if the user wants to cancel the uninstall */
                while( gtk_events_pending() ) {
                    gtk_main_iteration();
                }
                break;
            }
            clist = clist->next;
        }
        list = list->next;
    }
    switch ( uninstall_cancelled ) {
	case 1:
        set_status_text(_("Uninstall cancelled"));
		break;
	case 2:
        set_status_text(_("Uninstall aborted"));
		break;
	default:
        set_status_text(_("Uninstall complete"));
		gtk_progress_set_percentage(GTK_PROGRESS(progress), 1.0f);
		break;
    }
    widget = glade_xml_get_widget(uninstall_builder, "cancel_button");
    if ( widget ) {
        gtk_button_set_sensitive(widget, FALSE);
    }
    widget = glade_xml_get_widget(uninstall_builder, "finished_button");
    if ( widget ) {
        gtk_button_set_sensitive(widget, TRUE);
    }
}

static void component_toggled_slot(GtkWidget* w, gpointer data)
{
    static int in_component_toggled_slot = 0;
    component_list *list = (component_list *)data;
    gboolean state;
    struct component_button *button;

    /* Prevent recursion */
    if ( in_component_toggled_slot ) {
        return;
    }
    in_component_toggled_slot = 1;

    /* Set the state for any linked components */
    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    for ( button = list->buttons; button; button = button->next ) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->widget), state);
        if ( state ) {
            gtk_widget_set_sensitive(button->widget, FALSE);
        } else {
            gtk_widget_set_sensitive(button->widget, TRUE);
        }
    }

    /* Calculate recovered space, and we're done */
    calculate_recovered_space();
    in_component_toggled_slot = 0;
}

static void empty_container(GtkWidget *widget, gpointer data)
{
    gtk_container_remove(GTK_CONTAINER(data), widget);
}

static void log_handler(const gchar *log_domain,
					 GLogLevelFlags log_level,
					 const gchar *message,
					 gpointer user_data)
{
	/*	log_debug("Glib-WARNING(%s): %s\n", log_domain, message); */
}

/* Run a GUI to select and uninstall products */
int uninstall_ui(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *widget;
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *button;
    GtkWidget *label;
    const char *product_name;
    product_t *product;
    product_info_t *product_info;
    product_component_t *component;
    component_list *component_list, *addon_list;
    char text[1024];

#ifdef ENABLE_GTK2
    // Turn off any themes
    setenv("GTK2_RC_FILES", "", 1);
    setenv("GTK_DATA_PREFIX", "", 1);
#endif

gtk_init(&argc, &argv);

// Disable GLib warnings triggered by deprecated functionality, if needed (optional)
g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING | G_LOG_FLAG_RECURSION, log_handler, NULL);

// Initialize GtkBuilder
GtkBuilder *builder = gtk_builder_new();
if (!gtk_builder_add_from_file(builder, DATADIR "/" UNINSTALL_GLADE, NULL)) {
    g_print("Failed to load UI file\n");
    return -1;
}

// Connect signal handlers defined in the UI file
gtk_builder_connect_signals(builder, NULL);

// Get the uninstall button and make it insensitive
widget = GTK_WIDGET(gtk_builder_get_object(builder, "uninstall_button"));
if (widget) {
    gtk_widget_set_sensitive(widget, FALSE);
}

// Get the main window and ensure it is realized and visible
window = GTK_WIDGET(gtk_builder_get_object(builder, "loki_uninstall"));
gtk_widget_realize(window);
gtk_widget_show(window);

// Handle pending GTK events
while (gtk_events_pending()) {
    gtk_main_iteration();
}

// Add emergency signal handlers
signal(SIGHUP, main_signal_abort);
signal(SIGINT, main_signal_abort);
signal(SIGQUIT, main_signal_abort);
signal(SIGTERM, main_signal_abort);

// Get the uninstall_vbox and handle potential errors
widget = GTK_WIDGET(gtk_builder_get_object(builder, "uninstall_vbox"));
if (!widget) {
    fprintf(stderr, _("No uninstall_vbox in UI file!\n"));
    return -1;
}

    gtk_container_foreach(GTK_CONTAINER(widget), empty_container, widget);
    for ( product_name=loki_getfirstproduct();
          product_name;
          product_name=loki_getnextproduct() ) {
        /* See if we can open the product */
        product = loki_openproduct(product_name);
        if ( ! product ) {
            continue;
        }
        /* See if we have permissions to remove the product */
        product_info = loki_getinfo_product(product);
        if ( ! check_permissions(product_info, 0) ) {
            loki_closeproduct(product);
            continue;
        }
        /* Add the product and components to our list */
        strncpy(text, product_info->description, sizeof(text));
        frame = gtk_frame_new(text);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
        gtk_box_pack_start(GTK_BOX(widget), frame, FALSE, TRUE, 0);
        gtk_widget_show(frame);
        vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_add (GTK_CONTAINER (frame), vbox);
        gtk_widget_show(vbox);
        component = loki_getdefault_component(product);
        component_list = NULL;
        if ( component ) {
            component_list = create_component_list(product, product_info,
                                                   component);
            strncpy(text, _("Complete uninstall"), sizeof(text));
            button = gtk_check_button_new_with_label(text);
            gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
            gtk_signal_connect(G_OBJECT(button), "toggled",
                               G_CALLBACK(component_toggled_slot),
                               (gpointer)component_list);
            gtk_object_set_data(G_OBJECT(button), "data",
                                (gpointer)component_list);
            gtk_widget_show(button);
        }
        for ( component = loki_getfirst_component(product);
              component;
              component = loki_getnext_component(component) ) {
            if ( loki_isdefault_component(component) ) {
                continue;
            }
            addon_list = create_component_list(product, product_info,
                                               component);
            strncpy(text, loki_getname_component(component), sizeof(text));
            button = gtk_check_button_new_with_label(text);
            gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
            gtk_signal_connect(G_OBJECT(button), "toggled",
                               G_CALLBACK(component_toggled_slot),
                               (gpointer)addon_list);
            gtk_object_set_data(G_OBJECT(button), "data",
                                (gpointer)addon_list);
            gtk_widget_show(button);
            add_component_list(component_list, button);
        }

        /* Add this product to our list of open products */
        add_product(product);
    }

    /* Check to make sure there's something to uninstall */
    if ( ! product_list ) {
        label = gtk_label_new(
							  _("No products were installed by this user.\n"
								"You may need to run this tool as an administrator."));
        gtk_box_pack_start(GTK_BOX(widget), label, FALSE, TRUE, 0);
        gtk_widget_show(label);
    }

    /* Run the UI.. */
    gtk_main();

    /* Close all the products and return */
    close_products(0);
    return 0;
}
