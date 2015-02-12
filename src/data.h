#include <limits.h>
#include <stdio.h>

struct pipefs_data {
    FILE* logfile;
    char* rootdir;
    char* source_suffix;
    char* target_suffix;
};


#define GET_DATA ((struct pipefs_data *) fuse_get_context()->private_data)

