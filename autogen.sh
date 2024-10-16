#!/bin/sh

# This script covers all the necessary steps for building a project 
# using Autotools, with gettext and libtool support if needed.

# Explanation of the Flags:
#
#   glibtoolize:
#     -i: Installs required libtool files (e.g., ltmain.sh) into your project.
#     -W none: Suppresses any warnings during execution.
#     --verbose: Provides detailed output for each step performed by glibtoolize.
#   aclocal:
#     --install: Automatically installs missing auxiliary files, like aclocal.m4.
#     -W none: Suppresses any warnings that might appear.
#   autoconf:
#     -f: Forces the regeneration of the configure script, overwriting any existing one.
#     -W error: Treats warnings as errors to ensure the script follows best practices.
#   automake:
#     -f: Forces the regeneration of Makefile.in.
#     -W none: Suppresses any warnings.
#      --add-missing: Adds standard auxiliary files, such as INSTALL or COPYING, that may be missing from the project.
#  Optional:
#     autopoint: Initializes gettext files if your project supports translations.
#


# Initialize libtool support for creating portable libraries
# -i: Install required libtool files into the project
# -W none: Suppresses warnings
# --verbose: Provides detailed output of what glibtoolize is doing
glibtoolize -i -W none --verbose

# Set up gettext infrastructure for internationalization support
# (This copies necessary gettext files and sets up translation support)
gettextize

# Generate 'aclocal.m4' by scanning macros in 'configure.ac'
# --install: Automatically copy missing auxiliary files
# -W none: Suppresses warnings
aclocal --install -W none

# Update any obsolete macros found in 'configure.ac'
autoupdate

# Generate the 'configure' script from 'configure.ac'
# -f: Force overwriting of any existing 'configure' script
# -W error: Treat warnings as errors
autoconf -f -W error

# Generate 'Makefile.in' from 'Makefile.am'
# -f: Force overwriting of any existing 'Makefile.in'
# -W none: Suppresses warnings
# --add-missing: Adds any missing standard files (like INSTALL, COPYING)
automake -f -W none --add-missing

# Create 'config.h.in' template based on macros defined in 'configure.ac'
autoheader

# Regenerate 'configure' script to reflect any changes
autoconf

# Initialize gettext i18n files (only needed if gettext is used)
autopoint

# Generate 'Makefile.in' again, ensuring any missing files are added
automake --add-missing

# Run the 'configure' script to generate the final Makefiles
./configure


