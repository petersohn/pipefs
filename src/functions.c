#include "params.h"
#include "data.h"
#include "log.h"
#include "util.h"

#include "functions.h"
#include "functions_common.h"

#include <dirent.h>
#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <string.h>


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

    log_msg("pipefs_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
            path, link, size);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    retstat = readlink(fpath, link, size - 1);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_readlink readlink");
    } else {
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

    log_msg(" \nbb_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n", path, mode, dev);
    pipefs_fullpath(fpath, path);

    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_CREATE(fpath);

    // On Linux this could just be 'mknod(path, mode, rdev)' but this
    //  is more portable
    if (S_ISREG(mode)) {
        retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (retstat < 0) {
            retstat = pipefs_error("pipefs_mknod open");
        } else {
            retstat = close(retstat);
            if (retstat < 0) {
                retstat = pipefs_error("pipefs_mknod close");
            }
        }
    } else if (S_ISFIFO(mode)) {
        retstat = mkfifo(fpath, mode);
        if (retstat < 0) {
            retstat = pipefs_error("pipefs_mknod mkfifo");
        }
    } else {
        retstat = mknod(fpath, mode, dev);
        if (retstat < 0) {
            retstat = pipefs_error("pipefs_mknod mknod");
        }
    }

    return retstat;
}

/** Create a directory */
int pipefs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_mkdir(path=\"%s\", mode=0%3o)\n", path, mode);
    pipefs_fullpath(fpath, path);

    CHECK_SOURCE_PATH_CREATE(fpath);

    retstat = mkdir(fpath, mode);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_mkdir mkdir");
    }

    return retstat;
}

/** Remove a file */
int pipefs_unlink(const char *path)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("pipefs_unlink(path=\"%s\")\n", path);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
            data->target_suffix);

    if (translated_path) {
        retstat = unlink(translated_path);
        free(translated_path);
        if (retstat < 0) {
            retstat = pipefs_error("pipefs_unlink unlink");
        }

        return retstat;
    }

    retstat = unlink(fpath);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_unlink unlink");
    }

    return retstat;
}

/** Remove a directory */
int pipefs_rmdir(const char *path)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("pipefs_rmdir(path=\"%s\")\n", path);
    pipefs_fullpath(fpath, path);

    retstat = rmdir(fpath);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_rmdir rmdir");
    }

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

    log_msg(" \nbb_symlink(path=\"%s\", link=\"%s\")\n", path, link);
    pipefs_fullpath(flink, link);
    CHECK_SOURCE_PATH(flink);
    CHECK_SOURCE_PATH_CREATE(flink);

    retstat = symlink(path, flink);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_symlink symlink");
    }

    return retstat;
}

/** Rename a file */
// both path and newpath are fs-relative
int pipefs_rename(const char *path, const char *newpath)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg(" \nbb_rename(fpath=\"%s\", newpath=\"%s\")\n", path, newpath);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fnewpath);
    pipefs_fullpath(fnewpath, newpath);
    CHECK_SOURCE_PATH(fnewpath);
    CHECK_SOURCE_PATH_CREATE(fnewpath);

    retstat = rename(fpath, fnewpath);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_rename rename");
    }

    return retstat;
}

/** Create a hard link to a file */
int pipefs_link(const char *path, const char *newpath)
{
    int retstat = 0;
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg(" \nbb_link(path=\"%s\", newpath=\"%s\")\n", path, newpath);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fnewpath);
    pipefs_fullpath(fnewpath, newpath);
    CHECK_SOURCE_PATH(fnewpath);
    CHECK_SOURCE_PATH_CREATE(fnewpath);

    retstat = link(fpath, fnewpath);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_link link");
    }

    return retstat;
}

/** Change the permission bits of a file */
int pipefs_chmod(const char *path, mode_t mode)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_chmod(fpath=\"%s\", mode=0%03o)\n", path, mode);
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
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_chmod chmod");
    }

    return retstat;
}

/** Change the owner and group of a file */
int pipefs_chown(const char *path, uid_t uid, gid_t gid)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_chown(path=\"%s\", uid=%d, gid=%d)\n",
            path, uid, gid);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
        data->target_suffix);

    retstat = chown(translated_path ? translated_path : fpath, uid, gid);
    free(translated_path);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_chown chown");
    }

    return retstat;
}

/** Change the size of a file */
int pipefs_truncate(const char *path, off_t newsize)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_truncate(path=\"%s\", newsize=%lld)\n", path, newsize);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_MODIFY(fpath);

    retstat = truncate(fpath, newsize);
    if (retstat < 0) {
        pipefs_error("pipefs_truncate truncate");
    }

    return retstat;
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int pipefs_utime(const char *path, struct utimbuf *ubuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg(" \nbb_utime(path=\"%s\", ubuf=0x%08x)\n", path, ubuf);
    CHECK_SOURCE_PATH(fpath);
    pipefs_fullpath(fpath, path);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
            data->target_suffix);

    retstat = utime(translated_path ? translated_path : fpath, ubuf);
    free(translated_path);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_utime utime");
    }

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

    log_msg(" \nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, "
            "fi=0x%08x)\n", path, buf, size, offset, fi);

    struct pipefs_basic_filedata* filedata = (struct pipefs_basic_filedata*)(fi->fh);

    if (!filedata->data) {
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

    log_msg(" \nbb_statfs(path=\"%s\", statv=0x%08x)\n", path, statv);
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
        data->target_suffix);
    // get stats for underlying filesystem
    retstat = statvfs(translated_path ? translated_path : fpath, statv);
    free(translated_path);
    if (retstat < 0) {
        retstat = pipefs_error("pipefs_statfs statvfs");
    }

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

    log_msg(" \nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);

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

    log_msg(" \nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);

    struct pipefs_basic_filedata* filedata = (struct pipefs_basic_filedata*)(fi->fh);

    if (!filedata->data) {
        retstat = fsync(filedata->fd);
    }

    if (retstat < 0) {
        pipefs_error("pipefs_fsync fsync");
    }

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

    log_msg(" \nbb_opendir(path=\"%s\", fi=0x%08x)\n", path, fi);
    pipefs_fullpath(fpath, path);

    dp = opendir(fpath);
    if (dp == NULL) {
        retstat = pipefs_error("pipefs_opendir opendir");
    }

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

    log_msg(" \nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, "
            "offset=%lld, fi=0x%08x)\n", path, buf, filler, offset, fi);
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

    log_msg(" \nbb_releasedir(path=\"%s\", fi=0x%08x)\n", path, fi);

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

    log_msg(" \nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);

    return retstat;
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

    log_msg(" \nbb_access(path=\"%s\", mask=0%o)\n", path, mask);
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

    if (retstat < 0) {
        retstat = pipefs_error("pipefs_access access");
    }

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
    log_msg(" \nbb_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
            path, mode, fi);

    char fpath[PATH_MAX];
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);
    CHECK_SOURCE_PATH_CREATE(fpath);

    int fd = creat(fpath, mode);
    if (fd < 0) {
        return pipefs_error("pipefs_create creat");
    }

    struct pipefs_basic_filedata* filedata =
            malloc(sizeof(struct pipefs_basic_filedata));
    memset(filedata, 0, sizeof(struct pipefs_basic_filedata));
    filedata->fd = fd;
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

    log_msg(" \nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
        path, offset, fi);

    struct pipefs_basic_filedata* filedata = (struct pipefs_basic_filedata*)(fi->fh);

    if (filedata->data) {
        return -EINVAL;
    }

    retstat = ftruncate(filedata->fd, offset);

    if (retstat < 0)
    retstat = pipefs_error("pipefs_ftruncate ftruncate");

    return retstat;
}



