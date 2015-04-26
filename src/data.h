#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t flag_type;

#define FLAG_SEEKABLE           0x00000001
// cache implies seekable
#define FLAG_CACHE              0x00000003

#define FLAG_PRELOAD_STAT       0x00000004
#define FLAG_PRELOAD_READDIR    0x00000008

#define ADD_FLAG(flags, flag) (flags) = ((flags) | (flag))
#define IS_FLAG_SET(flags, flag) (((flags) & (flag)) == (flag))

struct pipefs_data {
    FILE* logfile;
    char* pidfile;
    char* rootdir;
    char* source_suffix;
    char* target_suffix;
    char* command;
    flag_type flags;
    size_t cache_limit;
    size_t process_limit;

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

