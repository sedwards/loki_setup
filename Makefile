DISTDIR = ..
SETUP_VERSION = 1.6.5
PACKAGE = setup-$(SETUP_VERSION)
DATADIR = .
LOCALEDIR = locale

# 
arch := arm
libc := 
os   := Darwin
BRANDELF = true

# If you modify this version, change it's setup.xml and release a new patch.
UNINSTALL_VERSION = 1.0.4

CC = gcc
CXX = g++ -std=gnu++11
AR = /usr/bin/ar

# This indicates where the 'setupdb' CVS module is checked out
SETUPDB	= /Users/sedwards/source/loki_setup/../loki_setupdb

IMAGE = ../../spp/private/image
BRAND = loki
UPDATES = /loki/updates/loki_uninstall
CONVERT_IMAGE = /loki/patch-tools/convert-image
SETUP_GTK = setup.gtk3

# The supported locales so far
LOCALES = fr de es sv it nl ru en_GB

LD = gcc
OPTIMIZE = -fsigned-char -funroll-loops -Wall -g -O2 -I.  -I/usr/X11/include -I/opt/homebrew/opt/ncurses/include -Ddarwin -Wno-implicit-function-declaration -Wno-incompatible-pointer-types -Wno-pointer-sign -Wno-unused-but-set-variable -Wno-int-conversion -Wno-deprecated-declarations -Wno-dangling-else -Wno-unused-variable -Wno-unused-function -Wno-int-to-pointer-cast  -I/Users/sedwards/source/loki_setup/../loki_setupdb -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi -I/opt/homebrew/Cellar/glade/3.40.0_1/include/libgladeui-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/gtk+3/3.24.43/include/gtk-3.0 -I/opt/homebrew/Cellar/glib/2.82.1/include/gio-unix-2.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include -I/opt/homebrew/Cellar/libepoxy/1.5.10/include -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/pango/1.54.0/include/pango-1.0 -I/opt/homebrew/Cellar/fribidi/1.0.16/include/fribidi -I/opt/homebrew/Cellar/harfbuzz/10.0.1_1/include/harfbuzz -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/at-spi2-core/2.54.0/include/atk-1.0 -I/opt/homebrew/Cellar/cairo/1.18.2/include/cairo -I/opt/homebrew/Cellar/fontconfig/2.15.0/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libxext/1.3.6/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libx11/1.8.10/include -I/opt/homebrew/Cellar/libxcb/1.17.0/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.5/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/include/gdk-pixbuf-2.0 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/libtiff/4.7.0/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/Cellar/xz/5.6.3/include -I/opt/homebrew/Cellar/jpeg-turbo/3.0.4/include -I/opt/homebrew/Cellar/glib/2.82.1/include -I/opt/homebrew/Cellar/glib/2.82.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.82.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.44/include -I/opt/homebrew/Cellar/xorgproto/2024.1/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX14.sdk/usr/include/ffi  -DENABLE_DIALOG -DZIP_SUPPORT -DUZ2_SUPPORT
LDFLAGS = -L/opt/homebrew/opt/ruby/lib -L/opt/homebrew/opt/ncurses/lib -framework Cocoa -framework Carbon -framework Security -framework IOKit
BDYNAMIC = 
HEADERS = -I/usr/X11R6/include -I/usr/local/include
OPTIONS = -DSTUB_UI -DSETUP_VERSION_MAJOR=1 \
		  -DSETUP_VERSION_MINOR=6 \
		  -DSETUP_VERSION_RELEASE=5 \
		  -DSETUP_VERSION=\"$(SETUP_VERSION)\" -DSU_PATH=\"/usr/bin/su\" \
		  -DMOUNT_PATH=\"/sbin/mount\" -DUMOUNT_PATH=\"/sbin/umount\" \
		  -DDATADIR=\"$(DATADIR)\" -DLOCALEDIR=\"$(LOCALEDIR)\" -DLOKI_PREFIX=\"$(BRAND)\"

CFLAGS += $(OPTIMIZE) $(HEADERS) $(OPTIONS)
CXXFLAGS = $(CFLAGS)

COMMON_OBJS = log.o install_log.o
CORE_OBJS   = detect.o plugins.o network.o install.o copy.o file.o loki_launchurl.o bools.o
OBJS 		= $(COMMON_OBJS) $(CORE_OBJS) main.o 
LOKI_UNINSTALL_OBJS = loki_uninstall.o uninstall_ui.o
CARBON_UNINSTALL_OBJS = $(COMMON_OBJS) carbon_uninstall.o uninstall_carbonui.o
UNINSTALL_OBJS 		= $(COMMON_OBJS) uninstall.o
CHECK_OBJS			= $(COMMON_OBJS) $(CORE_OBJS) check.o
CHECK_CARBON_OBJS	= $(COMMON_OBJS) $(CORE_OBJS) check_carbon.o
CONSOLE_OBJS 		= $(OBJS) console_ui.o dialog_ui.o
GUI_OBJS 			= $(OBJS) gtk_ui.o
CARBON_OBJS          = $(OBJS) carbon_ui.o
XSU_OBJS			= xsu.o pseudo.o

SRCS = $(OBJS:.o=.c) $(CONSOLE_OBJS:.o=.c) $(GUI_OBJS:.o=.c) $(CARBON_OBJS:.o=.c) $(XSU_OBJS:.o=.c) testcd.c

COMMON_LIBS = plugins/libplugins.a carbon/carbonres.o carbon/carbondebug.o carbon/STUPControl.o carbon/YASTControl.o carbon/appbundle.o dialog/libdialog.a \
	      $(SETUPDB)/$(arch)/libsetupdb.a
LIBS =  $(COMMON_LIBS)   -L/lib -lxml   -L/opt/homebrew/lib  -lxml2     -lz
ifeq ($(os),Linux)
GUI_LIBS = $(COMMON_LIBS)   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz  -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -L/opt/homebrew/Cellar/glade/3.40.0_1/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgladeui-2 -lgmodule-2.0 -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl /lib/libxml2.a   -lz $(BDYNAMIC) $(BDYNAMIC)  -L/usr/X11/lib -lXi -lXext -lX11 -lm
else
GUI_LIBS = $(COMMON_LIBS)  -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -L/opt/homebrew/Cellar/glade/3.40.0_1/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgladeui-2 -lgmodule-2.0 -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl /lib/libxml2.a   -lz $(BDYNAMIC) $(BDYNAMIC)  -L/usr/X11/lib -lXi -lXext -lX11 -lm   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz
endif
CARBON_LIBS = $(COMMON_LIBS)   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz  /lib/libxml2.a   -lz $(BDYNAMIC)
CONSOLE_LIBS = $(LIBS)  -lcurses -ldl -lm

all: do-plugins do-dialog setup $(SETUP_GTK) uninstall xsu

testxml: testxml.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS) -lm

loki_uninstall.o: uninstall.c
	$(LD) $(LDFLAGS) -c -o $@ $^ $(CFLAGS) -DUNINSTALL_UI -DVERSION=\"$(UNINSTALL_VERSION)\"

loki_uninstall: $(LOKI_UNINSTALL_OBJS) $(SETUPDB)/$(arch)/libsetupdb.a
	$(LD) $(LDFLAGS) -o $@ $(LOKI_UNINSTALL_OBJS) $(GUI_LIBS)

carbon_uninstall.o: uninstall.m
	$(LD) $(LDFLAGS) -c -o $@ $^ $(CFLAGS) -DUNINSTALL_CARBONUI -DUNINSTALL_UI -DVERSION=\"$(UNINSTALL_VERSION)\"

carbon_uninstall: $(CARBON_UNINSTALL_OBJS) $(SETUPDB)/$(arch)/libsetupdb.a
	$(LD) $(LDFLAGS) -o $@ $(CARBON_UNINSTALL_OBJS) $(CARBON_LIBS)
	rm -f -R ./uninstall.APP
	cp -R ./setup.APP ./uninstall.APP
	cp ./carbon/Info.plist.uninstall ./uninstall.APP/Contents/Info.plist
	rm -f ./uninstall.APP/Contents/MacOS/setup.carbon
	cp ./carbon_uninstall ./uninstall.APP/Contents/MacOS
uninstall: $(UNINSTALL_OBJS) $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(UNINSTALL_OBJS) $(CONSOLE_LIBS)

check: $(CHECK_OBJS)  $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(CHECK_OBJS) $(GUI_LIBS)

check.carbon: $(CHECK_CARBON_OBJS)  $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(CHECK_CARBON_OBJS) $(CARBON_LIBS)
	rm -f -R ./check.APP
	cp -R ./setup.APP ./check.APP
	cp ./carbon/Info.plist.check ./check.APP/Contents/Info.plist
	rm -f ./check.APP/Contents/MacOS/setup.carbon
	cp ./check.carbon ./check.APP/Contents/MacOS

testcd: testcd.o $(CORE_OBJS) $(COMMON_OBJS) $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $^   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz  -lcurses -ldl

setup: do-plugins do-dialog $(CONSOLE_OBJS) $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LIBS) 

$(SETUP_GTK): $(GUI_OBJS) $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(GUI_OBJS) $(GUI_LIBS)

#!!!TODO - Take dependency of dialog libs out of carbon	
#!!!TODO - Should we include setup.carbon as part of "all", "install", etc...?
# Carbon needs to be a target all its own
setup.carbon: do-plugins do-dialog $(CARBON_OBJS) $(COMMON_LIBS)
	$(LD) $(LDFLAGS) -o $@ $(CARBON_OBJS) $(CARBON_LIBS) 
	cp setup.carbon setup.APP/Contents/MacOS

xsu: $(XSU_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^   -L/lib -lxml   -L/opt/homebrew/lib  /lib/libxml2.a     -lz  -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -L/opt/homebrew/Cellar/glade/3.40.0_1/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/Cellar/gtk+3/3.24.43/lib -L/opt/homebrew/Cellar/pango/1.54.0/lib -L/opt/homebrew/Cellar/harfbuzz/10.0.1_1/lib -L/opt/homebrew/Cellar/at-spi2-core/2.54.0/lib -L/opt/homebrew/Cellar/cairo/1.18.2/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.12/lib -L/opt/homebrew/Cellar/glib/2.82.1/lib -L/opt/homebrew/opt/gettext/lib -lgladeui-2 -lgmodule-2.0 -lgtk-3 -lgdk-3 -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,CoreGraphics -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl /lib/libxml2.a   -lz $(BDYNAMIC) $(BDYNAMIC)  -L/usr/X11/lib -lXi -lXext -lX11 -lm  $(BDYNAMIC)  -L/usr/X11/lib -lXi -lXext -lX11 -lm  

do-plugins:
	$(MAKE) -C plugins all

do-dialog:
	$(MAKE) -C dialog all

install.dbg: all
ifeq ($(DYN_PLUGINS),true)
	$(MAKE) -C plugins install.dbg
endif
	@if [ -d image/setup.data/bin/$(os)/$(arch)/$(libc) ]; then \
	    cp setup image/setup.data/bin/$(os)/$(arch); \
	    cp uninstall image/setup.data/bin/$(os)/$(arch); \
	    cp $(SETUP_GTK) image/setup.data/bin/$(os)/$(arch)/$(libc); \
	fi

install: all
ifeq ($(DYN_PLUGINS),true)
	$(MAKE) -C plugins install
endif
	@mkdir -p image/setup.data/bin/$(os)/$(arch)/$(libc) 
	@if [ -d image/setup.data/bin/$(os)/$(arch)/$(libc) ]; then \
	    cp setup image/setup.data/bin/$(os)/$(arch); \
	    strip image/setup.data/bin/$(os)/$(arch)/setup; \
	    $(BRANDELF) -t $(os) image/setup.data/bin/$(os)/$(arch)/setup; \
	    cp uninstall image/setup.data/bin/$(os)/$(arch); \
	    strip image/setup.data/bin/$(os)/$(arch)/uninstall; \
	    $(BRANDELF) -t $(os) image/setup.data/bin/$(os)/$(arch)/uninstall; \
	    cp xsu image/setup.data/bin/$(os)/$(arch)/$(libc); \
	    strip image/setup.data/bin/$(os)/$(arch)/$(libc)/xsu; \
	    $(BRANDELF) -t $(os) image/setup.data/bin/$(os)/$(arch)/$(libc)/xsu; \
	    cp $(SETUP_GTK) image/setup.data/bin/$(os)/$(arch)/$(libc); \
	    strip image/setup.data/bin/$(os)/$(arch)/$(libc)/$(SETUP_GTK); \
	    $(BRANDELF) -t $(os) image/setup.data/bin/$(os)/$(arch)/$(libc)/$(SETUP_GTK); \
	else \
		echo No directory to copy the binary files to.; \
		echo   wanted image/setup.data/bin/${os}/${arch}/${libc}; \
	fi

install-setup: setup
	@if [ -d $(IMAGE)/setup.data/bin/$(os)/$(arch) ]; then \
	    cp setup $(IMAGE)/setup.data/bin/$(os)/$(arch); \
	    strip $(IMAGE)/setup.data/bin/$(os)/$(arch)/setup; \
	    $(BRANDELF) -t $(os) $(IMAGE)/setup.data/bin/$(os)/$(arch)/setup; \
	else \
		echo No directory to copy the setup binary file to.; \
	fi

install-uninstall: uninstall
	@if [ -d $(IMAGE)/setup.data/bin/$(os)/$(arch)/ ]; then \
	    cp uninstall $(IMAGE)/setup.data/bin/$(os)/$(arch)/uninstall; \
	    strip $(IMAGE)/setup.data/bin/$(os)/$(arch)/uninstall; \
	    $(BRANDELF) -t $(os) $(IMAGE)/setup.data/bin/$(os)/$(arch)/uninstall; \
	else \
		echo No directory to copy the uninstall binary file to.; \
	fi

install-image: all install-setup install-uninstall
ifeq ($(DYN_PLUGINS),true)
	$(MAKE) -C plugins install
endif
	@if [ -d $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc) ]; then \
	    cp xsu $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc); \
	    strip $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc)/xsu; \
	    $(BRANDELF) -t $(os) $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc)/xsu; \
		if [ $(os) = Linux -a -d $(CONVERT_IMAGE) ]; then \
	    	cp uninstall $(CONVERT_IMAGE)/bin/$(os)/$(arch); \
	    	strip $(CONVERT_IMAGE)/bin/$(os)/$(arch)/uninstall; \
	    	$(BRANDELF) -t $(os) $(CONVERT_IMAGE)/bin/$(os)/$(arch)/uninstall; \
		fi; \
	    cp $(SETUP_GTK) $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc); \
	    strip $(IMAGE)/setup.data/bin/$(os)/$(arch)/$(libc)/$(SETUP_GTK); \
	else \
		echo No directory to copy the binary files to.; \
	fi

# Pretty LPP-specific
install-check: check
	@if [ -d $(IMAGE)/bin/$(os)/$(arch)/ ]; then \
	    cp check $(IMAGE)/bin/$(os)/$(arch)/; \
	    strip $(IMAGE)/bin/$(os)/$(arch)/check; \
	    cp check.glade check.gtk2.glade $(IMAGE)/misc/; \
	else \
		echo No directory to copy the binary files to.; \
	fi

# Copy loki_uninstall and the required files

copy-loki_uninstall: loki_uninstall
	@if [ -d $(IMAGE)/bin/$(os)/$(arch)/bin ]; then \
		if [ -d $(IMAGE)/bin/$(os)/$(arch)/bin/gtk3 ]; then \
		    cp loki_uninstall $(IMAGE)/bin/$(os)/$(arch)/bin/gtk3/$(BRAND)-uninstall; \
		    strip $(IMAGE)/bin/$(os)/$(arch)/bin/gtk3/$(BRAND)-uninstall; \
		else \
		    cp loki_uninstall $(IMAGE)/bin/$(os)/$(arch)/bin/$(BRAND)-uninstall; \
		    strip $(IMAGE)/bin/$(os)/$(arch)/bin/$(BRAND)-uninstall; \
		fi; \
	    for file in `find image/setup.data -name loki-uninstall.mo -print`; \
	    do  path="$(IMAGE)/`dirname $$file | sed 's,image/setup.data/,,'`"; \
	        mkdir -p $$path; \
	        cp $$file $$path; \
	    done; \
	else \
		echo No directory to copy the $(BRAND)-uninstall binary file to.; \
	fi

copy-image: install-image copy-loki_uninstall po-image

install-loki_uninstall: loki_uninstall
	@if [ -d $(IMAGE)/loki_uninstall/bin/$(arch)/$(libc)/ ]; then \
	    cp loki_uninstall $(IMAGE)/loki_uninstall/bin/$(arch)/$(libc)/; \
	    strip $(IMAGE)/loki_uninstall/bin/$(arch)/$(libc)/loki_uninstall; \
	    cp README.loki_uninstall $(IMAGE)/loki_uninstall/README; \
	    cp icon.xpm uninstall.glade uninstall.gtk2.glade $(IMAGE)/loki_uninstall/; \
	    for file in `find image/setup.data -name loki-uninstall.mo -print`; \
	    do  path="$(IMAGE)/loki_uninstall/`dirname $$file | sed 's,image/setup.data/,,'`"; \
	        mkdir -p $$path; \
	        cp $$file $$path; \
	    done; \
	else \
		echo No directory to copy the binary files to.; \
	fi
	@if [ -d $(UPDATES) ]; then \
	    rm -rf $(UPDATES)/bin-$(arch)-$(UNINSTALL_VERSION)/; \
	    mkdir $(UPDATES)/bin-$(arch)-$(UNINSTALL_VERSION)/; \
	    cp loki_uninstall $(UPDATES)/bin-$(arch)-$(UNINSTALL_VERSION)/; \
	    strip $(UPDATES)/bin-$(arch)-$(UNINSTALL_VERSION)/loki_uninstall; \
	    rm -rf $(UPDATES)/data-$(UNINSTALL_VERSION)/; \
	    mkdir $(UPDATES)/data-$(UNINSTALL_VERSION)/; \
	    cp README.loki_uninstall $(UPDATES)/data-$(UNINSTALL_VERSION)/README; \
	    cp icon.xpm uninstall.glade uninstall.gtk2.glade $(UPDATES)/data-$(UNINSTALL_VERSION)/; \
	    for file in `find image/setup.data -name loki-uninstall.mo -print`; \
	    do  path="$(UPDATES)/data-$(UNINSTALL_VERSION)/`dirname $$file | sed 's,image/setup.data/,,'`"; \
	        mkdir -p $$path; \
	        cp $$file $$path; \
	    done; \
	fi
	@echo "Don't forget to update the version in setup.xml to $(UNINSTALL_VERSION)"

po-image:
	for lang in $(LOCALES); do \
		cp -f image/setup.data/locale/$$lang/LC_MESSAGES/*.mo $(IMAGE)/setup.data/locale/$$lang/LC_MESSAGES/; \
		if test -d $(CONVERT_IMAGE); then cp -f image/setup.data/locale/$$lang/LC_MESSAGES/loki-uninstall.mo $(CONVERT_IMAGE)/locale/$$lang/LC_MESSAGES/; fi \
	done

clean:
	$(MAKE) -C plugins clean
	$(MAKE) -C dialog clean
	rm -f foo.xml core tags *.o
	rm -f carbon/*.o
	rm -f unrar/*.o

distclean: clean
	rm -f Makefile config.cache config.status config.log
	rm -f setup $(SETUP_GTK) setup.carbon uninstall loki_uninstall testxml

dist: distclean
	cp -r . $(DISTDIR)/$(PACKAGE)
	(cd $(DISTDIR)/$(PACKAGE) && rm -r `find . -name CVS`)
	(cd $(DISTDIR)/$(PACKAGE) && rm -r `find . -name .cvsignore`)
	(cd $(DISTDIR) && tar zcvf $(PACKAGE).tar.gz $(PACKAGE))
	rm -rf $(DISTDIR)/$(PACKAGE)

po/setup.po: $(SRCS) image/setup.data/setup.glade
	libglade-xgettext image/setup.data/setup.glade > po/setup.po
	xgettext -p po -j -d setup --keyword=_ $(SRCS) plugins/*.c

po/loki-uninstall.po: uninstall.c uninstall_ui.c uninstall.glade
	libglade-xgettext uninstall.glade > po/loki-uninstall.po
	xgettext -p po -j -d loki-uninstall --keyword=_ uninstall.c uninstall_ui.c

gettext: po/setup.po po/loki-uninstall.po
	for lang in $(LOCALES); do \
		msgfmt -f po/$$lang/setup.po -o image/setup.data/locale/$$lang/LC_MESSAGES/setup.mo; \
		msgfmt -f po/$$lang/loki-uninstall.po -o image/setup.data/locale/$$lang/LC_MESSAGES/loki-uninstall.mo; \
	done

# This rule merges changes from the newest PO file in all the translated PO files
update-po: po/setup.po po/loki-uninstall.po
	for lang in $(LOCALES); do \
		msgmerge po/$$lang/setup.po po/setup.po > po/$$lang/tmp; \
		mv po/$$lang/tmp po/$$lang/setup.po; \
		msgmerge po/$$lang/loki-uninstall.po po/loki-uninstall.po > po/$$lang/tmp; \
		mv po/$$lang/tmp po/$$lang/loki-uninstall.po; \
	done

dep: depend

depend:
	$(MAKE) -C plugins depend
	$(MAKE) -C dialog depend
	$(CC) -MM $(CFLAGS) $(SRCS) > .depend

ifeq ($(wildcard .depend),.depend)
include .depend
endif
