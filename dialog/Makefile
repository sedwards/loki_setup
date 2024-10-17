# $Id: Makefile.in,v 1.6 2008-07-25 22:56:53 megastep Exp $
# template makefile for DIALOG
#
SHELL		= /bin/sh

VERSION		= $(shell cat VERSION)

CFLAGS		= -fsigned-char -funroll-loops -Wall -g -O2 -I.  -I/usr/X11/include -I/opt/homebrew/opt/ncurses/include -Ddarwin -Wno-implicit-function-declaration -Wno-incompatible-pointer-types -Wno-pointer-sign -Wno-unused-but-set-variable -Wno-int-conversion -Wno-deprecated-declarations -Wno-dangling-else -Wno-unused-variable -Wno-unused-function -Wno-int-to-pointer-cast  -I/Users/sedwards/source/loki_setup/../loki_setupdb -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi -I/opt/homebrew/Cellar/glade/3.40.0_1/include/libgladeui-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi  -DENABLE_DIALOG -DZIP_SUPPORT -DUZ2_SUPPORT -DHAVE_COLOR -I. -I..
CPPFLAGS	= -I/opt/homebrew/opt/ruby/include -DPACKAGE_NAME=\"setup\" -DPACKAGE_TARNAME=\"setup\" -DPACKAGE_VERSION=\"1.6.5\ \#\ configure.ac\ -\ loki_setup\ AC_CONFIG_HEADERS\(config.h\)\ AM_GNU_GETTEXT\(external\)\ AM_INIT_AUTOMAKEforeign\" -DPACKAGE_STRING=\"setup\ 1.6.5\ \#\ configure.ac\ -\ loki_setup\ AC_CONFIG_HEADERS\(config.h\)\ AM_GNU_GETTEXT\(external\)\ AM_INIT_AUTOMAKEforeign\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DHAVE_STRINGS_H=1 -DHAVE_PWD_H=1 -DHAVE_CARBON_CARBON_H=1 -DHAVE_DIRENT_H=1 -DHAVE_SYS_DIRENT_H=1 -DHAVE_SYS_DIR_H=1 -DHAVE_SYS_STATVFS_H=1 -DHAVE_SYS_MOUNT_H=1 -DHAVE_SYS_UCRED_H=1 -DHAVE_FSTAB_H=1 -DHAVE_UTIL_H=1 -DHAVE_LIBGEN_H=1 -DHAVE_GETOPT_H=1 -DHAVE__DEV_PTMX=1 -DHAVE_LANGINFO_H=1 -DHAVE_LOCALE_H=1 -DHAVE_NCURSES_TERM_H=1 -DHAVE_NCURSES_CURSES_H=1 -DHAVE_NCURSES_H=1 -DENABLE_gtk3=1 -DENABLE_ZIP=1 -DENABLE_UZ2=1 -DLOCALEDIR=\"locale\" -DVERSION=\"$(VERSION)\"
CC		= gcc
LDFLAGS		= -L/opt/homebrew/opt/ruby/lib -L/opt/homebrew/opt/ncurses/lib -framework Cocoa -framework Carbon -framework Security -framework IOKit
LIBS		=   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz
RANLIB		= ranlib
AR		= /usr/bin/ar

RM		= rm -f
LINT		= lint

#
# Standard .c to .o compile line.
#
.c.o :
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -c $<

OBJS = 	arrows.o \
	buttons.o \
	checklist.o \
	inputbox.o \
	inputstr.o \
	menubox.o \
	mouse.o \
	mousewget.o \
	msgbox.o \
	textbox.o \
	ui_getc.o \
	util.o \
	gauge.o \
	fselect.o \
	timebox.o \
	yesno.o

SRCS = $(OBJS:.o=.c)
HDRS = \
	colors.h \
	dialog.h

LIB	= libdialog.a
ALL	= $(LIB) $(PROG)

all	:: $(ALL)

dialog.o $(OBJS) : dialog.h

$(LIB) : $(OBJS)
	$(AR) crv $(LIB) $(OBJS)
	$(RANLIB) $@

clean:
	rm -f core *.o *~ tags TAGS $(ALL)

tags:
	ctags $(SRCS) $(HDRS)

TAGS:
	etags $(SRCS) $(HDRS)

lint:
	$(LINT) $(CPPFLAGS) *.c


depend:
	$(CC) -MM $(CFLAGS) $(SRCS) > .depend

ifeq ($(wildcard .depend),.depend)
include .depend
endif
