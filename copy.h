
/* Functions for unpacking and copying files with status bar update */

#include <sys/types.h>
#include <gnome-xml/parser.h>

#include "install.h"

/* Copy a path to the destination directory */
extern size_t copy_path(install_info *info, const char *path, const char *dest,
        void (*update)(void *udata, const char *path, int size), void *udata);

extern size_t copy_node(install_info *info, xmlDocPtr doc, xmlNodePtr cur,
        const char *dest,
        void (*update)(void *udata, const char *path, int size), void *udata);
