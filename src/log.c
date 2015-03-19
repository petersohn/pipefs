#include "params.h"
#include "data.h"

#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "log.h"

static FILE* log_file = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define TIME_BUFFER_SIZE 20
#define MAX_LOG_SIZE 10000

void log_open(const char* filename)
{
    log_file = NULL;

    log_file = fopen(filename, "w");
    if (log_file == NULL) {
        perror("logfile");
        exit(1);
    }
}

void log_msg(const char* format, ...)
{
    if (!log_file) {
        return;
    }

    char buffer[MAX_LOG_SIZE];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, MAX_LOG_SIZE, format, ap);

    char time_buffer[TIME_BUFFER_SIZE];
    time_t now;
    time(&now);
    strftime(time_buffer, TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", gmtime(&now));

    pthread_t self = pthread_self();

    if (pthread_mutex_lock(&mutex)) {
        exit(2);
    }

    char delimiter[2] = "\n";
    char* save_ptr = NULL;
    char* token = strtok_r(buffer, delimiter, &save_ptr);

    while (token) {
        fprintf(log_file, "%s [0x%lx] %s\n", time_buffer, self, token);
        token = strtok_r(NULL, delimiter, &save_ptr);
    }

    fflush(log_file);

    if (pthread_mutex_unlock(&mutex)) {
        exit(2);
    }
}
