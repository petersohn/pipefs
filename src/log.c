#include "params.h"
#include "data.h"

#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"

FILE* log_open()
{
    FILE* logfile;

    // very first thing, open up the logfile and mark that we got in
    // here.  If we can't open the logfile, we're dead.
    logfile = fopen("pipefs.log", "w");
    if (logfile == NULL) {
        perror("logfile");
        exit(EXIT_FAILURE);
    }

    // set logfile to line buffering
    setvbuf(logfile, NULL, _IOLBF, 0);

    return logfile;
}

void log_msg(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(GET_DATA->logfile, format, ap);
}
