#include "params.h"
#include "functions_common.h"
#include "data.h"
#include "log.h"
#include "util.h"

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <string.h>



void log_error(const char *str, int error)
{
    log_msg("    ERROR %s: %s\n", str, strerror(error));
}

int pipefs_error(char *str)
{
    int ret = -errno;
    log_error(str, errno);
    return ret;
}

void pipefs_fullpath(char fpath[PATH_MAX], const char *path)
{
	struct pipefs_data* data = GET_DATA;
    strcpy(fpath, data->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will break here

    log_msg("    pipefs_fullpath:  rootdir = \"%s\", path = \"%s\", "
            "fpath = \"%s\"\n", data->rootdir, path, fpath);
}

int has_source_path(const char* fpath)
{
    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
            data->target_suffix);
    free(translated_path);
    int result = !!translated_path;

    return result;
}

int check_suffix(const char* fpath)
{
    int result = is_suffix(fpath, GET_DATA->source_suffix, NULL);
    return result;
}


