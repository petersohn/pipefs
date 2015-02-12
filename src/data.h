#include <limits.h>
#include <stdio.h>

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

#ifdef __cplusplus
}
#endif


#define GET_DATA ((struct pipefs_data *) fuse_get_context()->private_data)

