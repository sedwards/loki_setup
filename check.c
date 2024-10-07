/*
 * Check and Rescue Tool for Loki Setup packages. Verifies the consistency of the files,
 * and optionally restores them from the original installation medium.
 *
 * $Id: check.c,v 1.17 2006-03-08 18:58:53 megastep Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>

#include <gtk/gtk.h>

#include "config.h"
#include "arch.h"
#include "setupdb.h"
#include "install.h"
#include "copy.h"

#ifdef HAVE_SELINUX_SELINUX_H
#include <selinux/selinux.h>
#endif

#undef PACKAGE
#define PACKAGE "loki-uninstall"

    #define CHECK_GLADE "loki_interface.gtk3.ui"

product_t *product = NULL;
GtkWidget *file_selector;
GtkBuilder *check_glade = NULL;
GtkBuilder *rescue_glade = NULL;
extern struct component_elem *current_component;

#ifdef __linux
int have_selinux = 0;
#endif

const char *argv0 = NULL;

product_info_t *info;

void abort_install(void)
{
	/* TODO */
	exit(1);
}

static void goto_installpath(char *argv0)
{
    char temppath[PATH_MAX];
    char datapath[PATH_MAX];
    char *home;

    home = getenv("HOME");
    if ( ! home ) {
        home = ".";
    }

    strcpy(temppath, argv0);    /* If this overflows, it's your own fault :) */
    if ( ! strrchr(temppath, '/') ) {
        char *path;
        char *last;
        int found;

        found = 0;
        path = getenv("PATH");
        do {
            /* Initialize our filename variable */
            temppath[0] = '\0';

            /* Get next entry from path variable */
            last = strchr(path, ':');
            if ( ! last )
                last = path+strlen(path);

            /* Perform tilde expansion */
            if ( *path == '~' ) {
                strcpy(temppath, home);
                ++path;
            }

            /* Fill in the rest of the filename */
            if ( last > (path+1) ) {
                strncat(temppath, path, (last-path));
                strcat(temppath, "/");
            }
            strcat(temppath, "./");
            strcat(temppath, argv0);

            /* See if it exists, and update path */
            if ( access(temppath, X_OK) == 0 ) {
                /* make sure it's not a directory... */
                struct stat s;
                if ((stat(temppath, &s) == 0) && (S_ISREG(s.st_mode)))
                    ++found;
            }
            path = last+1;

        } while ( *last && !found );

    } else {
        /* Increment argv0 to the basename */
        argv0 = strrchr(argv0, '/')+1;
    }

    /* Now canonicalize it to a full pathname for the data path */
    datapath[0] = '\0';
    if ( realpath(temppath, datapath) ) {
        /* There should always be '/' in the path */
        *(strrchr(datapath, '/')) = '\0';
    }
    if ( ! *datapath || (chdir(datapath) < 0) ) {
        fprintf(stderr, _("Couldn't change to install directory\n"));
        exit(1);
    }
}


static void init_locale(void)
{
    char locale[PATH_MAX];

	setlocale (LC_ALL, "");
    strcpy(locale, "locale");
	bindtextdomain (PACKAGE, locale);
	textdomain (PACKAGE);
	DetectLocale();
}

static int prompt_response;

static void prompt_button_slot( GtkWidget* widget, gpointer func_data)
{
    prompt_response = 1;
}

static void message_dialog(const char *txt, const char *title)
{
    GtkWidget *dialog, *label, *ok_button;
       
    /* Create the widgets */
    
    dialog = gtk_dialog_new();
    label = gtk_label_new (txt);
    ok_button = gtk_button_new_with_label("OK");

    prompt_response = 0;
    
    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    
    gtk_signal_connect_object (G_OBJECT (ok_button), "clicked",
                               G_CALLBACK (prompt_button_slot), G_OBJECT(dialog));

	gtk_signal_connect_object(G_OBJECT(dialog), "delete-event",
							  G_CALLBACK(prompt_button_slot), G_OBJECT(dialog));
	//gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), ok_button);

        gtk_dialog_add_button(GTK_DIALOG(dialog), "_OK", GTK_RESPONSE_OK);


    /* Add the label, and show everything we've added to the dialog. */
    
    //gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
      //                 label);

   GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   gtk_container_add(GTK_CONTAINER(content_area), label);


    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
	while ( !prompt_response ) {
	    gtk_main_iteration();
	}

    gtk_widget_destroy(dialog);	
}

static void add_message(GtkWidget *list, const char *str, ...)
{
	static int line = 0;
    va_list ap;
    char buf[BUFSIZ];
	GList *items = NULL;
	GtkWidget *item;

    va_start(ap, str);
    vsnprintf(buf, sizeof(buf), str, ap);
    va_end(ap);

	item = gtk_list_item_new_with_label(buf);
	gtk_widget_show(item);
	items = g_list_append(items, item);
	gtk_list_append_items(GTK_LIST(list), items);
	line++;
	gtk_list_scroll_vertical(GTK_LIST(list), GTK_SCROLL_JUMP, 1.0);
}

void on_cdrom_radio_toggled(GtkWidget *widget, gpointer user_data)
{
	gtk_widget_set_sensitive(gtk_builder_get_object(rescue_glade, "dir_entry"), FALSE);
	gtk_widget_set_sensitive(gtk_builder_get_object(rescue_glade, "pick_dir_but"), FALSE);
}

void on_dir_radio_toggled(GtkWidget *widget, gpointer user_data)
{
	gtk_widget_set_sensitive(gtk_builder_get_object(rescue_glade, "dir_entry"), TRUE);
	gtk_widget_set_sensitive(gtk_builder_get_object(rescue_glade, "pick_dir_but"), TRUE);
}

void
on_media_cancel_clicked (GtkButton       *button,
						 gpointer         user_data)
{
	gtk_widget_hide(gtk_builder_get_object(rescue_glade, "media_select"));
}

int check_xml_setup(const char *file, const char *product)
{
	int ret = 0;
	xmlDocPtr doc = xmlParseFile(file);
	if ( doc ) {
		const char *prod = xmlGetProp(XML_ROOT(doc), "product");
		if ( prod && !strcmp(prod, product) ) {
			ret = 1;
		}
		xmlFreeDoc(doc);
	}
	return ret;
}

void
on_media_ok_clicked (GtkButton       *button,
					 gpointer         user_data)
{
	GtkWidget *diag = gtk_builder_get_object(check_glade, "diagnostic_label"), *radio;

	char path[PATH_MAX], root[PATH_MAX];
	install_info *install;
	
	radio = gtk_builder_get_object(rescue_glade, "dir_radio");
	if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) ) {
		/* Directory */
		const gchar *str = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(rescue_glade, "dir_entry")));
		/* We ignore the CDROM prefix in that case */
		snprintf(path, sizeof(path), "%s/setup.data/setup.xml", str);
		if ( access( path, R_OK) ) {
			/* Doesn't look like a setup archive */
			gtk_label_set_text(GTK_LABEL(diag), _("Unable to identify an installation media."));
			gtk_widget_hide(gtk_builder_get_object(rescue_glade, "media_select"));
			return;
		}
		strncpy(root, str, sizeof(root));
	} else {
		/* Go through all CDROM devices and look for setup.xml files */
		char *cds[SETUP_MAX_DRIVES];
		int nb_drives = detect_and_mount_cdrom(cds), i;

		for(i = 0; i < nb_drives; ++i ) {
			snprintf(path, sizeof(path), "%s/%s/setup.data/setup.xml", cds[i], info->prefix);
			if ( !access( path, R_OK) ) {
				snprintf(root, sizeof(root), "%s/%s", cds[i], info->prefix);
				break; /* FIXME: What if there are many setup CDs ? */
			}
		}
		free_mounted_cdrom(nb_drives, cds);
	}

	/* Verify that the package is the same (product name and version) */
	if ( ! check_xml_setup(path, info->name) ) {
		gtk_label_set_text(GTK_LABEL(diag), _("Installation media doesn't match the product."));
		gtk_widget_hide(gtk_builder_get_object(rescue_glade, "media_select"));
		return;		
	}

	gtk_label_set_text(GTK_LABEL(diag), _("Restoring files..."));
	gtk_widget_realize(diag);
	
	/* Fetch the files to be refreshed, i.e install with a restricted set of files  */
	install = create_install(path, info->root, NULL, info->prefix);
	if ( install ) {
		const char *f;

		if ( chdir(root) < 0 ) {
			fprintf(stderr, _("Unable to change to directory %s\n"), root);
		} else {
			/* Update the setup path */
			strncpy(install->setup_path, root, sizeof(install->setup_path));
		}

		/* Check if we need to create a default component entry */
		if ( GetProductNumComponents(install) == 0 ) {
			current_component = add_component_entry(install, "Default", install->version, 1, NULL, NULL);
		}

		/* Restore any environment */
		loki_put_envvars_component(loki_find_component(product, current_component->name));

		/* Enable the relevant options */
		select_corrupt_options(install);
		copy_tree(install, XML_CHILDREN(XML_ROOT(install->config)), install->install_path, NULL);

		/* Menu items are currently not being restored - maybe they should be tagged in setupdb ? */

		/* Install the optional README and EULA files
		   Warning: those are always installed in the root of the installation directory!
		*/
		f = GetProductREADME(install, NULL);
		if ( f && ! GetProductIsMeta(install) ) {
			copy_path(install, f, install->install_path, NULL, 1, NULL, NULL, NULL);
		}
		f = GetProductEULA(install, NULL);
		if ( f && ! GetProductIsMeta(install) ) {
			copy_path(install, f, install->install_path, NULL, 1, NULL, NULL, NULL);
		}

	}

	/* Print end message and disable the "Rescue" button */
	gtk_label_set_text(GTK_LABEL(diag), _("Files successfully restored !"));
	gtk_widget_set_sensitive(gtk_builder_get_object(check_glade, "rescue_button"), FALSE);
	gtk_widget_hide(gtk_builder_get_object(rescue_glade, "media_select"));

	message_dialog(_("Files successfully restored !"), _("Success"));

	/* Unmount filesystems that may have been mounted */
	unmount_filesystems();
}

void store_filename(GtkButton *but, GtkWidget *entry)
{
	const gchar *str = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));
	struct stat st;

	if ( stat(str, &st)==0 && S_ISDIR(st.st_mode) ) {
		gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(rescue_glade, "dir_entry")), str);
	} else {
	    char buf[PATH_MAX];
		/* Warn that there is no such directory ? */
		snprintf(buf, sizeof(buf), _("Warning: '%s' is not an accessible directory."), str);
		message_dialog(buf, _("Warning"));
	}
}

void on_file_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    if (response_id == GTK_RESPONSE_ACCEPT) {
        // Retrieve the selected directory
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_print("Selected folder: %s\n", folder);
        g_free(folder); // Free the memory allocated for the folder string
    }

    // Destroy the dialog after handling the response
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void on_pick_dir_but_clicked(GtkButton *button, gpointer user_data)
{
    // Create a new file chooser dialog for selecting a directory
    GtkWidget *file_selector = gtk_file_chooser_dialog_new("Please select a directory",
                                                           //GTK_WINDOW(file_selector),
                                                           file_selector,
                                                           GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                                           "_OK", GTK_RESPONSE_ACCEPT,
                                                           NULL);

    // Set the current folder to the user's home directory, or another default path
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_selector), g_get_home_dir());

    // Connect the response signal to handle OK or Cancel
    g_signal_connect(file_selector, "response", G_CALLBACK(on_file_dialog_response), user_data);

    // Make the file chooser dialog modal
    gtk_window_set_modal(GTK_WINDOW(file_selector), TRUE);

    // Show the dialog
    gtk_widget_show(file_selector);
}


void on_rescue_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window;

    // Load the UI using GtkBuilder instead of Glade
    rescue_glade = gtk_builder_new();
    gtk_builder_add_from_file(rescue_glade, CHECK_GLADE, NULL);
    gtk_builder_connect_signals(rescue_glade, NULL);

    // Ask the user to insert the media
    window = GTK_WIDGET(gtk_builder_get_object(rescue_glade, "media_select"));
    
    // Trigger the toggle action
    on_cdrom_radio_toggled(NULL, NULL);

    // Show the window
    gtk_widget_show(window);
}

void
on_dismiss_button_clicked            (GtkButton       *button,
									  gpointer         user_data)
{
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	GtkWidget *window, *ok_but, *fix_but, *diag, *list, *scroll;
	GtkAdjustment *adj;
    product_component_t *component;
	product_option_t *option;
	product_file_t *file;
	int removed = 0, modified = 0;
	
    goto_installpath(argv[0]);

	/* Set the locale */
    init_locale();

#ifdef __linux
	/* See if we have a SELinux environment */
# ifdef HAVE_SELINUX_SELINUX_H
	have_selinux = is_selinux_enabled();
# else
	if ( !access("/usr/bin/chcon", X_OK) && !access("/usr/sbin/getenforce", X_OK) ) {
		have_selinux = 1;
	}
# endif
#endif

	if ( argc < 2 ) {
		fprintf(stderr, _("Usage: %s product\n"), argv[0]);
		return 1;
	}

    gtk_init(&argc,&argv);

	argv0 = argv[0];

    /* Initialize Glade */
    check_glade = GLADE_XML_NEW(CHECK_GLADE, "check_dialog"); 

    /* Add all signal handlers defined in glade file */
    window = gtk_builder_get_object(check_glade, "check_dialog");
    gtk_widget_realize(window);
    while( gtk_events_pending() ) {
        gtk_main_iteration();
    }

	diag = gtk_builder_get_object(check_glade, "diagnostic_label");
	ok_but = gtk_builder_get_object(check_glade, "dismiss_button");
	fix_but = gtk_builder_get_object(check_glade, "rescue_button");
	list = gtk_builder_get_object(check_glade, "main_list");
	scroll = gtk_builder_get_object(check_glade, "scrolledwindow");

	product = loki_openproduct(argv[1]);
	if ( ! product ) {
	  message_dialog(_("Impossible to locate the product information.\nMaybe another user installed it?"),
					 _("Error"));
		return 1;
	}

	info = loki_getinfo_product(product);

	gtk_label_set_text(GTK_LABEL(diag), "");
	gtk_widget_set_sensitive(fix_but, FALSE);

	adj = GTK_ADJUSTMENT(gtk_adjustment_new(100.0, 1.0, 100.0, 1.0, 10.0, 10.0));
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(scroll), adj);

	/* Iterate through the components */
	for ( component = loki_getfirst_component(product);
		  component;
		  component = loki_getnext_component(component) ) {

		add_message(list, _("---> Checking component '%s'..."), loki_getname_component(component));

		for ( option = loki_getfirst_option(component);
			  option;
			  option = loki_getnext_option(option) ) {
			
			add_message(list, _("-> Checking option '%s'..."), loki_getname_option(option));

			for ( file = loki_getfirst_file(option);
				  file;
				  file = loki_getnext_file(file) ) {

				gtk_main_iteration();
				switch ( loki_check_file(file) ) {
				case LOKI_REMOVED:
					add_message(list, _("%s was REMOVED"), loki_getpath_file(file));
					removed ++;
					add_corrupt_file(product, loki_getpath_file(file), loki_getname_option(option));
					break;
				case LOKI_CHANGED:
					add_message(list, _("%s was MODIFIED"), loki_getpath_file(file));
					modified ++;
					add_corrupt_file(product, loki_getpath_file(file), loki_getname_option(option));
					break;
				case LOKI_OK:
					add_message(list, _("%s is OK"), loki_getpath_file(file));
					break;
				}
			}
		}
	}

	if ( removed || modified ) {
		char status[200];

		snprintf(status, sizeof(status), _("Changes detected: %d files removed, %d files modified."), 
				 removed, modified);
		gtk_label_set_text(GTK_LABEL(diag), status);
		gtk_widget_set_sensitive(fix_but, TRUE);
	} else {
		gtk_label_set_text(GTK_LABEL(diag), _("No problems were found."));
	}

    /* Run the UI.. */
    gtk_main();

	return 0;
}



