#include "cache/pipefs_cache.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pipefs_data {
    FILE* logfile;
    char* pidfile;
    char* rootdir;
    char* source_suffix;
    char* target_suffix;
    char* command;
    int seekable;
    int cache;
    size_t cache_limit;

    struct pipefs_controller* controller;
};

struct pipefs_filedata;

struct pipefs_basic_filedata {
    int fd;
    struct pipefs_filedata* data;
};

#ifdef __cplusplus
}
#endif


#define GET_DATA ((struct pipefs_data *) fuse_get_context()->private_data)

