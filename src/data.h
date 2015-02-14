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
};

struct pipefs_filedata {
    int original_fd;
    int fd;
    pid_t pid;
    off_t current_offset;
};

#ifdef __cplusplus
}
#endif


#define GET_DATA ((struct pipefs_data *) fuse_get_context()->private_data)

