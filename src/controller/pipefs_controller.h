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
struct stat;

struct pipefs_controller* pipefs_controller_create(const struct pipefs_data* data);
void pipefs_controller_destroy(struct pipefs_controller* controller);
int pipefs_controller_open(struct pipefs_controller* controller,
        const char* filename, int fd, struct fuse_file_info* fi,
        struct pipefs_filedata** result);
void pipefs_controller_preload(struct pipefs_controller* controller,
        const char* filename, const char* translated_path);
int pipefs_controller_read(struct pipefs_controller* controller,
        struct pipefs_filedata* data, void* buffer, size_t size, off_t offset);
int pipefs_controller_release(struct pipefs_controller* controller,
        const char* filename, struct pipefs_filedata* data);
int pipefs_controller_correct_stat_info_file(struct pipefs_controller* controller,
        const char* filename, struct stat* statbuf);
int pipefs_controller_correct_stat_info_fd(struct pipefs_controller* controller,
        struct pipefs_filedata* data, struct stat* statbuf);
void pipefs_controller_wait_until_finished_file(struct pipefs_controller* controller,
        const char* filename);
void pipefs_controller_wait_until_finished_fd(struct pipefs_controller* controller,
        struct pipefs_filedata* data);


#ifdef __cplusplus
}
#endif



#endif /* SRC_CACHE_PIPEFS_CACHE_H */
