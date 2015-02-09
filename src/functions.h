#ifndef SRC_FUNCTIONS_H
#define SRC_FUNCTIONS_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int pipefs_getattr(const char *path, struct stat *statbuf);
int pipefs_readlink(const char *path, char *link, size_t size);
int pipefs_mknod(const char *path, mode_t mode, dev_t dev);
int pipefs_mkdir(const char *path, mode_t mode);
int pipefs_unlink(const char *path);
int pipefs_rmdir(const char *path);
int pipefs_symlink(const char *path, const char *link);
int pipefs_rename(const char *path, const char *newpath);
int pipefs_link(const char *path, const char *newpath);
int pipefs_chmod(const char *path, mode_t mode);
int pipefs_chown(const char *path, uid_t uid, gid_t gid);
int pipefs_truncate(const char *path, off_t newsize);
int pipefs_utime(const char *path, struct utimbuf *ubuf);
int pipefs_open(const char *path, struct fuse_file_info *fi);
int pipefs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi);
int pipefs_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi);
int pipefs_statfs(const char *path, struct statvfs *statv);
int pipefs_flush(const char *path, struct fuse_file_info *fi);
int pipefs_release(const char *path, struct fuse_file_info *fi);
int pipefs_fsync(const char *path, int datasync, struct fuse_file_info *fi);
int pipefs_opendir(const char *path, struct fuse_file_info *fi);
int pipefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi);
int pipefs_releasedir(const char *path, struct fuse_file_info *fi);
int pipefs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi);
void *pipefs_init(struct fuse_conn_info *conn);
void pipefs_destroy(void *userdata);
int pipefs_access(const char *path, int mask);
int pipefs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int pipefs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi);
int pipefs_fgetattr(const char *path, struct stat *statbuf,
		struct fuse_file_info *fi);


#endif /* SRC_FUNCTIONS_H */
