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

static FILE* log_file = NULL;

void log_open(const char* filename)
{
    log_file = NULL;

    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        perror("logfile");
        exit(1);
    }

    // set logfile to line buffering
    setvbuf(log_file, NULL, _IOLBF, 0);
}

void log_msg(const char* format, ...)
{
    if (!log_file) {
        return;
    }

    va_list ap;
    va_start(ap, format);

    vfprintf(log_file, format, ap);
}
