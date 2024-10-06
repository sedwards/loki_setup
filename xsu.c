/* * C o p y r i g h t *(also read COPYING)* * * * * * * * * * * * * * * * * * *
 *                                                                             *
 *  Copyright (C) 2002  <Philip Van Hoof>                                      *
 *                                                                             *
 *  This program is free software; you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by       *
 *  the Free Software Foundation; either version 2 of the License, or          *
 *  (at your option) any later version.                                        *
 *                                                                             *
 *  This program is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *  GNU General Public License for more details.                               *
 *                                                                             *
 *  You should have received a copy of the GNU General Public License          *
 *  along with this program; if not, write to the Free Software                *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
 *                                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

#define DBG
#define GTK2
#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <ctype.h>
#include <termios.h>

#include "xsu.h"

static int term = -1;
static FILE *fterm = NULL;
static pid_t pid = 0;

static gint
exec_su_failed (gpointer user_data)
{
    printf("static gint exec_su_failed (gpointer user_data)\n");
    return FALSE;
}

typedef void SigFunc(int);

static SigFunc *Signal(int signo, SigFunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if ( signo == SIGALRM ) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
    }
    if ( sigaction(signo, &act, &oact) < 0 )
		return SIG_ERR;
    return oact.sa_handler;
}

void sighandler(int sig)
{
	fprintf(stderr, "Signal %d received, term = %d.\n", sig, term);
	if ( sig == SIGCHLD && term > 0 ) {
		int status;
		pid_t pid;
		pid = wait(&status);
		if ( WIFEXITED(status) ) {
			fprintf(stderr,"Child %d returned\n", pid);
		} else if ( WIFSIGNALED(status) ) { 
			fprintf(stderr,"Xsu: subprocess %d has died from signal %d\n", pid, WTERMSIG(status));
		}
		close(term); term = -1;
		
		while (gtk_events_pending ())
			gtk_main_iteration ();
		gtk_main_quit ();
	}
}

/* Execute an external command on a pty without going through system() */
int exec_program(const char *prog, char *argv[], pid_t *pid)
{
	int  fd = -1;
	pid_t child;
	char slavename[PATH_MAX];
	struct termios *stdin_termios = NULL;
    struct winsize stdin_winsize;
	int havewin = 0;

	if (isatty(STDIN_FILENO))
		havewin = ioctl(STDIN_FILENO, TIOCGWINSZ, &stdin_winsize) != -1;

	if (isatty(STDIN_FILENO) ) {
		stdin_termios = malloc(sizeof(struct termios));
		if ( tcgetattr(STDIN_FILENO, stdin_termios) < 0) {
			perror("tcgetattr");
			free(stdin_termios);
			stdin_termios = NULL;
		} else {
			/* stdin_termios->c_lflag |= ICANON;
			   stdin_termios->c_oflag &= ~ONLCR; */
			stdin_termios->c_iflag &= ~(ISTRIP | IGNCR | INLCR | IXOFF | ECHO);
			stdin_termios->c_iflag |= (ICRNL | IGNPAR | BRKINT | IXON);
			stdin_termios->c_cflag &= ~CSIZE;
			stdin_termios->c_cflag |= CREAD | CS8;
		}
	}

	Signal(SIGCHLD, sighandler);

	switch( child = pty_fork(&fd, slavename, sizeof(slavename), stdin_termios, havewin ? &stdin_winsize : NULL) ) {
	case 0:
		execvp(prog, argv);
		fprintf(stderr, "execv(%s): %s\n", prog, strerror(errno));
		_exit(1);
	case -1:
		perror("pty_fork");
		return -1;
	default:
		fprintf(stderr,"Started program %s, fd = %d (%s), PID = %d\n", prog, fd, slavename, child);
		if (pid)
			*pid = child;
		break;
	}
	free(stdin_termios);
	return fd;
}

void xsu_perform()
{ 
/* 
	Some code from gnomesu, thanks to Hongli Lai <hongli@telekabel.nl> 
*/
	guint timeout;
	gchar *password= g_strdup(gtk_entry_get_text (GTK_ENTRY (gtk_password_textbox)));

	const gchar *username=gtk_entry_get_text (GTK_ENTRY (gtk_user_textbox)),
		  *command=gtk_entry_get_text (GTK_ENTRY (gtk_command_textbox));
	gchar *buffer;
	char *argv[6], c, line[120];
	fd_set fds;
	struct timeval delay = { 0, 10*1000 }; /* 10 ms wait */

    printf("void xsu_perform()\n");

/* 0.2.1 *
	Minor security fix. Password would remain in memory if we don't
	clean the password textbox.
*/
	gtk_entry_set_text(GTK_ENTRY(gtk_password_textbox),"");
	gtk_widget_hide (gtk_xsu_window);
	
    while( gtk_events_pending() ) {
        gtk_main_iteration();
    }

	if (password == NULL)
		return;


/* xsu 0.2.1 *
	Option to set the DISPLAY environment
	system() has been replaced with execlp()
   xsu 0.2.2 execlp() has been replaced with execl()
	This also fixes the problem that the zvt
	widget could not get the child_died signal.
*/
	if (set_display_env)
	  buffer = g_strdup_printf(SET_DISPL_ENV "%s", displ_host, command);
	else		
	  buffer = g_strdup_printf("%s", command);
/* xsu 0.2.2 *
	Security issue noticed by Havoc 
	Security fix, execlp exports the PATH which is a security problem. SU_PATH contains
	the full path of the su binary found by the configure script.
*/
	/* buffer with " and/or ' gives some problems. */
	/* buffer = g_strdup_printf("%s - %s -c %s",SU_PATH, username, buffer); */
	
	/* su - username -c command */
	argv[0] = su_command;
	argv[1] = (char *) username;
	argv[2] = "-c";
	argv[3] = buffer;
	argv[4] = NULL;
	term = exec_program(su_command, argv, &pid);
	/* We are not a gnome-application anymore but under control of su */

	if ( term < 0 ) {
		perror ("Could not exec");
                exit(1);
	}

	timeout = g_timeout_add (SU_DELAY, exec_su_failed, NULL);
	for (;;)
	{
		char buf[80], *ptr;
		int cnt, ret;

		/* We can not use fscanf as there might not be a space
		   character after the initial prompt, hence hanging */
		/* fscanf(fd_term, "%80s", buf); */
		ptr = buf;
		cnt = 0;
		do {
		  ret = read(term, &c, 1);

		  if ( c == EOF || ret == 0 || isspace((int)c) ) {
		    *ptr ++ = '\0';
		    break;
		  } else {
		    *ptr ++ = c;
		  }
		  cnt++;
		  if ( cnt==SU_PWD_LEN && !strncmp(buf, SU_PWD_OUT, SU_PWD_LEN) ) {
		    *ptr ++ = '\0';
		    break;
		  }
		} while ( cnt < sizeof(buf));
		
 		fprintf(stderr,"reading from pty: '%s'\n", buf);

		if (strncmp (buf, SU_PWD_OUT, SU_PWD_LEN) == 0)
		{
			g_source_remove(timeout);
			break;
		}

		while (gtk_events_pending ()) 
		  gtk_main_iteration ();
		usleep (5);
	}

#ifdef DEBUG
	fprintf(stderr, "Got it!\n");
#endif

	/* Discard any remaining characters on stdin */
	for(;;) {
		FD_ZERO(&fds);
		FD_SET(term, &fds);
		if ( select(term+1, &fds, NULL, NULL, &delay) ) {
		    if ( read(term, &c, 1) <= 0 ) {
				fprintf(stderr, "'%c'", c);
				break;
			}
		} else
			break;
	}

	fterm = fdopen(term, "w+");
	setbuf(fterm, NULL);
 	fprintf(stderr,"Sending password: '%s'\n", password);
/* 0.2.2 *
	Minor security fix, clear the password from memory
*/  
   	fprintf(fterm, "%s\n", password);
	fflush(fterm);

	/* Clear out all the lines of output, we may want to have an option to show those */
	while ( !feof(fterm) ) {
		if ( fgets(line, sizeof(line), fterm) ) {
			fprintf(stderr,"Read %d chars: %s", strlen(line), line);
		}
	}
	g_free(password);
	password = NULL;
}


void on_gtk_password_textbox_activate (GtkButton *button, gpointer user_data)
{
    printf("on_gtk_user_textbox_activate (GtkButton *button, gpointer user_data)\n");
    xsu_perform();
}

void on_gtk_user_textbox_activate (GtkButton *button, gpointer user_data)
{
    printf("on_gtk_user_textbox_activate (GtkButton *button, gpointer user_data)\n");
    gtk_widget_grab_focus (gtk_password_textbox);
}

void on_gtk_command_textbox_activate (GtkButton *button, gpointer user_data)
{
    printf("on_gtk_command_textbox_activate (GtkButton *button, gpointer user_data)\n");
        if (gtk_widget_get_visible(gtk_user_textbox))
		gtk_widget_grab_focus (gtk_user_textbox);
	else
		gtk_widget_grab_focus (gtk_password_textbox);
}

void on_gtk_cancel_button_clicked (GtkButton *button, gpointer user_data)
{
    printf("on_gtk_cancel_button_clicked (GtkButton *button, gpointer user_data)\n");
    /* return 3 to tell setup that the user willingly aborted */
}


void on_gtk_ok_button_clicked (GtkButton *button, gpointer user_data)
{
    printf("on_gtk_ok_button_clicked (GtkButton *button, gpointer user_data)\n");
    xsu_perform();
}

GtkWidget* create_gtk_pixmap(const char *file)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(file, &error);
    if (!pixbuf) {
        g_print("Error loading XPM file: %s\n", error->message);
        g_clear_error(&error);
        return NULL;
    }
    
    return gtk_image_new_from_pixbuf(pixbuf);
}

GtkWidget* create_gtk_pixmap_d(char *xpm[])
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)xpm);
    if (!pixbuf) {
        g_print("Error loading XPM data\n");
        return NULL;
    }
    
    return gtk_image_new_from_pixbuf(pixbuf);
}

GtkWidget* create_gtk_xsu_window (void)
{
  gtk_xsu_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (gtk_xsu_window, "gtk_xsu_window");
  g_object_set_data (G_OBJECT (gtk_xsu_window), "gtk_xsu_window", gtk_xsu_window);

  if (title_in)
	gtk_window_set_title (GTK_WINDOW (gtk_xsu_window), arg_title);
  else 
	gtk_window_set_title (GTK_WINDOW (gtk_xsu_window), _("Login Prompt"));
	
  gtk_window_set_position (GTK_WINDOW (gtk_xsu_window), GTK_WIN_POS_CENTER);

  //gtk_window_set_policy (GTK_WINDOW (gtk_xsu_window), FALSE, FALSE, FALSE);
  gtk_window_set_resizable(GTK_WINDOW(gtk_xsu_window), FALSE);  // Make window non-resizable

  gtk_window_table = gtk_table_new (6, 2, FALSE);
  gtk_widget_set_name (gtk_window_table, "gtk_window_table");
  g_object_ref (gtk_window_table);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_window_table", gtk_window_table,
  //                          (GtkDestroyNotify) gtk_widget_unref);

  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_window_table", gtk_window_table, (GDestroyNotify) g_object_unref);


  gtk_widget_show (gtk_window_table);
  gtk_container_add (GTK_CONTAINER (gtk_xsu_window), gtk_window_table);

  gtk_password_label = gtk_label_new (_("Username : "));
  gtk_widget_set_name (gtk_password_label, "gtk_password_label");
  g_object_ref (gtk_password_label);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_password_label", gtk_password_label,
    //                        (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_password_table", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_password_label);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_password_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (gtk_password_label), 4.91738e-07, 0);
  gtk_misc_set_padding (GTK_MISC (gtk_password_label), 24, 8);

  gtk_commandtxt_label = gtk_label_new (_("Command :"));
  gtk_widget_set_name (gtk_commandtxt_label, "gtk_commandtxt_label");
  g_object_ref (gtk_commandtxt_label);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_commandtxt_label", gtk_commandtxt_label,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_commandtxt_label", gtk_window_table, (GDestroyNotify) g_object_unref);


  gtk_widget_show (gtk_commandtxt_label);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_commandtxt_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (gtk_commandtxt_label), 0, 7.45058e-09);
  gtk_misc_set_padding (GTK_MISC (gtk_commandtxt_label), 24, 6);

  if (message_in)
    gtk_text_label = gtk_label_new (_(arg_message));
  else
    gtk_text_label = gtk_label_new (_("This installer requires root privileges.\nPlease enter the correct password for it\nbelow and press [Return] or click OK."));
  
  gtk_widget_set_name (gtk_text_label, "gtk_text_label");
  g_object_ref (gtk_text_label);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_text_label", gtk_text_label,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_text_label", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_text_label);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_text_label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (gtk_text_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gtk_text_label), 0, 7.45058e-09);
  gtk_misc_set_padding (GTK_MISC (gtk_text_label), 9, 14);

  gtk_keys_pixmap = create_gtk_pixmap_d (keys_xpm);

  gtk_widget_set_name (gtk_keys_pixmap, "gtk_keys_pixmap");
  g_object_ref (gtk_keys_pixmap);
  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_keys_pixmap", gtk_keys_pixmap,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_keys_pixmap", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_keys_pixmap);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_keys_pixmap, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_password_textbox = gtk_entry_new ();
  gtk_widget_set_name (gtk_password_textbox, "gtk_password_textbox");
  g_object_ref (gtk_password_textbox);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_password_textbox", gtk_password_textbox,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_password_textbox", gtk_window_table, (GDestroyNotify) g_object_unref);


  gtk_widget_show (gtk_password_textbox);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_password_textbox, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);	    
  gtk_widget_set_size_request(gtk_password_textbox, 230, -1);
  gtk_widget_set_tooltip_markup ( gtk_password_textbox, _("Type the password here"));
  gtk_entry_set_visibility (GTK_ENTRY (gtk_password_textbox), FALSE);

  gtk_user_textbox = gtk_entry_new ();
  gtk_widget_set_name (gtk_user_textbox, "gtk_user_textbox");
  g_object_ref (gtk_user_textbox);

//  gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_user_textbox", gtk_user_textbox,
//                            (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_user_textbox", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_user_textbox);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_user_textbox, 1, 2, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_size_request( gtk_user_textbox, 230, -2);
  gtk_widget_set_tooltip_markup ( gtk_user_textbox, _("Type the username here"));

  gtk_command_textbox = gtk_entry_new ();
  gtk_widget_set_name (gtk_command_textbox, "gtk_command_textbox");
  g_object_ref (gtk_command_textbox);
  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_command_textbox", gtk_command_textbox,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_command_textbox", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_command_textbox);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_command_textbox, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_size_request( gtk_command_textbox, 230, -2);
  gtk_widget_set_tooltip_markup  ( gtk_command_textbox, _("Type the command here") );
/* xsu 0.2.3 *
	The buttons and their size in other languages have been fixed. This
	bug was filed by Robert Millan <zeratul2@wanadoo.es>
*/
  gtk_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (gtk_hbox);

  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_hbox, 1, 2, 5, 6,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 7);

  // Create a horizontal box instead of GtkHButtonBox
  GtkWidget *gtk_utilbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

  // Pack the new box into the parent horizontal box
  gtk_box_pack_start(GTK_BOX(gtk_hbox), gtk_utilbox, FALSE, TRUE, 14);

  // Show the utilbox
  gtk_widget_show(gtk_utilbox);

  // Create the OK button
  GtkWidget *gtk_ok_button = gtk_button_new_with_label(_("OK"));
  gtk_box_pack_start(GTK_BOX(gtk_utilbox), gtk_ok_button, FALSE, FALSE, 5);

  // Set the OK button as the default
  gtk_widget_set_can_default(gtk_ok_button, TRUE);
  gtk_widget_grab_default(gtk_ok_button);

  // Show the OK button
  gtk_widget_show(gtk_ok_button);

  // Set the tooltip for the OK button
  gtk_widget_set_tooltip_text(gtk_ok_button, _("Press this button when finished"));

  // Create the Cancel button
  GtkWidget *gtk_cancel_button = gtk_button_new_with_label(_("Cancel"));
  gtk_box_pack_start(GTK_BOX(gtk_utilbox), gtk_cancel_button, FALSE, FALSE, 5);

  // Show the Cancel button
  gtk_widget_show(gtk_cancel_button);

  // Set the Cancel button as a default candidate (though not grabbing default)
  gtk_widget_set_can_default(gtk_cancel_button, TRUE);

  // Set the tooltip for the Cancel button
  gtk_widget_set_tooltip_text(gtk_cancel_button, _("Press this button to Cancel"));

  gtk_txtuser_label = gtk_label_new (_("Password :"));
  gtk_widget_set_name (gtk_txtuser_label, "gtk_txtuser_label");
  g_object_ref (gtk_txtuser_label);
  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_txtuser_label", gtk_txtuser_label,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_txtuser_label", gtk_window_table, (GDestroyNotify) g_object_unref);

  gtk_widget_show (gtk_txtuser_label);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_txtuser_label, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (gtk_txtuser_label), 0, 7.45058e-09);
  gtk_misc_set_padding (GTK_MISC (gtk_txtuser_label), 24, 2);

  gtk_separator_one = gtk_hseparator_new ();
  gtk_widget_set_name (gtk_separator_one, "gtk_separator_one");
  g_object_ref (gtk_separator_one);

  //gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_separator_one", gtk_separator_one,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_separator_one", gtk_window_table, (GDestroyNotify) g_object_unref);


  gtk_widget_show (gtk_separator_one);
  gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_separator_one, 0, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 8);


  g_signal_connect (G_OBJECT (gtk_user_textbox), "activate",
			G_CALLBACK (on_gtk_user_textbox_activate), gtk_user_textbox);
		      
		      
  g_signal_connect (G_OBJECT (gtk_password_textbox), "activate",
			G_CALLBACK (on_gtk_password_textbox_activate), gtk_password_textbox);


  g_signal_connect (G_OBJECT (gtk_command_textbox), "activate",
			G_CALLBACK (on_gtk_command_textbox_activate), gtk_command_textbox);
		            
		      
  g_signal_connect (G_OBJECT (gtk_cancel_button), "clicked",
                      G_CALLBACK (on_gtk_cancel_button_clicked),
                      NULL);

  g_signal_connect (G_OBJECT (gtk_ok_button), "clicked",
                      G_CALLBACK (on_gtk_ok_button_clicked),
                      NULL);

  return gtk_xsu_window;
}

int main (int argc, char *argv[])
{
	GtkWidget *gtk_xsu_window;
	gchar 	  *arg_command = "",
			  *arg_username = "root",
			  *arg_iconfile = "";
	gint 	  x;
	gboolean  command_in=FALSE, 
			  username_in=FALSE,
			  icon_in=FALSE, 
			  dis_textboxes=FALSE,
			  hide_textboxes=FALSE,
			  endofargs=FALSE,
			  first=TRUE;

	/* Set the locale */
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

    /*	Nope..no popt, maybe next time. Oh and getopt stuff is not
		the same in *BSD as with GNU getopt, so I don't immedialty 
		feel like wanting to change this option parsing :-\. 

		Some problems about parsing the arguments have been filed by
		Jeff Licquia <licquia@debian.org>. I added the "--" option
		which stops parsing. (Only " and \ have to be escaped of course,
		this depends on your shell)
	*/

	for (x=0; x<argc; x++)
	{
/* xsu 0.2.3 *
		Argument "--" stops argument parsing and collects all arguments
		after the "--" argument. It will put all these arguments in one
		string and use this string in arg_command.
*/
		if (!strcmp (argv[x], "--")) endofargs=TRUE;
			
		if (endofargs) {
			gchar *buffer;
			buffer = g_strdup(argv[x+1]);
			if (buffer) {
				if (first) {
					arg_command = g_strdup_printf("%s",buffer);
					first=FALSE;
				} else {
					arg_command = g_strdup_printf("%s %s", arg_command, buffer);
				}
				continue;
			} else {
				command_in=TRUE;
				break;
			}
		}
		
		if ((!strcmp (argv[x], "-c")) || (!strcmp (argv[x], "--command")))
		{
			if (argv[x+1] != NULL)
			{
				arg_command = g_strdup(argv[x+1]);
				command_in=TRUE;
			}
		}

		if ((!strcmp (argv[x], "-s")) || (!strcmp (argv[x], "--su-command")))
		{
			if (argv[x+1] != NULL)
			{
				su_command = g_strdup(argv[x+1]);
			}
		}
    
		if ((!strcmp (argv[x], "-u")) || (!strcmp (argv[x], "--username")))
		{
			if (argv[x+1] != NULL)
			{	
				arg_username = g_strdup(argv[x+1]);
				username_in=TRUE;
			}
		}

		if ((!strcmp (argv[x], "-m")) || (!strcmp (argv[x], "--message")))
		{
			if (argv[x+1] != NULL)
			{	
				int i;
				arg_message = g_strdup(argv[x+1]);
				for (i=0;i<strlen(arg_message);++i)
					if (arg_message[i]=='^') arg_message[i]='\n';
				for (i=0;i<strlen(arg_message);++i)
					if (arg_message[i]=='~') arg_message[i]='\t';
				message_in=TRUE;
			}
		}
/* xsu 0.1.5 *
	Option to load another icon
*/	
		if ((!strcmp (argv[x], "-i")) || (!strcmp (argv[x], "--icon")))
		{
			if (argv[x+1] != NULL)
			{
				arg_iconfile = g_strdup(argv[x+1]);
				icon_in=TRUE;
			}
		}
/* xsu 0.1.6 *
	Option to set another window title
*/	
		if ((!strcmp (argv[x], "-t")) || (!strcmp (argv[x], "--title")))
		{
			if (argv[x+1] != NULL)
			{
				arg_title = g_strdup(argv[x+1]);
				title_in=TRUE;
			}
		}

/* xsu 0.1.4 *
	Option to disable the Command and User entries.
*/
		if ((!strcmp (argv[x], "-d")) || (!strcmp (argv[x], "--unadaptable")))
		{
			dis_textboxes=TRUE;
		}

		if ((!strcmp (argv[x], "-e")) || (!strcmp (argv[x], "--hide")))
		{
			hide_textboxes=TRUE;
		}
/* xsu 0.2.1 *
	Option to set the DISPLAY environment
*/
		if ((!strcmp (argv[x], "-a")) || (!strcmp (argv[x], "--set-display")))
		{
			const char *env = getenv("DISPLAY");
			set_display_env=TRUE;
			
	    	if ((argv[x+1] != NULL) && ((argv[x+1])[0] != '-'))
	    	{ /* if there is a 2e argument and it does not begin with a dash */
				displ_host = g_strdup(argv[x+1]);
			} else if (env) {
				displ_host = g_strdup(env);
	    	} else {
				displ_host = g_strdup(":0");
	    	}
		}
		if ((!strcmp (argv[x], "-v")) || (!strcmp (argv[x], "--version")))
		{
			printf(NAME " " VERSION " by " AUTHOR "\n");
			exit(EXIT_CORRECT);
		}
	
		if ((!strcmp (argv[x], "-h")) || (!strcmp (argv[x], "--help")))
		{
			printf(NAME " " VERSION " by " AUTHOR "\n\n");
			printf("Arguments :\n\n");
			printf("\t-u|--username \"USERNAME\"\n");
			printf("\t-c|--command \"COMMAND\"\n");
			printf("\t-s|--su-command \"SU COMMAND\"\n");
			printf("\t-m|--message \"MESSAGE\"\n");
			printf("\t-i|--icon \"FILENAME\"\n");
			printf("\t-t|--title \"WINDOW TITLE\"\n");
			printf("\t-a|--set-display [\"HOSTNAME\"]\n");
			printf("\t-d|--unadaptable\n");
			printf("\t-e|--hide\n");
			printf("\t-h|--help\n\n");
			printf("ps. for \"message\" :\n");
			printf("\t* You can use ^ if you want a newline in the message string.\n");
			printf("\t* You can use ~ if you want a tab in the message string.\n\n");  
			
			exit(EXIT_CORRECT);
		}

	}


/* xsu 0.1.0 *
	This one is to confuse gnome_init() actually. I don't want
	gnome to use the program arguments. And I am to lazy to learn
	how to use popt. Well.. if you are not..modify so that it uses
	popt and send me the patch :)
	xsu 0.1.4 *
	Option (dis_textboxes) to disable the Command and User entries.
*/

	argc=1;
	/* And init GTK ... */
	if ( ! gtk_init_check(&argc, &argv) ) {
	    return 1; /* X11 not available */
	}
	gtk_xsu_window = create_gtk_xsu_window ();

	if ( !su_command ) {
		su_command = g_strdup(SU_PATH);
	}

	if (username_in) /* If there was a username argument */
	{
		gtk_entry_set_text(GTK_ENTRY(gtk_user_textbox),arg_username);
		//if (dis_textboxes)
                  // gtk_entry_set_editable(GTK_ENTRY(gtk_user_textbox), FALSE);

		if (hide_textboxes) 
		{
			gtk_widget_hide(gtk_user_textbox);
			gtk_widget_hide(gtk_password_label);
		}
	} else {
		gtk_widget_grab_focus (gtk_user_textbox);
	}
	if (command_in) /* If there was a command argument */
	{
		gtk_entry_set_text(GTK_ENTRY(gtk_command_textbox),arg_command);
		//if (dis_textboxes)
                  //  gtk_entry_set_editable(GTK_ENTRY(gtk_command_textbox), FALSE);
		if (hide_textboxes) 
		{ 
			gtk_widget_hide(gtk_command_textbox);
			gtk_widget_hide(gtk_commandtxt_label);
		}
	} else {
		gtk_widget_grab_focus (gtk_command_textbox);
	}
	if ((command_in) && (username_in)) /* if both */
	{
		gtk_widget_grab_focus (gtk_password_textbox);
	}
/* xsu 0.1.5 *
	Option to load another icon
*/
	if (icon_in) {
		gtk_widget_destroy(gtk_keys_pixmap);
		gtk_keys_pixmap = create_gtk_pixmap(arg_iconfile);

		gtk_widget_set_name (gtk_keys_pixmap, "gtk_keys_pixmap");
		g_object_ref (gtk_keys_pixmap);

		//gtk_object_set_data_full (G_OBJECT (gtk_xsu_window), "gtk_keys_pixmap", gtk_keys_pixmap,
		//			(GtkDestroyNotify) gtk_widget_unref);
		g_object_set_data_full(G_OBJECT(gtk_xsu_window), "gtk_keys_pixmap", gtk_window_table, (GDestroyNotify) g_object_unref);

		gtk_widget_show (gtk_keys_pixmap);
		gtk_table_attach (GTK_TABLE (gtk_window_table), gtk_keys_pixmap, 0, 1, 0, 1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (GTK_FILL), 0, 0);
	}
	/* All done.. lets go gnome'ing :) */
	gtk_widget_show (gtk_xsu_window);
	gtk_main ();    

	/* Wait for child to finish */
	if ( pid ) {
#ifdef DEBUG
		printf("Waiting for child %d\n", pid);
#endif
		waitpid(pid, NULL, 0);
	}
	if ( fterm ) {
		fclose(fterm);
	} if ( term > 0 ) {
		close(term);
	}
	g_free(su_command);
	return 0;
}

