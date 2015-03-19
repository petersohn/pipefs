#include "params.h"
#include "data.h"
#include "log.h"
#include "util.h"
#include "process.h"

#include "functions.h"

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
#include <sys/wait.h>
#include <pthread.h>

// Report errors to logfile and give -errno to caller
static int pipefs_error(char *str)
{
    int ret = -errno;
    log_msg("    ERROR %s: %s\n", str, strerror(errno));
    return ret;
}

//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
static void pipefs_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, GET_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
				    // break here

    log_msg("    pipefs_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
	    GET_DATA->rootdir, path, fpath);
}

static int has_source_path(const char* fpath)
{
    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);
    free(translated_path);
    int result = !!translated_path;
    if (result) {
	log_msg("    has_source_path: returning error\n");
    }

    return result;
}

static int check_suffix(const char* fpath)
{
    int result = is_suffix(fpath, GET_DATA->source_suffix, NULL);
    if (result) {
	log_msg("    check_suffix: returning error\n");
    }
    return result;
}

#define CHECK_SOURCE_PATH_CREATE(fpath) if (has_source_path(fpath)) return -EEXIST
#define CHECK_SOURCE_PATH_MODIFY(fpath) if (has_source_path(fpath)) return -EPERM
#define CHECK_SOURCE_PATH(fpath) if (check_suffix(fpath)) return -ENOENT

static void correct_stat_info(struct stat* statbuf)
{
    statbuf->st_mode = S_IFREG | (statbuf->st_mode & 0444);
    statbuf->st_size = 0;
    statbuf->st_blocks = 0;
}

static void create_cache(const char* key, const char* translated_path, int flags,
	struct pipefs_filedata* filedata)
{
    struct pipefs_data* data = GET_DATA;
    if (pipefs_caches_get(data->caches, key, &filedata->cache)) {
	int fd = spawn_command(data->command, translated_path, flags, filedata);
	pipefs_readloop_add(data->readloop, fd, filedata->cache);
    }
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int pipefs_getattr(const char *path, struct stat *statbuf)
{
    log_msg(" \nbb_getattr(path=\"%s\", statbuf=0x%08x, tid=0x%08x)\n",
	  path, statbuf, pthread_self());

    char fpath[PATH_MAX];
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);

    if (translated_path) {
	int retstat = stat(translated_path, statbuf);
	free(translated_path);

	if (retstat != 0) {
	    retstat = pipefs_error("pipefs_getattr lstat");
	    return retstat;
	}

	correct_stat_info(statbuf);
	return retstat;
    }

    int retstat = lstat(fpath, statbuf);
    if (retstat != 0)
	retstat = pipefs_error("pipefs_getattr lstat");

    return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to pipefs_readlink()
// pipefs_readlink() code by Bernardo F Costa (thanks!)
int pipefs_readlink(const char *path, char *link, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("pipefs_readlink(path=\"%s\", link=\"%s\", size=%d, tid=0x%08x)\n",
	  path, link, size, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    retstat = readlink(fpath, link, size - 1);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_readlink readlink");
    else  {
	link[retstat] = '\0';
	retstat = 0;
    }

    return retstat;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
// shouldn't that comment be "if" there is no.... ?
int pipefs_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_mknod(path=\"%s\", mode=0%3o, dev=%lld, tid=0x%08x)\n",
	  path, mode, dev, pthread_self());
    pipefs_fullpath(fpath, path);

    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_CREATE(fpath);

    // On Linux this could just be 'mknod(path, mode, rdev)' but this
    //  is more portable
    if (S_ISREG(mode)) {
        retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
	if (retstat < 0)
	    retstat = pipefs_error("pipefs_mknod open");
        else {
            retstat = close(retstat);
	    if (retstat < 0)
		retstat = pipefs_error("pipefs_mknod close");
	}
    } else
	if (S_ISFIFO(mode)) {
	    retstat = mkfifo(fpath, mode);
	    if (retstat < 0)
		retstat = pipefs_error("pipefs_mknod mkfifo");
	} else {
	    retstat = mknod(fpath, mode, dev);
	    if (retstat < 0)
		retstat = pipefs_error("pipefs_mknod mknod");
	}

    return retstat;
}

/** Create a directory */
int pipefs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_mkdir(path=\"%s\", mode=0%3o, tid=0x%08x)\n",
	    path, mode, pthread_self());
    pipefs_fullpath(fpath, path);

    CHECK_SOURCE_PATH_CREATE(fpath);

    retstat = mkdir(fpath, mode);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_mkdir mkdir");

    return retstat;
}

/** Remove a file */
int pipefs_unlink(const char *path)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("pipefs_unlink(path=\"%s\", tid=0x%08x)\n",
	    path, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);
    if (translated_path) {
	retstat = unlink(translated_path);
	free(translated_path);
	if (retstat < 0)
	    retstat = pipefs_error("pipefs_unlink unlink");

	return retstat;
    }

    retstat = unlink(fpath);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_unlink unlink");

    return retstat;
}

/** Remove a directory */
int pipefs_rmdir(const char *path)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("pipefs_rmdir(path=\"%s\", tid=0x%08x)\n",
	    path, pthread_self());
    pipefs_fullpath(fpath, path);

    retstat = rmdir(fpath);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_rmdir rmdir");

    return retstat;
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int pipefs_symlink(const char *path, const char *link)
{
    int retstat = 0;
    char flink[PATH_MAX];

    log_msg(" \nbb_symlink(path=\"%s\", link=\"%s\", tid=0x%08x)\n",
	    path, link, pthread_self());
    pipefs_fullpath(flink, link);
    CHECK_SOURCE_PATH(flink);
    CHECK_SOURCE_PATH_CREATE(flink);

    retstat = symlink(path, flink);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_symlink symlink");

    return retstat;
}

/** Rename a file */
// both path and newpath are fs-relative
int pipefs_rename(const char *path, const char *newpath)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg(" \nbb_rename(fpath=\"%s\", newpath=\"%s\", tid=0x%08x)\n",
	    path, newpath, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fnewpath);
    pipefs_fullpath(fnewpath, newpath);
    CHECK_SOURCE_PATH(fnewpath);
    CHECK_SOURCE_PATH_CREATE(fnewpath);

    retstat = rename(fpath, fnewpath);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_rename rename");

    return retstat;
}

/** Create a hard link to a file */
int pipefs_link(const char *path, const char *newpath)
{
    int retstat = 0;
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg(" \nbb_link(path=\"%s\", newpath=\"%s\", tid=0x%08x)\n",
	    path, newpath, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fnewpath);
    pipefs_fullpath(fnewpath, newpath);
    CHECK_SOURCE_PATH(fnewpath);
    CHECK_SOURCE_PATH_CREATE(fnewpath);

    retstat = link(fpath, fnewpath);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_link link");

    return retstat;
}

/** Change the permission bits of a file */
int pipefs_chmod(const char *path, mode_t mode)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_chmod(fpath=\"%s\", mode=0%03o, tid=0x%08x)\n",
	    path, mode, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);
    if (translated_path && (mode & 0333)) {
	free(translated_path);
	return -EPERM;
    }

    retstat = chmod(translated_path ? translated_path : fpath, mode);
    free(translated_path);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_chmod chmod");

    return retstat;
}

/** Change the owner and group of a file */
int pipefs_chown(const char *path, uid_t uid, gid_t gid)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_chown(path=\"%s\", uid=%d, gid=%d, tid=0x%08x)\n",
	    path, uid, gid, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);

    retstat = chown(translated_path ? translated_path : fpath, uid, gid);
    free(translated_path);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_chown chown");

    return retstat;
}

/** Change the size of a file */
int pipefs_truncate(const char *path, off_t newsize)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_truncate(path=\"%s\", newsize=%lld, tid=0x%08x)\n",
	    path, newsize, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fpath);

    retstat = truncate(fpath, newsize);
    if (retstat < 0)
	pipefs_error("pipefs_truncate truncate");

    return retstat;
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int pipefs_utime(const char *path, struct utimbuf *ubuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_utime(path=\"%s\", ubuf=0x%08x, tid=0x%08x)\n",
	    path, ubuf, pthread_self());
    CHECK_SOURCE_PATH(fpath);
    pipefs_fullpath(fpath, path);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);

    retstat = utime(translated_path ? translated_path : fpath, ubuf);
    free(translated_path);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_utime utime");

    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int pipefs_open(const char *path, struct fuse_file_info *fi)
{
    char fpath[PATH_MAX];

    log_msg(" \nbb_open(path\"%s\", fi=0x%08x, tid=0x%08x)\n",
	    path, fi, pthread_self());
    CHECK_SOURCE_PATH(fpath);
    pipefs_fullpath(fpath, path);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);

    struct pipefs_filedata* filedata = malloc(sizeof(struct pipefs_filedata));
    memset(filedata, 0, sizeof(struct pipefs_filedata));
    int fd = 0;
    if (translated_path) {
	if (fi->flags & (O_WRONLY | O_RDWR | O_CREAT)) {
	    log_msg("    error: flags=%04o\n", fi->flags);
	    return -EINVAL;
	}

	fi->direct_io = 1;

	if (data->cache) {
	    create_cache(path, translated_path, fi->flags, filedata);
	    fi->nonseekable = 0;
	} else {
	    fd = spawn_command(data->command, translated_path, fi->flags, filedata);

	    if (data->seekable) {
		fi->nonseekable = 0;
		filedata->cache = pipefs_cache_create();
		pipefs_readloop_add(data->readloop, fd, filedata->cache);
	    } else {
		fi->nonseekable = 1;
	    }
	}
    } else {
	fd = open(fpath, fi->flags);
	filedata->fd = fd;
	filedata->original_fd = -1;
    }

    if (fd < 0) {
	free(filedata);
	return pipefs_error("pipefs_open open");
    }

    log_msg("    filedata=%08x -- fd=%d, original_fd=%d, offset=%d\n",
	    filedata, filedata->fd, filedata->original_fd, filedata->current_offset);
    fi->fh = (intptr_t)(filedata);
    return 0;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
int pipefs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x, tid=0x%08x)\n",
	    path, buf, size, offset, fi, pthread_self());

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);
    log_msg("    filedata=%08x -- fd=%d, original_fd=%d, offset=%d\n",
	    filedata, filedata->fd, filedata->original_fd, filedata->current_offset);
    if (filedata->original_fd < 0) {
	log_msg("    pread()\n");
	retstat = pread(filedata->fd, buf, size, offset);
    } else {
	if (filedata->cache) {
	    log_msg("    cache_read()\n");
	    retstat = pipefs_cache_read(filedata->cache, buf, size, offset);
	} else {
	    if (offset != filedata->current_offset) {
		log_msg("    bad offset, expected=%d\n", filedata->current_offset);
		return -ESPIPE;
	    }
	    log_msg("    read()\n");
	    retstat = read(filedata->fd, buf, size);
	    if (retstat > 0) {
		filedata->current_offset += retstat;
	    }
	}
    }
    log_msg("    read result = %d\n", retstat);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_read read");

    return retstat;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
// As  with read(), the documentation above is inconsistent with the
// documentation for the write() system call.
int pipefs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x, tid=0x%08x)\n",
	    path, buf, size, offset, fi, pthread_self()
	    );

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);
    log_msg("    filedata=%08x -- fd=%d, original_fd=%d, offset=%d\n",
	    filedata, filedata->fd, filedata->original_fd, filedata->current_offset);
    if (filedata->original_fd < 0) {
	retstat = pwrite(filedata->fd, buf, size, offset);
	if (retstat < 0)
	    retstat = pipefs_error("pipefs_write pwrite");

	return retstat;
    } else {
	return -EINVAL;
    }
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int pipefs_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_statfs(path=\"%s\", statv=0x%08x, tid=0x%08x)\n",
	    path, statv, pthread_self());
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);
    // get stats for underlying filesystem
    retstat = statvfs(translated_path ? translated_path : fpath, statv);
    free(translated_path);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_statfs statvfs");

    return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int pipefs_flush(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_flush(path=\"%s\", fi=0x%08x, tid=0x%08x)\n",
	    path, fi, pthread_self());

    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int pipefs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_release(path=\"%s\", fi=0x%08x, tid=0x%08x)\n",
	  path, fi, pthread_self());

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);
    if (filedata->original_fd >= 0) {
	close(filedata->original_fd);
	struct pipefs_data* data = GET_DATA;

	if (data->caches) {
	    pipefs_caches_release(data->caches, path);
	    pipefs_caches_cleanup(data->caches, data->cache_limit);
	} else if (filedata->cache) {
	    pipefs_readloop_remove(data->readloop, filedata->fd);
	    pipefs_cache_destroy(filedata->cache);
	}
    } else {
	retstat = close(filedata->fd);
    }
    free(filedata);

    return retstat;
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int pipefs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x, tid=0x%08x)\n",
	    path, datasync, fi, pthread_self());

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);

    if (!filedata->cache) {
	retstat = fsync(filedata->fd);
    }

    if (retstat < 0)
	pipefs_error("pipefs_fsync fsync");

    return retstat;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int pipefs_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_opendir(path=\"%s\", fi=0x%08x, tid=0x%08x)\n",
	  path, fi, pthread_self());
    pipefs_fullpath(fpath, path);

    dp = opendir(fpath);
    if (dp == NULL)
	retstat = pipefs_error("pipefs_opendir opendir");

    fi->fh = (intptr_t) dp;

    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int pipefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dp;
    struct dirent *de;

    log_msg(" \nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x, tid=0x%08x)\n",
	    path, buf, filler, offset, fi, pthread_self());
    // once again, no need for fullpath -- but note that I need to cast fi->fh
    dp = (DIR *) (uintptr_t) fi->fh;

    // Every directory contains at least two entries: . and ..  If my
    // first call to the system readdir() returns NULL I've got an
    // error; near as I can tell, that's the only condition under
    // which I can get an error from readdir()
    de = readdir(dp);
    if (de == 0) {
	retstat = pipefs_error("pipefs_readdir readdir");
	return retstat;
    }

    // This will copy the entire directory into the buffer.  The loop exits
    // when either the system readdir() returns NULL, or filler()
    // returns something non-zero.  The first case just means I've
    // read the whole directory; the second means the buffer is full.
    do {
	struct pipefs_data* data = GET_DATA;
	char* real_name = translate_suffix(de->d_name, data->source_suffix,
		data->target_suffix);

	log_msg("calling filler with name %s\n", de->d_name);
	int filler_result = filler(buf, real_name ? real_name : de->d_name,
		NULL, 0);
	free(real_name);

	if (filler_result != 0) {
	    log_msg("    ERROR pipefs_readdir filler:  buffer full");
	    return -ENOMEM;
	}
    } while ((de = readdir(dp)) != NULL);

    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int pipefs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_releasedir(path=\"%s\", fi=0x%08x, tid=0x%08x)\n",
	    path, fi, pthread_self());

    closedir((DIR *) (uintptr_t) fi->fh);

    return retstat;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ???
int pipefs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x, tid=0x%08x)\n",
	    path, datasync, fi, pthread_self());

    return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
void *pipefs_init(struct fuse_conn_info *conn)
{
    log_msg(" \nbb_init()\n");
    (void)conn;

    struct pipefs_data* data = GET_DATA;
    data->io_thread = pipefs_io_thread_create();
    pipefs_io_thread_start(data->io_thread);

    data->readloop = pipefs_readloop_create(data->io_thread);
    data->signal_handler = pipefs_signal_handler_create(data->io_thread);
    pipefs_signal_handler_start(data->signal_handler);

    if (data->cache) {
	data->caches = pipefs_caches_create();
    }

    return data;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void pipefs_destroy(void *userdata)
{
    struct pipefs_data* data = (struct pipefs_data*)userdata;
    log_msg(" \nbb_destroy(userdata=0x%08x)\n", userdata);

    if (data->caches) {
	pipefs_caches_destroy(data->caches);
    }

    pipefs_readloop_cancel(data->readloop);
    pipefs_signal_handler_cancel(data->signal_handler);
    pipefs_io_thread_stop(data->io_thread);
    pipefs_readloop_destroy(data->readloop);
    pipefs_signal_handler_destroy(data->signal_handler);
    pipefs_io_thread_destroy(data->io_thread);
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int pipefs_access(const char *path, int mask)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_access(path=\"%s\", mask=0%o, tid=0x%08x)\n",
	    path, mask, pthread_self());
    pipefs_fullpath(fpath, path);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
	    data->target_suffix);
    if (translated_path && (mask & (W_OK | X_OK))) {
	free(translated_path);
	return -EACCES;
    }

    retstat = access(translated_path ? translated_path : fpath, mask);
    free(translated_path);

    if (retstat < 0)
	retstat = pipefs_error("pipefs_access access");

    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int pipefs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    log_msg(" \nbb_create(path=\"%s\", mode=0%03o, fi=0x%08x, tid=0x%08x)\n",
	    path, mode, fi, pthread_self());

    char fpath[PATH_MAX];
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_CREATE(fpath);

    struct pipefs_filedata* filedata = malloc(sizeof(struct pipefs_filedata));
    memset(filedata, 0, sizeof(struct pipefs_filedata));
    int fd = creat(fpath, mode);
    if (fd < 0) {
	free(filedata);
	return pipefs_error("pipefs_create creat");
    }

    filedata->fd = fd;
    filedata->original_fd = -1;
    fi->fh = (intptr_t)(filedata);

    return 0;
}

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int pipefs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x, tid=0x%08x)\n",
	    path, offset, fi, pthread_self());

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);
    retstat = ftruncate(filedata->fd, offset);
    if (retstat < 0)
	retstat = pipefs_error("pipefs_ftruncate ftruncate");

    return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int pipefs_fgetattr(const char *path, struct stat *statbuf,
		struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg(" \nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x, tid=0x%08x)\n",
	    path, statbuf, fi, pthread_self());

    // On FreeBSD, trying to do anything with the mountpoint ends up
    // opening it, and then using the FD for an fgetattr.  So in the
    // special case of a path of "/", I need to do a getattr on the
    // underlying root directory instead of doing the fgetattr().
    if (!strcmp(path, "/"))
	return pipefs_getattr(path, statbuf);

    struct pipefs_filedata* filedata = (struct pipefs_filedata*)(fi->fh);
    if (filedata->original_fd >= 0) {
	retstat = fstat(filedata->original_fd, statbuf);
	correct_stat_info(statbuf);
    } else {
	retstat = fstat(filedata->fd, statbuf);
    }
    if (retstat < 0)
	retstat = pipefs_error("pipefs_fgetattr fstat");

    return retstat;
}



