#ifndef SRC_CACHE_PIPEFS_CACHE_H
#define SRC_CACHE_PIPEFS_CACHE_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pipefs_controller;
struct pipefs_data;
struct pipefs_filedata;
struct fuse_file_info;

struct pipefs_controller* pipefs_controller_create(const struct pipefs_data* data);
void pipefs_controller_destroy(struct pipefs_controller* controller);
int pipefs_controller_open(struct pipefs_controller* controller,
        const char* filename, int fd, struct fuse_file_info* fi,
        struct pipefs_filedata** result);
int pipefs_controller_read(struct pipefs_controller* controller,
        struct pipefs_filedata* data, void* buffer, size_t size, off_t offset);
int pipefs_controller_release(struct pipefs_controller* controller,
        const char* filename, struct pipefs_filedata* data);

void log_error(const char *str, int error);

#ifdef __cplusplus
}
#endif



#endif /* SRC_CACHE_PIPEFS_CACHE_H */
