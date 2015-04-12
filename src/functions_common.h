#ifndef FUNCTIONS_COMMON_H
#define FUNCTIONS_COMMON_H

#include <linux/limits.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_error(const char *str, int error);
int pipefs_error(char *str);
void pipefs_fullpath(char fpath[PATH_MAX], const char *path);
int has_source_path(const char* fpath);
int check_suffix(const char* fpath);

#ifdef __cplusplus
}
#endif

#define CHECK_SOURCE_PATH_CREATE(fpath) if (has_source_path(fpath)) return -EEXIST
#define CHECK_SOURCE_PATH_MODIFY(fpath) if (has_source_path(fpath)) return -EPERM
#define CHECK_SOURCE_PATH(fpath) if (check_suffix(fpath)) return -ENOENT

#endif /* FUNCTIONS_COMMON_H */
