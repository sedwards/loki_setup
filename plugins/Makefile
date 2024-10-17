#
# Makefile for the setup plugins
# $Id: Makefile.in,v 1.7 2008-07-25 22:56:53 megastep Exp $
#


STATICS = cpio.c
PLUGINS = tar.c zip.c uz2.c
CFLAGS  = -fPIC -fsigned-char -funroll-loops -Wall -g -O2 -I.  -I/usr/X11/include -I/opt/homebrew/opt/ncurses/include -Ddarwin -Wno-implicit-function-declaration -Wno-incompatible-pointer-types -Wno-pointer-sign -Wno-unused-but-set-variable -Wno-int-conversion -Wno-deprecated-declarations -Wno-dangling-else -Wno-unused-variable -Wno-unused-function -Wno-int-to-pointer-cast  -I/Users/sedwards/source/loki_setup/../loki_setupdb -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi -I/opt/homebrew/Cellar/glade/3.40.0_1/include/libgladeui-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi  -DENABLE_DIALOG -DZIP_SUPPORT -DUZ2_SUPPORT -I..
CC	= gcc
AR	= /usr/bin/ar
DYN_PLUGINS = false

arch := arm
libc := 

.SUFFIXES: .so .o .c

OBJS	= $(STATICS:.c=.o)

ifeq ($DYN_PLUGINS,true)

# Build all plugins as .so files
SOBJS = $(PLUGINS:.c=.so)

all: $(SOBJS) libplugins.a

install: all
	for a in $(SOBJS); do \
		cp -v $$a ../image/setup.data/bin/$(arch)/$(libc)/plugins; \
		strip ../image/setup.data/bin/$(arch)/$(libc)/plugins/$$a; \
	done

install.dbg: all
	for a in $(SOBJS); do \
		cp -v $$a ../image/setup.data/bin/$(arch)/$(libc)/plugins; \
	done

else

# Make a libplugin.a archive that will be linked with the main program
OBJS += $(PLUGINS:.c=.o)

all: libplugins.a

install:
	true

install.dbg:
	true

endif

libplugins.a: $(OBJS)
	$(AR) rcs libplugins.a $(OBJS)

.c.so:
	$(CC) -shared -o $@ $*.c $(CFLAGS)
.c.o:
	$(CC) -o $@ -c $*.c $(CFLAGS)

clean:
	rm -f *.o *.so *.a *~

depend:
	$(CC) -MM $(CFLAGS) $(OBJS:.o=.c) > .depend

ifeq ($(wildcard .depend),.depend)
include .depend
endif
