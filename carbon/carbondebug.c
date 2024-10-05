#include "carbondebug.h"
#include "install_log.h"

#if 0
void carbon_debug(const char *str)
{
 	/* Just use the standard Setup debug mechanism, i.e. go to
	   stderr when debugging is enabled */   
	log_debug("%s", str);
}
#endif

#include <stdarg.h>
#include <stdio.h>

void carbon_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);  // Print the formatted string
    va_end(args);
}
