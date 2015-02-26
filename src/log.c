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

FILE* log_open(const char* filename)
{
    FILE* logfile = NULL;

    logfile = fopen(filename, "w");
    if (logfile == NULL) {
        perror("logfile");
        return NULL;
    }

    // set logfile to line buffering
    setvbuf(logfile, NULL, _IOLBF, 0);

    return logfile;
}

void log_msg(const char* format, ...)
{
    struct pipefs_data* data = GET_DATA;
    if (!data->logfile) {
        return;
    }

    va_list ap;
    va_start(ap, format);

    vfprintf(data->logfile, format, ap);
}
