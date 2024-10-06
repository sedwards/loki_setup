/* GTK-based UI
   $Id: gtk_ui.c,v 1.130 2007-01-30 00:06:19 megastep Exp $
*/

/* Modifications by Borland/Inprise Corp.
   04/11/2000: Added check in check_install_button to see if install and
               binary path are the same. If so, leave install button
	       disabled and give user a message.

   04/17/2000: Created two new GladeXML objects, one for the readme dialog,
               the other for the license dialog and modified gtkui_init,
	       setup_button_view_readme_slot and gtkui_license to create &
	       use these new objects. Created 2 new handlers for destroy
	       events on the readme and license dialogs. This was done to fix
	       problems when the user uses the 'X' button in the upper
	       right corner instead of the Close, Cancel or Agree buttons.
	       For the readme, setup would seg fault if the user tried to open
	       the readme dialog a second time. For the license, setup would
	       stop responding.

	       The setup.glade file was  modified to include destroy
	       handlers for the readme_dialog and license_dialog widgets.

	       Added code to gtkui_complete to clean up the GladeXML objects.

  04/21/2000:  Cleaned up a bit too much, too soon in gtkui_complete. 
               Removed the gtk_object_unref for setup_interface because it's 
	       still in use by the Play Now button if uid is root...

  04/28/2000:  More cleanup problems. Can't unref the gtk objects in 
               gtkui_complete...too much is still in use. Maybe make a cleanup
	       routine that gets called by the exit and play button handlers.
	       
	       Added code to disable the View Readme button (all 3 of them)
	       when it is clicked. That way, the user can have only 1 instance
	       of the readme dialog. Multiple instances was causing a problem
	       for the destroy routine. Destroying the latest instance 
	       worked OK. Destroying the others caused a seg fault. Readme
	       buttons are re-enabled when the dialog is closed.

	       Cleaned up close_view_readme_slot to avoid the duplication with
	       destroy_view_readme. close now just calls destroy.

  05/12/2000:  Changed the way the focus mechanism works for the install path
               and binary path fields and how the Begin Install button gets
	       enabled/disabled. There were ways in which an invalid path 
	       could be selected from the combo boxes and the Begin Install
	       button would not be disabled until the user clicked on it. Then,
	       the mouse focus was on an insensitive item, making it appear 
	       that the UI locked up. Here's the changes:

	       Modified gtkui_init to add signal handlers for the keypress and
	       mouse button release events for both the install path and binary
	       path fields. Also added these functions to handle the signals:
	       path_entry_keypress_slot, path_combo_change_slot,
	       binary_path_entry_keypress_slot, binary_path_combo_change_slot.
	       The "keypress" slots evaluate the status of the Begin Install
	       button after every keystroke when the user is typing a pathname.
	       The "combo_change" slots evaluate the status of the button when-
	       ever the user makes a selection from the drop-down list.

	       Modified setup.glade to remove the signal handlers that grabbed
	       the focus when the mouse was passed over these fields.

	       Modified gtkui_prompt to add a RESPONSE_OK option and modifed
	       the yesno_answer structure to support this. This will display
	       a dialog with a message and a single OK button. This was a 
	       result of one of the (many) failed attempts to fix the focus 
	       issue. It turns out that I don't use this anywhere anymore, but 
	       I figured I'd leave it for someone who wants to display a 
	       simple message without using the gnome libs. To use it, just 
	       pass RESPONSE_OK as the last parameter.

  05/17/2000:  Modified gtkui_init to disable the install path or binary path
               widgets if the value was passed in as a command-line parameter
	       (see main.c for the new -i and -b options).

  06/02/2000:  Modified gtkui_update to make sure the progress bar object is
               never sent an invalid value. If the <option size="xx"> value is
	       not set correctly, then the calculated new_update value could
	       sometimes be greater than 1. This generated lots of gtk error
	       messages on the console. Added an if/else statement to make
	       sure the calculated value never exceeds 1.
*/

#define ENABLE_GTK2
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <gtk/gtk.h>
//#include <glade/glade.h>

#include "install.h"
#include "install_ui.h"
#include "install_log.h"
#include "detect.h"
#include "file.h"
#include "copy.h"
#include "bools.h"
#include "loki_launchurl.h"

#define SETUP_GLADE SETUP_BASE "loki_interface.gtk3.ui"

#define LICENSE_FONT            \
        "-misc-fixed-medium-r-semicondensed-*-*-120-*-*-c-*-iso8859-8"

#define MAX_TEXTLEN	40	/* The maximum length of current filename */

/* Globals */

static char *default_install_paths[] = {
    "/usr/local/games",
    "/opt/games",
    "/usr/games",
    NULL
};

/* Use an arbitrary maximum; the lack of
   flexibility shouldn't be an issue */
#define MAX_INSTALL_PATHS       26
 
static char *install_paths[MAX_INSTALL_PATHS];

/* Various warning dialogs */
static enum {
    WARNING_NONE,
    WARNING_ROOT
} warning_dialog;

typedef enum
{
	CLASS_PAGE,
    OPTION_PAGE,
    COPY_PAGE,
    DONE_PAGE,
    ABORT_PAGE,
    WARNING_PAGE,
    WEBSITE_PAGE,
    CDKEY_PAGE
} InstallPages;

static GtkBuilder *setup_interface = NULL;
static GtkBuilder *setup_interface_readme = NULL;
static GtkBuilder *setup_interface_license = NULL;
static int cur_state;
static install_info *cur_info;
static int diskspace;
static int license_okay = 0;
static gboolean in_setup = TRUE;
static GSList *radio_list = NULL; /* Group for the radio buttons */

static const char* glade_file = SETUP_GLADE;

/******** Local prototypes **********/

static const char *check_for_installation(install_info *info, char** explanation);
static void check_install_button(void);
static void update_space(void);
static void update_size(void);
void setup_destroy_view_readme_slot(GtkWidget*, gpointer);
static yesno_answer gtkui_prompt(const char*, yesno_answer);
static void gtkui_abort(install_info *info);

static int iterate_for_state(void)
{
	int start = cur_state;
	while(cur_state == start) {
		if ( !gtk_main_iteration() )
			break;
	}

	/* fprintf(stderr,"New state: %d\n", cur_state); */
	return cur_state;
}

/*********** GTK slots *************/

/*void setup_entry_gainfocus( GtkWidget* widget, gpointer func_data )
{
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(widget)), widget);
}
void setup_entry_givefocus( GtkWidget* widget, gpointer func_data )
{
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(widget)), NULL);
}*/

gint path_entry_keypress_slot(GtkWidget *widget, GdkEvent *event, 
				  gpointer data)
{
    const char* string;
    
    string = gtk_entry_get_text( GTK_ENTRY(widget) );
    if ( string ) {
        set_installpath(cur_info, string, 0);
        if ( strcmp(string, cur_info->install_path) != 0 ) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->install_path);
        }
        update_space();
    }
    return FALSE;
}

/******** Combined GtkWidget porting Helper Function ************/
static GtkWidget *get_widget_with_builder(GtkBuilder *builder, const char *widget_name) {
    GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object(builder, widget_name));

    if (!widget) {
        g_print("Failed to load widget: %s\n", widget_name);
    } else {
        g_print("Successfully loaded widget: %s\n", widget_name);
    }

    return widget;
}

/******** Helper functions to use specific builders ************/
static GtkWidget *get_widget(const char *widget_name) {
    return get_widget_with_builder(setup_interface, widget_name);
}

static GtkWidget *get_widget_license(const char *widget_name) {
    return get_widget_with_builder(setup_interface_license, widget_name);
}

static GtkWidget *get_widget_readme(const char *widget_name) {
    return get_widget_with_builder(setup_interface_readme, widget_name);
}

gint binary_path_combo_change_slot(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    const char *string;
    GtkWidget *binary_entry;

    // Use GtkBuilder to retrieve the "binary_entry" widget
    binary_entry = get_widget_with_builder(setup_interface, "binary_entry" );

    string = gtk_entry_get_text(GTK_ENTRY(binary_entry));
    if (string) {
        // Set the symlink path
        set_symlinkspath(cur_info, string);

        // Update the entry text if it differs from the current symlink path
        if (strcmp(string, cur_info->symlinks_path) != 0) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->symlinks_path);
        }

        // Check if the install button should be enabled
        check_install_button();
    }

    return FALSE;  // Event was handled, no further processing needed
}

gint path_combo_change_slot(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    const char *string;
    GtkWidget *install_entry;

    install_entry = get_widget_with_builder(setup_interface, "install_entry" );

    string = gtk_entry_get_text(GTK_ENTRY(install_entry));
    if (string) {
        set_installpath(cur_info, string, 1);

        // Compare and update the entry text if it has changed
        if (strcmp(string, cur_info->install_path) != 0) {
            gtk_entry_set_text(GTK_ENTRY(install_entry), cur_info->install_path);
        }
        update_space();  // Update space after setting the new install path
    }

    return FALSE;  // Return FALSE to indicate the event was handled
}

gboolean setup_entry_installpath_slot( GtkWidget* widget, GdkEventFocus *event, gpointer func_data )
{
    const char* string;
    string = gtk_entry_get_text( GTK_ENTRY(widget) );
    if ( string ) {
        set_installpath(cur_info, string, 1);
        if ( strcmp(string, cur_info->install_path) != 0 ) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->install_path);
        }
        update_space();
    }
	return FALSE;
}

gint binary_path_entry_keypress_slot(GtkWidget *widget, GdkEvent *event, 
				  gpointer data)
{
    const char* string;
    
    string = gtk_entry_get_text( GTK_ENTRY(widget) );
    if ( string ) {
        set_symlinkspath(cur_info, string);
        if ( strcmp(string, cur_info->symlinks_path) != 0 ) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->symlinks_path);
        }
        check_install_button();
    }    
    return FALSE;
}

gboolean setup_entry_binarypath_slot(GtkWidget *widget, GdkEventFocus *event, gpointer func_data)
{
    const char *string;

    // Get the text from the GtkEntry widget
    string = gtk_entry_get_text(GTK_ENTRY(widget)); 
    if (string) {
        // Set the symlink path based on the entered text
        set_symlinkspath(cur_info, string);

        // If the current symlink path differs from the entered text, update the entry
        if (strcmp(string, cur_info->symlinks_path) != 0) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->symlinks_path);
        }

        // Check if the install button should be enabled
        check_install_button();
    }

    return FALSE;  // Returning FALSE to propagate the event further
}

/* Computes a nice size for a dialog box */
static int get_nice_width(GtkWidget *widget, int maxlen)
{
    PangoContext *pc;
    PangoFontDescription *fd;
    PangoLanguage *pl;
    PangoFontMetrics *fm;
    int approx_width;

    pc = gtk_widget_get_pango_context(widget);
    fd = pango_context_get_font_description(pc);
    pl = pango_context_get_language(pc);
    fm = pango_context_get_metrics(pc, fd, pl);
    approx_width = pango_font_metrics_get_approximate_char_width(fm);

    if (maxlen > 100)
        maxlen = 100;


    return((maxlen * approx_width) / PANGO_SCALE);

}

/*
   Returns 0 if fails.
*/
static gboolean load_file_gtk3(GtkTextView *widget, const char *file)
{
    FILE *fp;
    int pos;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    int nice_width = 0;                     
    int maxlen = 0;
    GtkWidget *toplevel;
                                                
    buffer = gtk_text_view_get_buffer(widget);
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_delete(buffer, &start, &end);
    
    fp = fopen(file, "r");
    if (fp) {                    
        char line[BUFSIZ];
        pos = 0;
        while (fgets(line, BUFSIZ-1, fp)) {
            gtk_text_buffer_insert(buffer, &end, line, -1);  // Use iter instead of insert_at_cursor
            if (strlen(line) > maxlen)
                maxlen = strlen(line);
        }
        fclose(fp);
    }

    // Reset cursor to the start
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_place_cursor(buffer, &start);

    // Calculate and set window size based on file width
    nice_width = get_nice_width(GTK_WIDGET(widget), maxlen);
    if (nice_width / 5 < 75)
        nice_width += 75;
    else
        nice_width += (nice_width / 5);

    toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
    if (gtk_widget_is_toplevel(toplevel))  // GTK3 equivalent to GTK_WIDGET_TOPLEVEL
        gtk_window_set_default_size(GTK_WINDOW(toplevel), nice_width, (nice_width * 3) / 4);

    return (fp != NULL);
}

void on_class_continue_clicked(GtkWidget *w, gpointer data)
{
    GtkWidget *widget = get_widget( "recommended_but" );

    // Check if the current state is SETUP_CLASS
    if (cur_state != SETUP_CLASS) {
        return;
    }

    // Get the state of the express setup toggle button
    express_setup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
               
    if (express_setup) {
        const char *msg = check_for_installation(cur_info, NULL);
        if (msg) {
            char buf[BUFSIZ];
            snprintf(buf, sizeof(buf),
                     _("Installation could not proceed due to the following error:\n%s\nTry to use 'Expert' installation."), msg);
            
            gtkui_prompt(buf, RESPONSE_OK);
            
            widget = get_widget( "expert_but" );
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
            return;
        }
    }      

    // Install desktop menu items if applicable
    if ((!GetProductHasNoBinaries(cur_info)) && (GetProductInstallMenuItems(cur_info))) {
        cur_info->options.install_menuitems = 1;
    }

    // Move to the next page in the setup notebook
    widget = get_widget( "setup_notebook" );
    gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), OPTION_PAGE);
    cur_state = SETUP_OPTIONS;
}

void setup_close_view_readme_slot( GtkWidget* w, gpointer data )
{  
    setup_destroy_view_readme_slot(w, data);
}

void setup_destroy_view_readme_slot(GtkWidget* w, gpointer data)
{
    GtkWidget *widget;

    if (setup_interface_readme) {
        // Hide the readme dialog
        widget = GTK_WIDGET(gtk_builder_get_object(setup_interface_readme, "readme_dialog"));
        if (widget) {
            gtk_widget_hide(widget);
        }

        // Free the builder and reset the pointer
        g_object_unref(setup_interface_readme);
        setup_interface_readme = NULL;

        /*
         * Re-enable the 'view readme' buttons (all 3 of them)
         */
        widget = get_widget( "button_readme" );
        gtk_widget_set_sensitive(widget, TRUE);

        widget = get_widget( "class_readme" );
        gtk_widget_set_sensitive(widget, TRUE);

        widget = get_widget( "view_readme_progress_button" );
        gtk_widget_set_sensitive(widget, TRUE);

        widget = get_widget( "view_readme_end_button" );
        gtk_widget_set_sensitive(widget, TRUE);
    }
}

void setup_button_view_readme_slot(GtkWidget *w, gpointer data)
{   
    GtkWidget *readme;
    GtkWidget *widget; 
    const char *file;

    // Load the UI using GtkBuilder
    setup_interface_readme = gtk_builder_new();

    if (!gtk_builder_add_from_file(setup_interface_readme, "loki_interface.gtk3.ui", NULL)) {
      g_print("Failed to load UI file\n");
    }

    gtk_builder_add_from_file(setup_interface_readme, glade_file, NULL);
    gtk_builder_connect_signals(setup_interface_readme, NULL);

    // Get widgets from GtkBuilder
    readme = get_widget_readme( "readme_dialog");
    widget = get_widget_readme( "readme_area");

    file = GetProductREADME(cur_info, NULL);
    if (file && readme && widget) {
        // Hide, load the README file, and show the readme dialog
        gtk_widget_hide(readme);
        load_file_gtk3(GTK_TEXT_VIEW(widget), file);  // Assuming the function is adapted for GTK3
        gtk_widget_show(readme);

        // Disable all related buttons
        widget = get_widget( "button_readme" );
        gtk_widget_set_sensitive(widget, FALSE);

        widget = get_widget( "class_readme" );
        gtk_widget_set_sensitive(widget, FALSE);

        widget = get_widget( "view_readme_progress_button" );
        gtk_widget_set_sensitive(widget, FALSE);

        widget = get_widget( "view_readme_end_button" );
        gtk_widget_set_sensitive(widget, FALSE);
    }
}

void setup_button_license_agree_slot(GtkWidget *widget, gpointer func_data)
{
    GtkWidget *license;

    // Get the license dialog from GtkBuilder
    license = get_widget_license( "license_dialog" );

    if (license) {
        // Hide the license dialog
        gtk_widget_hide(license);
    }

    license_okay = 1;

    check_install_button();

    cur_state = SETUP_README;
}

void setup_destroy_license_slot(GtkWidget *w, gpointer data)
{       
    /* !!! FIXME: this gets called more than once if LANG is set...not sure
     * !!! FIXME:  why, but we explicitly set setup_interface_license to NULL
     * !!! FIXME:  here so we don't touch an unref'd var.
     */
    if (setup_interface_license) {
        GtkWidget *widget;
        
        // Use the get_widget wrapper to retrieve the license dialog widget
        widget = get_widget( "license_dialog");
        gtk_widget_hide(widget);

        cur_state = SETUP_EXIT;

        // Unreference the builder and set it to NULL
        g_object_unref(setup_interface_license);
        setup_interface_license = NULL;
    }
}

void setup_button_warning_continue_slot( GtkWidget* widget, gpointer func_data )
{
    switch (warning_dialog) {
        case WARNING_NONE:
            break;
        case WARNING_ROOT:
            cur_state = SETUP_PLAY;
            break;
    }
    warning_dialog = WARNING_NONE;
}
void setup_button_warning_cancel_slot( GtkWidget* widget, gpointer func_data )
{
    switch (warning_dialog) {
        case WARNING_NONE:
            break;
        case WARNING_ROOT:
            cur_state = SETUP_EXIT;
            break;
    }
    warning_dialog = WARNING_NONE;
}

void setup_button_complete_slot( GtkWidget* _widget, gpointer func_data )
{
    cur_state = SETUP_COMPLETE;
}

void setup_button_play_slot( GtkWidget* _widget, gpointer func_data )
{
	/* Enable this only if the application actually does that */
#if 0
    if ( getuid() == 0 ) {
		GtkWidget *widget;
		const char *warning_text =
			_("If you run this as root, the preferences will be stored in\n"
			  "root's home directory instead of your user account directory.");

        warning_dialog = WARNING_ROOT;
        widget = get_widget( "setup_notebook");
        gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), WARNING_PAGE);
        widget = get_widget( "warning_label");
        gtk_label_set_text(GTK_LABEL(widget), warning_text);
    } else
#endif
        cur_state = SETUP_PLAY;
}

void setup_button_exit_slot( GtkWidget* widget, gpointer func_data )
{
	cur_state = SETUP_EXIT;
}

void setup_button_abort_slot( GtkWidget* widget, gpointer func_data )
{
	/* Make sure that the state will be different so that we can iterate */
	cur_state = (cur_state == SETUP_ABORT) ? SETUP_EXIT : SETUP_ABORT;
}

void setup_button_cancel_slot( GtkWidget* widget, gpointer func_data )
{
	switch(cur_state) {
	case SETUP_COMPLETE:
	case SETUP_OPTIONS:
	case SETUP_ABORT:
	case SETUP_EXIT:
		cur_state = SETUP_EXIT;
		break;
	default:
		if ( gtkui_prompt(_("Are you sure you want to abort\nthis installation?"), RESPONSE_NO) == RESPONSE_YES ) {
			cur_state = SETUP_ABORT;
			abort_install();
		}
		break;
	}
}

static void message_dialog(const char *txt, const char *title);

/* hacked in cdkey support.  --ryan. */
extern char gCDKeyString[128];

void setup_cdkey_entry_changed_slot(GtkEntry *entry, gpointer user_data)
{
    const gchar *CDKey = gtk_entry_get_text( GTK_ENTRY(entry) );
    GtkWidget *button;
    button = get_widget( "setup_button_cdkey_continue");

    gtk_widget_set_sensitive(button, (*CDKey) ? TRUE : FALSE);
}

void setup_button_cdkey_continue_slot( GtkWidget* widget, gpointer func_data )
{
    /* HACK: Use external cd key validation program, if it exists. --ryan. */
    #define CDKEYCHECK_PROGRAM "./vcdk"
    char cmd[sizeof (gCDKeyString) + sizeof (CDKEYCHECK_PROGRAM) + 64];
    GtkWidget *entry = get_widget( "setup_cdkey_entry");
    char *CDKey = (char *) gtk_entry_get_text( GTK_ENTRY(entry) );
    char *p;

    snprintf(cmd, sizeof (cmd), "%s-%s", CDKEYCHECK_PROGRAM, cur_info->arch);
    if (access(cmd, X_OK) != 0)
    {
        message_dialog(_("ERROR: vcdk is missing. Installation aborted.\n"), _("Problem"));
        cur_state = SETUP_ABORT;
        return;
    }
    else
    {
        snprintf(cmd, sizeof (cmd), "%s-%s %s", CDKEYCHECK_PROGRAM, cur_info->arch, CDKey);
        if (system(cmd) == 0)  /* binary ran and reported key invalid? */
        {
            message_dialog(_("CD key is invalid!\nPlease double check your key and enter it again."), _("Problem"));
            return;
        }
    }

    strncpy(gCDKeyString, CDKey, sizeof (gCDKeyString));
    gCDKeyString[sizeof (gCDKeyString) - 1] = '\0';
    p = gCDKeyString;
    while(*p)
    {
        *p = toupper(*p);
        p++;
    }

    cur_state = SETUP_INSTALL;
}

void setup_button_install_slot( GtkWidget* widget, gpointer func_data )
{
	const char* message;
	char* explanation = NULL;
    GtkWidget *notebook;
    notebook = get_widget( "setup_notebook");

	message = check_for_installation(cur_info, &explanation);

	if(message)
	{
		if(explanation)
		{
			char* tmp = g_strconcat(message, "\n\n", explanation, NULL);
			g_free(explanation);
			explanation = tmp;
		}

		gtkui_prompt(explanation?explanation:message, RESPONSE_OK);
		g_free(explanation);
		return;
	}

    /* If CDKEY attribute was specified, show the CDKEY screen */
    if(GetProductCDKey(cur_info))
    {
        GtkWidget *button = get_widget( "setup_button_cdkey_continue");
        GtkWidget *entry = get_widget( "setup_cdkey_entry");

		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), CDKEY_PAGE);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        gtk_widget_set_sensitive(button, FALSE);

        cur_state = SETUP_CDKEY;
        iterate_for_state();
        if (cur_state != SETUP_INSTALL)
            return;
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), COPY_PAGE);
    cur_state = SETUP_INSTALL;
}

void setup_button_browser_slot( GtkWidget* widget, gpointer func_data )
{
    /* Don't let the user accidentally double-launch the browser */
    gtk_widget_set_sensitive(widget, FALSE);

    launch_browser(cur_info, loki_launchURL);
}

/* Returns NULL if installation can be performed */
static const char *check_for_installation(install_info *info, char** explanation)
{
    if ( ! license_okay ) {
        return _("Please respond to the license dialog");
    }
	return IsReadyToInstall_explain(info, explanation);
}

/* Checks if we can enable the "Begin install" button */
static void check_install_button(void)
{
    const char *message;
    GtkWidget *options_status;
    GtkWidget *install_widget;

    message = check_for_installation(cur_info, NULL);

    /* Get the appropriate widgets and set the new state */
    options_status = get_widget( "options_status");
    install_widget = get_widget( "button_install");

    if ( !message ) {
        message = _("Ready to install!");

        gtk_widget_set_sensitive(install_widget, TRUE);
    }
    else
        gtk_widget_set_sensitive(install_widget, FALSE);
    gtk_label_set_text(GTK_LABEL(options_status), message);
}

static void update_size(void)
{
    GtkWidget *widget;
    char text[32];

    // Use GtkBuilder to get the "label_install_size" widget
    widget = get_widget( "label_install_size" );
    if (widget) {
        // Format the install size as MB
        snprintf(text, sizeof(text), _("%d MB"), (int)BYTES2MB(cur_info->install_size));

        // Update the label text
        gtk_label_set_text(GTK_LABEL(widget), text);

        // Check if the install button should be enabled
        check_install_button();
    }
}

static void update_space(void)
{   
    GtkWidget *widget;
    char text[32]; 
    int diskspace;

    // Retrieve the "label_free_space" widget from GtkBuilder
    widget = get_widget( "label_free_space" );

    if (widget) {
        // Detect the available disk space at the install path
        diskspace = detect_diskspace(cur_info->install_path);
        
        // Format the detected disk space into a string (in MB)
        snprintf(text, sizeof(text), _("%d MB"), diskspace);
        
        // Update the label widget to display the available space
        gtk_label_set_text(GTK_LABEL(widget), text);
        
        // Check if the install button should be enabled based on available space
        check_install_button();
    }
}

static void empty_container(GtkWidget *widget, gpointer data)
{
    gtk_container_remove(GTK_CONTAINER(data), widget);
}

static void enable_tree(xmlNodePtr node, GtkWidget *window)
{
    if (strcmp((char *)node->name, "option") == 0) {
        // Use g_object_get_data() instead of g_object_get_data() in GTK3
        GtkWidget *button = (GtkWidget*)g_object_get_data(G_OBJECT(window),
                                                          get_option_name(cur_info, node, NULL, 0));
        if (button) {
            gtk_widget_set_sensitive(button, TRUE);
        }
    }

    // Recursively enable child nodes
    node = XML_CHILDREN(node);
    while (node) {
        enable_tree(node, window);
        node = node->next;
    }
}

gboolean on_manpage_entry_focus_out_event(GtkWidget *widget,
										  GdkEventFocus *event,
										  gpointer user_data)
{
    const char* string;
    string = gtk_entry_get_text( GTK_ENTRY(widget) );
    if ( string ) {
        set_manpath(cur_info, string);
        if ( strcmp(string, cur_info->man_path) != 0 ) {
            gtk_entry_set_text(GTK_ENTRY(widget), cur_info->man_path);
        }
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
**  on_use_binary_toggled
**      Signal function to repsond to a toggle state in the
** 'Use symbolic link' checkbox.
**---------------------------------------------------------------------------*/
void on_use_binary_toggled ( GtkWidget* widget, gpointer func_data)
{
    GtkWidget *binary_path_widget;
    GtkWidget *binary_label_widget;
    GtkWidget *binary_entry;
    const char *string = NULL;

    /*-------------------------------------------------------------------------
    ** Pick up widget handles
    **-----------------------------------------------------------------------*/
    binary_path_widget = get_widget( "binary_path");
    binary_label_widget = get_widget( "binary_label");

    /*-------------------------------------------------------------------------
    ** Mark the appropriate widgets active or inactive
    **-----------------------------------------------------------------------*/
    gboolean is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(binary_path_widget, is_active);
    gtk_widget_set_sensitive(binary_label_widget, is_active);

    /*-------------------------------------------------------------------------
    ** Finally, set the symlinks_path.  If we've made it active
    **      again, we have to go get the current binary entry box
    **      value and restash it into the global symlinkspath.
    **-----------------------------------------------------------------------*/
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
      GtkWidget *binary_entry = get_widget( "binary_entry") ;
      const gchar *string = gtk_entry_get_text(GTK_ENTRY(binary_entry));
    } else {
      string = NULL;
    }
    set_symlinkspath(cur_info, string ? string : "");
    check_install_button();
}


void setup_checkbox_option_slot( GtkWidget* widget, gpointer func_data)
{
	GtkWidget *window;
	xmlNodePtr node, data_node = (xmlNodePtr) func_data; 
	
	if(!data_node)
		return;
	
	window = get_widget( "setup_window");

	//if ( GTK_TOGGLE_BUTTON(widget)->active ) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		const char *warn = get_option_warn(cur_info, data_node);

		/* does this option require a seperate EULA? */
		xmlNodePtr child;
		child = XML_CHILDREN(data_node);
		while(child)
		{
			if (!strcmp((char *)child->name, "eula"))
			{
				/* this option has some EULA nodes
				 * we need to prompt before this change can be validated / turned on
				 */
				const char* name = GetProductEULANode(cur_info, data_node, NULL);
				if (name)
				{
					GtkWidget *license;
					GtkWidget *license_widget;


if (!setup_interface_license) {
    setup_interface_license = gtk_builder_new();
    gtk_builder_add_from_file(setup_interface_license, glade_file, NULL);
}

// Automatically connect signals defined in the UI file
gtk_builder_connect_signals(setup_interface_license, NULL);

// Retrieve the license dialog and license area widgets
license = get_widget_license( "license_dialog" );
license_widget = get_widget_license( "license_area" );
					if ( license && license_widget ) {
						install_state start;
						
						gtk_widget_hide(license);
						load_file_gtk3(GTK_TEXT_VIEW(license_widget), name);

						gtk_widget_show(license);
						gtk_window_set_modal(GTK_WINDOW(license), TRUE);
						
						start = cur_state; /* happy hacking */
						license_okay = 0;
						iterate_for_state();
						cur_state = start;

						gtk_widget_hide(license);
						if (!license_okay)
						{
							/* the user doesn't accept the option EULA, leave this option disabled */
							license_okay = 1; /* put things back in order regarding the product EULA */
							gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
							return;
						}
						license_okay = 1;
						break;
					}
				}
				else
				{
					log_warning("option-specific EULA not found, can't set option on\n");
					/* EULA not found 	or not accepted */
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
					return;
				}
			}
			child = child->next;
		}
		
		if ( warn && !in_setup ) { /* Display a warning message to the user */
			gtkui_prompt(warn, RESPONSE_OK);
		}

		/* Mark this option for installation */
		mark_option(cur_info, data_node, "true", 0);
		
		/* Recurse down any other options to re-enable grayed out options */
		node = XML_CHILDREN(data_node);
		while ( node ) {
			enable_tree(node, window);
			node = node->next;
		}
	} else {
		/* Unmark this option for installation */
		mark_option(cur_info, data_node, "false", 1);
		
		/* Recurse down any other options */
		node = XML_CHILDREN(data_node);
		while ( node ) {
			if ( !strcmp((char *)node->name, "option") ) {
				GtkWidget *button;
				
				button = (GtkWidget*)g_object_get_data((window),get_option_name(cur_info, node, NULL, 0));
				if(button){ /* This recursively calls this function */
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
					gtk_widget_set_sensitive(button, FALSE);
				}
			} else if ( !strcmp((char *)node->name, "exclusive") ) {
				xmlNodePtr child;
				for ( child = XML_CHILDREN(node); child; child = child->next) {
					GtkWidget *button;
					
					button = (GtkWidget*)g_object_get_data((window),get_option_name(cur_info, child, NULL, 0));
					if(button){ /* This recursively calls this function */
						gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
						gtk_widget_set_sensitive(button, FALSE);
					}
				}
			}
			node = node->next;
		}
	}
    cur_info->install_size = size_tree(cur_info, XML_CHILDREN(XML_ROOT(cur_info->config)));
	update_size();
}

#if 0 // gtk1/2
void setup_checkbox_menuitems_slot( GtkWidget* widget, gpointer func_data)
{
    cur_info->options.install_menuitems = (GTK_TOGGLE_BUTTON(widget)->active != 0);
}
#endif
void setup_checkbox_menuitems_slot(GtkWidget* widget, gpointer func_data)
{
    cur_info->options.install_menuitems = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static yesno_answer prompt_response;

static void prompt_button_slot( GtkWidget* widget, gpointer func_data)
{
    prompt_response = RESPONSE_YES;
}

static void prompt_yesbutton_slot( GtkWidget* widget, gpointer func_data)
{
    prompt_response = RESPONSE_YES;
}

static void prompt_nobutton_slot( GtkWidget* widget, gpointer func_data)
{
    prompt_response = RESPONSE_NO;
}

static void prompt_okbutton_slot( GtkWidget* widget, gpointer func_data)
{
    prompt_response = RESPONSE_OK;
}

static yesno_answer gtkui_prompt(const char *txt, yesno_answer suggest)
{
	GtkWidget *dialog;
#ifdef ENABLE_GTK2
	gint ret;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, 
									(suggest != RESPONSE_OK) ? GTK_MESSAGE_QUESTION : GTK_MESSAGE_INFO,
									(suggest != RESPONSE_OK) ? GTK_BUTTONS_YES_NO : GTK_BUTTONS_OK,
									txt);
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (ret==GTK_RESPONSE_YES || ret==GTK_RESPONSE_OK)
		return (suggest==RESPONSE_OK) ? RESPONSE_OK : RESPONSE_YES;
	return RESPONSE_NO;

#else
    GtkWidget *label, *yes_button, *no_button, *ok_button;
 
    /* Create the widgets */
 
    dialog = gtk_dialog_new();
    label = gtk_label_new (txt);
	gtk_misc_set_padding(GTK_MISC(label), 8, 8);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    ok_button = gtk_button_new_with_label(_("OK"));

    prompt_response = RESPONSE_INVALID;
    
    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    
    gtk_signal_connect_object ((ok_button), "clicked",
                               GTK_SIGNAL_FUNC (prompt_okbutton_slot), (dialog));

	gtk_signal_connect_object(GTK_OBJECT(dialog), "delete-event",
							  GTK_SIGNAL_FUNC(prompt_nobutton_slot), GTK_OBJECT(dialog));

    if (suggest != RESPONSE_OK) {
		yes_button = gtk_button_new_with_label(_("Yes"));
		no_button = gtk_button_new_with_label(_("No"));

		gtk_signal_connect_object (GTK_OBJECT (yes_button), "clicked",
								   GTK_SIGNAL_FUNC (prompt_yesbutton_slot), GTK_OBJECT(dialog));
		gtk_signal_connect_object (GTK_OBJECT (no_button), "clicked",
								   GTK_SIGNAL_FUNC (prompt_nobutton_slot), GTK_OBJECT(dialog));

        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                           yes_button);
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                           no_button);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Choice Requested"));
    } else {
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                           ok_button);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Message"));
    }
    
    /* Add the label, and show everything we've added to the dialog. */
    
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                       label);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);

    while ( prompt_response == RESPONSE_INVALID ) {
        gtk_main_iteration();
    }
    gtk_widget_destroy(dialog);
    return prompt_response;
#endif
}


static void message_dialog(const char *txt, const char *title)
{
    GtkWidget *dialog;
#ifdef ENABLE_GTK2
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, 
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_OK,
									txt);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
#else
	GtkWidget *label, *ok_button;
 
    /* Create the widgets */
 
    dialog = gtk_dialog_new();
    label = gtk_label_new (txt);
    ok_button = gtk_button_new_with_label("OK");

    prompt_response = RESPONSE_NO;

    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    
    gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
                               GTK_SIGNAL_FUNC (prompt_button_slot), GTK_OBJECT(dialog));

	gtk_signal_connect_object(GTK_OBJECT(dialog), "delete-event",
							  GTK_SIGNAL_FUNC(prompt_button_slot), GTK_OBJECT(dialog));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
					   ok_button);

    /* Add the label, and show everything we've added to the dialog. */
    
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                       label);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show_all (dialog);
	while ( prompt_response != RESPONSE_YES ) {
	    gtk_main_iteration();
	}

    gtk_widget_destroy(dialog);	
#endif
}


static inline int str_in_g_list(const char *str, GList *list)
{
    /* See if the item is already in our list */
    int i;
    char *path_elem;
    for ( i=0; (path_elem= (char *) g_list_nth_data(list, i)) != NULL; ++i ) {
        if ( strcmp(path_elem, str) == 0 ) {
            return 1;  /* it's in the list. */
        }
    }

    return 0;  /* not in the list. */
}

// Determine if there is a program to auto start
//   That is, if we don't want to make a symlink, but
//   we want to start something as an option at the end
//   the installation, let's enable that.

static void check_program_to_start(install_info *info)
{
    xmlNodePtr node;

    /*----------------------------------------------------------------------
    **  Find a program to start, if any, by traversing the config's children.
    **----------------------------------------------------------------------*/
    for (node = XML_CHILDREN(XML_ROOT(cur_info->config)); node; node = node->next) {
        if (strcmp((char *)node->name, "program_to_start") == 0) {
            // Retrieve the program content, making sure to free the memory later.
            char *content = (char *)xmlNodeGetContent(node);
            if (content) {
                // Warn if symlinks are present since program_to_start is invalid in this case.
                if (info->installed_symlink && info->symlinks_path && *info->symlinks_path) {
                    log_warning(_("Warning: program_to_start is only meaningful when there are no symlinks.\n"));
                    g_free(content);
                    return;
                }

                // Strip any leading/trailing whitespace from the content.
                g_strstrip(content);

                // Replace $INSTALLDIR with the actual install path.
                char *content_ptr = content;
                char *binary_ptr = info->play_binary;
 
                while ((binary_ptr - info->play_binary) < (PATH_MAX - 20) && *content_ptr) {
                    if (memcmp(content_ptr, "$INSTALLDIR", 11) == 0) {
                        // Copy the install path to the destination buffer.
                        strcpy(binary_ptr, info->install_path);
                        binary_ptr += strlen(info->install_path);
                        content_ptr += 11;

                        // Avoid double slashes if the install path and content both have a '/'
                        if (*(binary_ptr - 1) == '/' && *content_ptr == '/')
                            content_ptr++;
                    } else {
                        // Copy regular characters.
                        *binary_ptr++ = *content_ptr++;
                    }
                }

                // Free the retrieved content and return.
                g_free(content);
                return;
            }
        }
    }
}

// FIXME: this does not belong into the UI
static void init_install_path(void)
{
    GtkWidget *install_path_widget;
    GList *install_path_list = NULL;
    int path_count = 0;
    char expanded_path[PATH_MAX];
    xmlNodePtr config_node;
    char *home_directory = getenv("HOME");

    // Get the install path widget from GtkBuilder
    install_path_widget = get_widget( "install_path") ;

    // If the product is a meta-installer, hide the install path widget
    if (GetProductIsMeta(cur_info)) {
        gtk_widget_hide(install_path_widget);
        return;
    }

    // Add the current install path to the list if it is writable
    if (access(cur_info->install_path, W_OK) == 0) {
        install_path_list = g_list_append(install_path_list, cur_info->install_path);
    }

    /*----------------------------------------------------------------------
    **  Retrieve the list of install paths from the configuration file.
    **----------------------------------------------------------------------*/
    for (config_node = XML_CHILDREN(XML_ROOT(cur_info->config)); config_node; config_node = config_node->next) {

        if (strcmp((char *)config_node->name, "install_drop_list") == 0) {
            // Get the content of the install_drop_list node
            char *install_paths_content = (char *)xmlNodeGetContent(config_node);

            if (install_paths_content) {
                // Tokenize the content to retrieve individual install paths
                char *token = strtok(install_paths_content, "\n\t \r\b");

                while (token) {
                    // Expand any "~" characters to the home directory
                    char *temp_buf = malloc(PATH_MAX);
                    if (!temp_buf) {
                        fprintf(stderr, _("Fatal error: out of memory\n"));
                        return;
                    }
                    expand_home(cur_info, token, temp_buf);

                    // Ensure we don't exceed the maximum number of install paths
                    if (path_count >= MAX_INSTALL_PATHS - 2) {
                        fprintf(stderr, _("Error: maximum of %d install_path entries exceeded\n"), MAX_INSTALL_PATHS - 2);
                        free(temp_buf);
                        goto enough_of_config;
                    }

                    // Add the expanded path to the install paths array
                    install_paths[path_count++] = temp_buf;

                    // Get the next token
                    token = strtok(NULL, "\n\t \r\b");
                }
                g_free(install_paths_content); // Free the content after use
            }
        }
    }

enough_of_config:

    /*----------------------------------------------------------------------
    **  If no installation paths were specified, use the default
    **      values that are hard coded in.
    **--------------------------------------------------------------------*/
    if (path_count == 0) {
        for (path_count = 0; default_install_paths[path_count]; ++path_count) {
            install_paths[path_count] = default_install_paths[path_count];
	}
    }

    /*----------------------------------------------------------------------
    **  Add in the home directory, as a last-resort.
    **--------------------------------------------------------------------*/
    if (home_directory != NULL)
        install_paths[path_count++] = home_directory;

    /*----------------------------------------------------------------------
    **  Terminate the array
    **--------------------------------------------------------------------*/
    install_paths[path_count] = NULL;

    /*----------------------------------------------------------------------
    **  Now translate the default install paths into the gtk list,
    **      avoiding the current default value (which is already in the list)
    **--------------------------------------------------------------------*/
    for ( path_count=0; install_paths[path_count]; ++path_count ) {
        snprintf(path_count, sizeof(path_count), "%s/%s", install_paths[path_count], GetProductName(cur_info));
        if ((!str_in_g_list(expanded_path, install_path_list)) && (access(install_paths[path_count], W_OK) == 0)) {
            install_path_list = g_list_append( install_path_list, strdup(expanded_path));
        }
    }

// Assuming install_path_list is a GList* containing the items for the combo box.
GList *iter;
GtkWidget *combo_box = gtk_combo_box_text_new();

for (iter = install_path_list; iter != NULL; iter = iter->next) {
    const char *install_path = (const char *)iter->data;
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), install_path);
}

// Set the combo box as your new install_path_widget
install_path_widget = combo_box;

    /* !!! FIXME: Should we g_list_free ( install_path_list ) or not? */
    
    /*gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(install_path_widget)->entry), cur_info->install_path );*/
//    gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(install_path_widget)->entry), list->data );

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(install_path_widget), install_path_list->data);

    // Uncomment if edit needed
    //GtkEntry *entry = gtk_bin_get_child(GTK_BIN(install_path_widget));
    //gtk_entry_set_text(entry, list->data);

    /* cheat. Make the first entry the default for IsReadyToInstall(). */
    if(cur_info->install_path != install_path_list->data)
		strncpy(cur_info->install_path, install_path_list->data, sizeof (cur_info->install_path));

}

static void init_man_path(void)
{
    GList* list = NULL;
	GtkWidget *widget;
    char pathCopy[ 4069 ];
    const char *path;

	if ( ! GetProductHasManPages(cur_info) )
		return;

    path = getenv( "MANPATH" );
    if( path )
    {
        int len;
        char* pc0;
        char* pc;
        int sc;
        int end = 0;

        pc = pathCopy;
        strncpy( pathCopy, path, sizeof (pathCopy) - 1 );
        pathCopy[sizeof (pathCopy) - 1] = '\0';  /* just in case. */

        while ( *pc ) {
            pc0 = pc;
            len = 0;
            while( *pc != ':' && *pc != '\0' ) {
                len++;
                pc++;
            }
            if( *pc == '\0' )
                end = 1;
            else
                *pc = '\0';

            if( len && ((sc=strcmp( pc0, cur_info->man_path)) != 0) && (*pc0 != '.') ) {
				if ((!str_in_g_list(pc0, list)) && (access(pc0, W_OK) == 0)) {
					list = g_list_append( list, pc0 );
				}
            }

            if( ! end )
                pc++;
        }
    } 
	if ( ! list ) { /* At least these default values */
		list = g_list_append(list, "/usr/local/man");
		list = g_list_append(list, "/usr/local/share/man");
		list = g_list_append(list, "/usr/share/man");
		list = g_list_append(list, "/usr/man");
	}

        // Retrieve the manpage_combo widget from GtkBuilder
        widget = get_widget( "manpage_combo" );

        // Ensure the widget is a GtkComboBoxText and append items
        if (GTK_IS_COMBO_BOX_TEXT(widget)) {
           GList *iter;
           for (iter = list; iter != NULL; iter = iter->next) {
              gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), (const gchar *)iter->data);
           }
        }

        set_manpath(cur_info, list->data);
}

static void init_binary_path(void)
{
    char pathCopy[4096], resolved[PATH_MAX];
    GtkWidget* widget;
    GList* list;
    char* path;
    int change_default = TRUE;

    widget = get_widget( "binary_path" );

    if ( GetProductIsMeta(cur_info) ) {
		gtk_widget_hide(widget);
		return;
    }

    list = 0;

    if ( access(cur_info->symlinks_path, W_OK) == 0 ) {
        list = g_list_append( list, cur_info->symlinks_path );
        change_default = FALSE;
    }

    path = (char *)getenv( "PATH" );
    if( path )
    {
        int len;
        char* pc0;
        char* pc;
        int sc;
        int end = 0;

        pc = pathCopy;
        strncpy( pathCopy, path, sizeof (pathCopy) - 1 );
        pathCopy[sizeof (pathCopy) - 1] = '\0';  /* just in case. */

        while( *pc != '\0' ) {
            pc0 = pc;
            len = 0;
            while( *pc != ':' && *pc != '\0' ) {
                len++;
                pc++;
            }
            if( *pc == '\0' )
                end = 1;
            else
                *pc = '\0';

            if( len && ((sc=strcmp( pc0, cur_info->symlinks_path)) != 0) && (*pc0 != '.') ) {
				if ((!str_in_g_list(pc0, list)) && (access(pc0, W_OK) == 0)) {
					list = g_list_append( list, g_strdup(realpath(pc0, resolved)) );
				}
            }

            if( ! end )
                pc++;
        }
    }

    path = (char *)getenv( "HOME" );
    if( path ) {
	    if ((!str_in_g_list(path, list)) && (access(path, W_OK) == 0)) {
		    list = g_list_append( list, g_strdup(realpath(path, resolved)) );
		}
    }

if (list) {
    GList *iter;

    // Clear the existing combo box items if necessary
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));

    // Iterate through the list and add each item to the combo box
    for (iter = list; iter != NULL; iter = iter->next) {
        const char *item = (const char *)iter->data;
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), item);
    }
}

    if ( change_default && list && g_list_length(list) ) {
        set_symlinkspath(cur_info, g_list_nth(list,0)->data);
    }

    if ((list == NULL || g_list_length(list) == 0) && change_default)
    {
        log_warning(_("Warning: No writable targets in path... You may want to be root.\n"));
        /* FIXME */
    }

    /* !!! FIXME: Should we g_list_free ( list ) or not? */
    //gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(widget)->entry), cur_info->symlinks_path );
    GtkEntry *entry = gtk_bin_get_child(GTK_BIN(widget));
    gtk_entry_set_text(entry, cur_info->symlinks_path);

    // or

    //gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), cur_info->symlinks_path);

}

static void init_menuitems_option(install_info *info)
{
    GtkWidget *widget;

    widget = get_widget( "setup_menuitems_checkbox" );
    if (widget) {
        if ((!GetProductHasNoBinaries(info)) && (GetProductInstallMenuItems(info))) {
            setup_checkbox_menuitems_slot(widget, NULL);
        } else {
            gtk_widget_hide(widget);
            info->options.install_menuitems = 0;
        }
    } else {
        log_warning(_("Unable to locate 'setup_menuitems_checkbox'"));
    }
}

static void parse_option(install_info *info, const char *component, xmlNodePtr node,
						 GtkWidget *window, GtkWidget *box, int level, GtkWidget *parent,
						 int exclusive, int excl_reinst, GSList **radio)
{
    xmlNodePtr child;
    char text[1024] = "";
    const char *help;
	char *wanted;
    gchar *name;
    int i;
    GtkWidget *button = NULL;
	gboolean install = FALSE;

	/* Skip translation nodes */
	if ( !strcmp((char *)node->name, "lang") )
		return;

    /* See if this node matches the current architecture */
    wanted = (char *)xmlGetProp(node, BAD_CAST "arch");
    if ( ! match_arch(info, wanted) ) {
		xmlFree(wanted);
        return;
    }
	xmlFree(wanted);
		
    wanted = (char *)xmlGetProp(node, BAD_CAST "libc");
    if ( ! match_libc(info, wanted) ) {
		xmlFree(wanted);
        return;
    }
	xmlFree(wanted);

    wanted = (char *)xmlGetProp(node, BAD_CAST "distro");
    if ( ! match_distro(info, wanted) ) {
		xmlFree(wanted);
        return;
    }
	xmlFree(wanted);

    wanted = (char *)xmlGetProp(node, BAD_CAST "if");
    if ( ! match_condition(wanted) ) {
		xmlFree(wanted);
        return;
    }
	xmlFree(wanted);

    if ( ! get_option_displayed(info, node) ) {
		return;
    }

    /* See if the user wants this option */
	if ( node->type == XML_TEXT_NODE ) {
		log_debug("Parsing text node: '%s'\n", node->content);
		name = g_strdup((gchar *)node->content);
		g_strstrip(name);
		if ( *name ) {
			log_debug("String: '%s'\n", name);
			button = gtk_label_new(get_option_name(info, node->parent, NULL, 0));
			gtk_widget_show(button);
			gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(button), FALSE, FALSE, 0);
		}
		g_free(name);
		return;
	} else {
		name = get_option_name(info, node, NULL, 0);
		for(i=0; i < (level*5); i++)
			text[i] = ' ';
		text[i] = '\0';
		strncat(text, name, sizeof(text)-strlen(text));
	}

	log_debug("Parsing option: '%s'\n", text);
	if ( GetProductIsMeta(info) ) {
		button = gtk_radio_button_new_with_label(radio_list, text);
		radio_list = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	} else if ( exclusive ) {
		button = gtk_radio_button_new_with_label(*radio, text);
		*radio = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
	} else {
		button = gtk_check_button_new_with_label(text);
	}

        /* Add tooltip help, if available */
           help = get_option_help(info, node);
        if (help) {
           gtk_widget_set_tooltip_text(button, help);
        }

    /* Set the data associated with the button */
	if ( button ) {
		g_object_set_data((button), "data", (gpointer)node);

		/* Register the button in the window's private data */
		window = get_widget( "setup_window");
		g_object_set_data((window), name, (gpointer)button);
	}

    /* Check for required option */
    if ( xmlNodePropIsTrue(node, "required") ) {
		xmlSetProp(node, BAD_CAST "install", BAD_CAST "true");
		gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    }

    /* If this is a sub-option and parent is not active, then disable option */
    install = xmlNodePropIsTrue(node, "install");
    if( level>0 && GTK_IS_TOGGLE_BUTTON(parent) && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(parent)) ) {
		install = FALSE;
		gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    }
         if (button) {
                gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
                g_signal_connect(button, "toggled", G_CALLBACK(setup_checkbox_option_slot), (gpointer)node);
		gtk_widget_show(button);
	}

	if ( install ) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    } else {
        /* Unmark this option for installation */
        mark_option(info, node, "false", 1);
    }

    /* Recurse down any other options */
    child = XML_CHILDREN(node);
    while ( child ) {
		if ( !strcmp((char *)child->name, "option") ) {
			parse_option(info, component, child, window, box, level+1, button, 0, 0, NULL);
		} else if ( !strcmp((char *)child->name, "exclusive") ) {
			xmlNodePtr exchild;
			GSList *list = NULL;
			int reinst = GetReinstallNode(info, child);

			for ( exchild = XML_CHILDREN(child); exchild; exchild = exchild->next) {
				parse_option(info, component, exchild, window, box, level+1, button, 1, reinst, &list);
			}
		}
		child = child->next;
    }

    /* Reinstallation - Disable any options that are already installed */
    if ( info->product ) {
		product_component_t *comp;
		if ( component ) {
			comp = loki_find_component(info->product, component);
		} else {
			comp = loki_getdefault_component(info->product);
		}
		if ( exclusive ) {
			/* Reinstall an exclusive option - make sure to select the installed one */
			gtk_widget_set_sensitive(button, excl_reinst);
			if ( comp && loki_find_option(comp, name) ) {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
				mark_option(info, node, "true", 1);
			} else {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
				mark_option(info, node, "false", 1);
			}
		} else if ( ! GetProductReinstall(info) ) {			
			if ( comp && loki_find_option(comp, name) ) {
				/* Unmark this option for installation */
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
				gtk_widget_set_sensitive(button, FALSE);
				mark_option(info, node, "false", 1);
			}
		} else if (!GetReinstallNode(info, node)) {
			/* Unmark this option for installation, unless it was not installed already */
			gtk_widget_set_sensitive(button, FALSE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
			mark_option(info, node, "false", 1);
		}
    }
}

#if 0 // gtk1/2
static void update_image(const char *image_file, gboolean left)
{
    GtkWidget* window;
    GtkWidget* frame;
    GdkPixmap* pixmap;
	GdkBitmap* mask;
    GtkWidget* image;
	char image_path[PATH_MAX] = SETUP_BASE;

	strncat(image_path, image_file, sizeof(image_path)-strlen(image_path));
	image_path[sizeof(image_path)-1] = '\0';

	if(left)
		frame = get_widget( "image_frame");
	else
		frame = get_widget( "image_frame_top");

	g_return_if_fail(frame != NULL);

    gtk_container_remove(GTK_CONTAINER(frame), GTK_BIN(frame)->child);
    window = gtk_widget_get_toplevel(frame);
    pixmap = gdk_pixmap_create_from_xpm(window->window, &mask, NULL, image_path);
    if ( pixmap ) {
        image = gtk_pixmap_new(pixmap, mask);
        gtk_widget_show(image);
        gtk_container_add(GTK_CONTAINER(frame), image);
        gtk_widget_show(frame);
    } else {
        gtk_widget_hide(frame);
    }
}
#endif
static void update_image(const char *image_file, gboolean left)
{
    GtkWidget *window;
    GtkWidget *frame;
    GtkWidget *image;
    GdkPixbuf *pixbuf;
    char image_path[PATH_MAX] = SETUP_BASE;

    // Construct full image path
    strncat(image_path, image_file, sizeof(image_path) - strlen(image_path));
    image_path[sizeof(image_path) - 1] = '\0';

    // Get the correct frame widget using GtkBuilder
    if (left)
        frame = get_widget( "image_frame" );
    else
        frame = get_widget( "image_frame_top" );

    // Ensure the frame exists
    g_return_if_fail(frame != NULL);

    // Remove the current child from the frame
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(frame));
    if (child) {
        gtk_container_remove(GTK_CONTAINER(frame), child);
    }

    // Load the image from file
    pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);

    if (pixbuf) {
        // Create a new image widget with the pixbuf
        image = gtk_image_new_from_pixbuf(pixbuf);
        gtk_widget_show(image);

        // Add the new image to the frame
        gtk_container_add(GTK_CONTAINER(frame), image);
        gtk_widget_show(frame);
    } else {
        // Hide the frame if loading the image failed
        gtk_widget_hide(frame);
    }
}


static void log_handler(const gchar *log_domain,
					 GLogLevelFlags log_level,
					 const gchar *message,
					 gpointer user_data)
{
	log_debug("Glib-WARNING(%s): %s\n", log_domain, message);
}

/********** UI functions *************/

static install_state gtkui_init(install_info *info, int argc, char **argv, int noninteractive)
{
    FILE *opened;
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *widget;
    GtkWidget *button;
    GtkWidget *install_path, *install_entry, *binary_path, *binary_entry;
    char title[1024];

    cur_state = SETUP_INIT;
    cur_info = info;

#ifdef ENABLE_GTK2
   // Turn off any themes
   setenv("GTK2_RC_FILES", "", 1);
   setenv("GTK_DATA_PREFIX", "", 1);
#endif
    
    gtk_init(&argc,&argv);

	if (getenv("SETUP_GLADE_FILE"))
	{
		glade_file = getenv("SETUP_GLADE_FILE");
	}

    /* Glade segfaults if the file can't be read */
    opened = fopen(glade_file, "r");
    if ( opened == NULL ) {
        fprintf(stderr, _("Unable to open %s, aborting!\n"), glade_file);
        return SETUP_ABORT;
    }
    fclose(opened);

    // Use GtkBuilder instead of GladeXML to load the UI
    setup_interface = gtk_builder_new();
      if (!gtk_builder_add_from_file(setup_interface, "loki_interface.gtk3.ui", NULL)) {
      g_print("Failed to load UI file\n");
    }

gtk_builder_add_from_file(setup_interface, glade_file, NULL);

// Retrieve the install_path and set up the combo box
install_path = get_widget( "install_path" );
install_entry = get_widget( "install_entry" );

// Connect signals to manage enabling/disabling the Install button
g_signal_connect_after(install_path, "changed", G_CALLBACK(path_combo_change_slot), NULL);
g_signal_connect_after(install_entry, "key_press_event", G_CALLBACK(path_entry_keypress_slot), NULL);

// Retrieve the binary_path and set up the combo box
binary_path = get_widget( "binary_path" );
binary_entry = get_widget( "binary_entry" );

// Connect signals for binary_path and binary_entry
g_signal_connect_after(binary_path, "changed", G_CALLBACK(binary_path_combo_change_slot), NULL);
g_signal_connect_after(binary_entry, "key_press_event", G_CALLBACK(binary_path_entry_keypress_slot), NULL);

// Auto-connect other signal handlers defined in the UI file
gtk_builder_connect_signals(setup_interface, NULL);


#if 0 /* Sam 8/22 - I don't think this is necessary */
    GtkWidget *symlink_checkbox;
    /*-------------------------------------------------------------------------
    ** Connect a signal handle to control whether or not the symlink
    **  should be installed
    **------------------------------------------------------------------------*/
    symlink_checkbox = get_widget( "symlink_checkbox");
    gtk_signal_connect(GTK_OBJECT(symlink_checkbox), "toggled",
			   GTK_SIGNAL_FUNC(on_use_binary_toggled), NULL);
#endif /*0*/

// Set up the window title
window = get_widget( "setup_window" );
if (info->component) {
    snprintf(title, sizeof(title), _("%s / %s Setup"), info->desc, GetProductComponent(info));
} else {
    snprintf(title, sizeof(title), _("%s Setup"), info->desc);
}
gtk_window_set_title(GTK_WINDOW(window), title);

// Set the initial state for the notebook
notebook = get_widget( "setup_notebook" );

if (noninteractive) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), COPY_PAGE);
} else if (GetProductAllowsExpress(info)) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), CLASS_PAGE);

    // Workaround for weird GTK behavior on older systems
    widget = get_widget( "class_continue" );
    gtk_widget_set_sensitive(widget, FALSE);
} else {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), OPTION_PAGE);
}

// Force the window to redraw
gtk_widget_queue_draw(window);

// Disable the "View Readme" button if no README available
if (!GetProductREADME(cur_info, NULL)) {
    // Get the button widgets and modify their properties
    button = get_widget( "button_readme" );
    gtk_widget_set_sensitive(button, FALSE);

    button = get_widget( "view_readme_progress_button" );
    gtk_widget_hide(button);

    button = get_widget( "view_readme_end_button" );
    gtk_widget_hide(button);

    button = get_widget( "class_readme" );
    gtk_widget_hide(button);
}

// Set the text for some blank labels
widget = get_widget( "current_option_label" );
if (widget) {
    gtk_label_set_text(GTK_LABEL(widget), "");
}

widget = get_widget( "current_file_label" );
if (widget) {
    gtk_label_set_text(GTK_LABEL(widget), "");
}

// Disable useless widgets for meta-installer
if (GetProductIsMeta(info)) {
    widget = get_widget( "global_frame" );
    if (widget) {
        gtk_widget_hide(widget);
    }

    widget = get_widget( "label_free_space" );
    if (widget) {
        gtk_widget_hide(widget);
    }

    widget = get_widget( "free_space_label" );
    if (widget) {
        gtk_widget_hide(widget);
    }

    widget = get_widget( "label_install_size" );
    if (widget) {
        gtk_widget_hide(widget);
    }

    widget = get_widget( "estim_size_label" );
    if (widget) {
        gtk_widget_hide(widget);
    }

    widget = get_widget( "install_separator" );
    if (widget) {
        gtk_widget_hide(widget);
    }
}


    /* Disable the path fields if they were provided via command line args */
    if (disable_install_path) {
        widget = get_widget( "install_path");
		gtk_widget_set_sensitive(widget, FALSE);
    }
    if (disable_binary_path) {
        widget = get_widget( "binary_path");
		gtk_widget_set_sensitive(widget, FALSE);
    }
	if (GetProductHasNoBinaries(info)) {
        widget = get_widget( "binary_path");
		if(widget) gtk_widget_hide(widget);
		widget = get_widget( "binary_label");
		if(widget) gtk_widget_hide(widget);
		widget = get_widget( "setup_menuitems_checkbox");
		if(widget) gtk_widget_hide(widget);
	}
	if ( !GetProductHasManPages(info) ) {
        widget = get_widget( "manpage_combo");
		if(widget) gtk_widget_hide(widget);
        widget = get_widget( "manpage_label");
		if(widget) gtk_widget_hide(widget);
	}
	/*--------------------------------------------------------------------
	**  Hide the checkbox allowing the user to pick whether or
	**      not to install a symlink to the binaries if they
	**      haven't asked for that feature.
	**------------------------------------------------------------------*/
	if (GetProductHasNoBinaries(info) || (!GetProductHasPromptBinaries(info))) {
		widget = get_widget( "symlink_checkbox");
		if (widget)	gtk_widget_hide(widget);
	}

    info->install_size = size_tree(info, XML_CHILDREN(XML_ROOT(info->config)));

    license_okay = 1; /* Needed so that Expert is detected properly at this point */
    
	/* Check if we should check "Expert" installation by default */
	if ( check_for_installation(info, NULL) ) {
		widget = get_widget( "expert_but");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
	}
    
    if ( GetProductEULA(info, NULL) ) {
        license_okay = 0;
        cur_state = SETUP_LICENSE;
    } else {
        license_okay = 1;
        cur_state = SETUP_README;
    }

    /* Realize the main window for pixmap loading */
    gtk_widget_realize(window);

    /* Update the install image */
    update_image(GetProductSplash(info), GetProductSplashPosition(info));

    /* Center the installer, it will be shown later */
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    return cur_state;
}

static install_state gtkui_license(install_info *info)
{
    GtkWidget *license;
    GtkWidget *widget;
    PangoFontDescription *font_desc;

    // Use GtkBuilder instead of GladeXML
    setup_interface_license = gtk_builder_new();
    gtk_builder_add_from_file(setup_interface_license, "loki_interface.gtk3.ui", NULL);
    gtk_builder_connect_signals(setup_interface_license, NULL);

    // Retrieve widgets
    license = GTK_WIDGET(gtk_builder_get_object(setup_interface_license, "license_dialog"));
    widget = GTK_WIDGET(gtk_builder_get_object(setup_interface_license, "license_area"));

    if (license && widget) {
        // Use PangoFontDescription instead of GdkFont
        font_desc = pango_font_description_from_string(LICENSE_FONT);
        gtk_widget_override_font(widget, font_desc);
        pango_font_description_free(font_desc);

        // Load the EULA text (assuming you have a GTK3 version of load_file_gtk2)
        load_file_gtk3(GTK_TEXT_VIEW(widget), GetProductEULA(info, NULL));

        // Hide and show the license dialog
        gtk_widget_hide(license);
        gtk_widget_show(license);

        // Make the dialog modal
        gtk_window_set_modal(GTK_WINDOW(license), TRUE);

        iterate_for_state();
    } else {
        cur_state = SETUP_README;
    }
    return cur_state;
}

static install_state gtkui_readme(install_info *info)
{
    if ( GetProductAllowsExpress(info) ) {
		cur_state = SETUP_CLASS;
    } else {
		cur_state = SETUP_OPTIONS;
    }
    return cur_state;
}

static install_state gtkui_pick_class(install_info *info)
{
	/* Enable the Continue button now */
	GtkWidget *widget = get_widget( "class_continue");

	/* Make sure the window is being shown */
    gtk_widget_show(get_widget( "setup_window"));

	gtk_widget_set_sensitive(widget, TRUE);
	return iterate_for_state();
}

static void gtkui_idle(install_info *info)
{
#ifdef ENABLE_GTK2
	/*	if (g_main_context_pending(NULL)) 
	  g_main_context_iteration(NULL, FALSE); */
	/*
    if( gtk_events_pending() ) {
        gtk_main_iteration();
		}  */
#else
    while( gtk_events_pending() == TRUE) {
        gtk_main_iteration();
    }
#endif
}


static install_state gtkui_setup(install_info *info)
{
    GtkWidget *window;
    GtkWidget *options;
    xmlNodePtr node;

    // Use GtkBuilder to get the setup window
    window = get_widget( "setup_window" );

    // Make sure the window is being shown
    gtk_widget_show(window);

    // Set paths regardless of whether we are in express or not
    init_install_path();
    init_binary_path();
    init_man_path();

    if (express_setup) {
        GtkWidget *notebook = get_widget( "setup_notebook" );
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), COPY_PAGE);

        // Must hide cancel button if reinstalling/upgrading, or install can be mangled
        if (info->options.reinstalling) {
            GtkWidget *button = get_widget( "cancel_progress_button" );
            gtk_widget_hide(button);
        }

        return cur_state = SETUP_INSTALL;
    }

    // Go through the install options
    options = get_widget( "option_vbox");
    gtk_container_foreach(GTK_CONTAINER(options), empty_container, options);
    info->install_size = 0;
    node = XML_CHILDREN(XML_ROOT(info->config));
    radio_list = NULL;
    in_setup = TRUE;

    while (node) {
        if (!strcmp((char *)node->name, "option")) {
            parse_option(info, NULL, node, window, options, 0, NULL, 0, 0, NULL);
        } else if (!strcmp((char *)node->name, "exclusive")) {
            xmlNodePtr child;
            GSList *list = NULL;
            int reinst = GetReinstallNode(info, node);

            for (child = XML_CHILDREN(node); child; child = child->next) {
                parse_option(info, NULL, child, window, options, 0, NULL, 1, reinst, &list);
            }
        } else if (!strcmp((char *)node->name, "component")) {
            char *arch = (char *)xmlGetProp(node, BAD_CAST "arch");
            char *libc = (char *)xmlGetProp(node, BAD_CAST "libc");
            char *distro = (char *)xmlGetProp(node, BAD_CAST "distro");
            char *cond = (char *)xmlGetProp(node, BAD_CAST "if");

            if (match_arch(info, arch) &&
                match_libc(info, libc) &&
                match_distro(info, distro) &&
                match_condition(cond)) {
                xmlNodePtr child;
                char *name = (char *)xmlGetProp(node, BAD_CAST "name");

                if (xmlGetProp(node, BAD_CAST "showname")) {
                    GtkWidget *widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
                    gtk_box_pack_start(GTK_BOX(options), widget, FALSE, FALSE, 0);
                    gtk_widget_show(widget);

                    widget = gtk_label_new(name);
                    gtk_box_pack_start(GTK_BOX(options), widget, FALSE, FALSE, 10);
                    gtk_widget_show(widget);
                }

                for (child = XML_CHILDREN(node); child; child = child->next) {
                    if (!strcmp((char *)child->name, "option")) {
                        parse_option(info, name, child, window, options, 0, NULL, 0, 0, NULL);
                    } else if (!strcmp((char *)child->name, "exclusive")) {
                        xmlNodePtr child2;
                        GSList *list = NULL;
                        int reinst = GetReinstallNode(info, child);

                        for (child2 = XML_CHILDREN(child); child2; child2 = child2->next) {
                            parse_option(info, name, child2, window, options, 0, NULL, 1, reinst, &list);
                        }
                    }
                }
                xmlFree(name);
            }
            xmlFree(arch);
            xmlFree(libc);
            xmlFree(distro);
            xmlFree(cond);
        }
        node = node->next;
    }

    // Update size and space, and initialize menu items
    update_size();
    update_space();
    init_menuitems_option(info);

    in_setup = FALSE;

    return iterate_for_state();
}

static int gtkui_update(install_info *info, const char *path, size_t progress, size_t size, const char *current)
{
    static gfloat last_update = -1.0;
    static gdouble last_installed_bytes = -1.0;
    GtkWidget *widget;
    int textlen;
    const char *text;
    char *install_path;
    gfloat new_update;
    static GTimeVal ltv = { 0, 0 };
    GTimeVal tv;

    if ( cur_state == SETUP_ABORT ) {
		return FALSE;
    }

    if ( progress && size ) {
        new_update = (gfloat)progress / (gfloat)size;
    } else { /* "Running script" */
        new_update = 1.0;
    }

    g_get_current_time(&tv);

    if( (int)(new_update*100) != 100) {
		if(tv.tv_sec == ltv.tv_sec
				&& tv.tv_usec - ltv.tv_usec < 50000) { /* 50ms */
			return TRUE;
		}
		else if(tv.tv_sec == ltv.tv_sec+1
				&& tv.tv_usec + (1000000-ltv.tv_usec) < 50000) { /* 50ms */
			return TRUE;
		}
    }
	ltv = tv;

    if ( ( (int)(new_update*100) != (int)(last_update*100) ) || ( last_installed_bytes !=  (gdouble)info->installed_bytes ) ) {
        if ( new_update == 1.0 ) {
            last_update = 0.0;
        } else {
            last_update = new_update;
        }
        widget = get_widget( "current_option_label");
        if ( widget ) {
            gtk_label_set_text( GTK_LABEL(widget), current);
        }
        widget = get_widget( "current_file_label");
        if ( widget ) {
            text = path;
            /* Remove the install path from the string */
            install_path = cur_info->install_path;
            if ( strncmp(text, install_path, strlen(install_path)) == 0 ) {
                text+=strlen(install_path)+1;
            }
            textlen = strlen(text);
            if ( textlen > MAX_TEXTLEN ) {
                text+=textlen-MAX_TEXTLEN;
            }
            gtk_label_set_text( GTK_LABEL(widget), text);
        }
        widget = get_widget( "current_file_progress");

//        gtk_progress_bar_update(GTK_PROGRESS_BAR(widget), new_update);
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), new_update );

        new_update = (gdouble)info->installed_bytes / (gdouble)info->install_size;
		last_installed_bytes=(gdouble)info->installed_bytes;
		if (new_update > 1.0) {
			new_update = 1.0;
		} else if (new_update < 0.0) {
			new_update = 0.0;
		}
        widget = get_widget( "total_file_progress");

//        gtk_progress_bar_update(GTK_PROGRESS_BAR(widget), new_update);
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), new_update );


    }
	gtkui_idle(info);
	return TRUE;
}

static void gtkui_abort(install_info *info)
{
    GtkWidget *notebook, *w;

	/* No point in waiting for a change of state if the window is not there */
	w = get_widget( "setup_window");
	if ( !w || ! gtk_widget_get_visible(w) )
		return;

    if ( setup_interface ) {
        notebook = get_widget( "setup_notebook");
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), ABORT_PAGE);
        iterate_for_state();
		gtk_widget_hide(w);
    } else {
        fprintf(stderr, _("Unable to open %s, aborting!\n"), SETUP_GLADE);
    }
}

static install_state gtkui_website(install_info *info)
{
    GtkWidget *notebook;
    GtkWidget *widget;
    GtkWidget *hideme;
    const char *website_text;
    int do_launch;

    notebook = get_widget( "setup_notebook");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), WEBSITE_PAGE);

    /* Add the proper product text */
    widget = get_widget( "website_product_label");
    gtk_label_set_text(GTK_LABEL(widget), GetProductDesc(info));

    /* Add special website text if desired */
    website_text = GetWebsiteText(info);
    if ( website_text ) {
        widget = get_widget( "website_text_label");
        gtk_label_set_text(GTK_LABEL(widget), website_text);
    }

    /* Hide the proper widget based on the auto_url state */
    do_launch = 0;
    if ( strcmp(GetAutoLaunchURL(info), "true") == 0 ) {
        do_launch = 1;
        hideme = get_widget( "auto_url_no");
    } else {
        do_launch = 0;
        hideme = get_widget( "auto_url_yes");
    }
    gtk_widget_hide(hideme);

    /* Automatically launch the browser if necessary */
    if ( do_launch ) {
        launch_browser(info, loki_launchURL);
    }
    return iterate_for_state();
}

static install_state gtkui_complete(install_info *info)
{
    GtkWidget *widget;
    char text[1024];

    widget = get_widget( "setup_notebook");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), DONE_PAGE);
    widget = get_widget( "install_directory_label");
    gtk_label_set_text(GTK_LABEL(widget), info->install_path);

    check_program_to_start(info);
    widget = get_widget( "play_game_label");
    if ( info->installed_symlink && info->symlinks_path && *info->symlinks_path ) {
        snprintf(text, sizeof(text), _("Type '%s' to start the program"), info->installed_symlink);
    }
    else if ( *info->play_binary ) {
        snprintf(text, sizeof(text), _("Type '%s' to start the program"), info->play_binary);
    } else {
		*text = '\0';
    }
    gtk_label_set_text(GTK_LABEL(widget), text);

    /* Hide the play game button if there's no game to play. :) */
    widget = get_widget( "play_game_button");
    if ( widget && 
		 (!info->installed_symlink || !info->symlinks_path || !*info->symlinks_path ) && ! *info->play_binary) {
        gtk_widget_hide(widget);
    }

    /* Hide the 'View Readme' button if we have no readme... */
    if ( ! GetProductREADME(info, NULL) ) {
        widget = get_widget( "view_readme_end_button");
        if(widget)
            gtk_widget_hide(widget);
    }

    widget = get_widget( "setup_complete_label");
    if (info->options.reinstalling)
        strcpy(text, _("The update/reinstall was successfully completed!"));
    else
        strcpy(text, _("The installation was successfully completed!"));

    gtk_label_set_text(GTK_LABEL(widget), text);

    /* TODO: Lots of cleanups here (free() mostly) */

    return iterate_for_state();
}

static void gtkui_shutdown(install_info *info)
{
    /* Destroy all windows */
    GtkWidget *window = get_widget( "setup_window");

    gtk_widget_hide(window);
    if ( setup_interface_readme ) {
		window = get_widget_readme( "readme_dialog" );
		gtk_widget_hide(window);
    }
    if ( setup_interface_license ) {
		window = get_widget_license( "license_dialog");
		gtk_widget_hide(window);
    }

    /* This seems to work better on GTK2 */
    while( gtk_events_pending() == TRUE) {
        gtk_main_iteration();
    }
}

int gtkui_okay(Install_UI *UI, int *argc, char ***argv)
{
    extern int force_console;
    int okay;

    okay = 0;
    if ( !force_console ) {
        /* Try to open a GTK connection */
        if( gtk_init_check(argc, argv) ) {
            /* Set up the driver */
            UI->init = gtkui_init;
            UI->license = gtkui_license;
            UI->readme = gtkui_readme;
            UI->setup = gtkui_setup;
            UI->update = gtkui_update;
            UI->abort = gtkui_abort;
            UI->prompt = gtkui_prompt;
            UI->website = gtkui_website;
            UI->complete = gtkui_complete;
			UI->pick_class = gtkui_pick_class;
			UI->idle = gtkui_idle;
			UI->exit = NULL;
			UI->shutdown = gtkui_shutdown;
			UI->is_gui = 1;

            okay = 1;
        }
    }
    return(okay);
}

#ifdef STUB_UI
int console_okay(Install_UI *UI, int *argc, char ***argv)
{
    return(0);
}

int carbonui_okay(Install_UI *UI, int *argc, char ***argv)
{
    return(0);
}

#ifdef ENABLE_DIALOG
int dialog_okay(Install_UI *UI, int *argc, char ***argv)
{
    return(0);
}
#endif

#endif




