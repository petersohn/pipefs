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
    char* rootdir;
    char* source_suffix;
    char* target_suffix;
    char* command;
    int seekable;
    int cache;
    size_t cache_limit;

    struct pipefs_io_thread* io_thread;
    struct pipefs_readloop* readloop;
    struct pipefs_signal_handler* signal_handler;
    struct pipefs_caches* caches;
};

struct pipefs_filedata {
    int original_fd;
    int fd;
    pid_t pid;
    off_t current_offset;
    struct pipefs_cache* cache;
};

#ifdef __cplusplus
}
#endif


#define GET_DATA ((struct pipefs_data *) fuse_get_context()->private_data)

