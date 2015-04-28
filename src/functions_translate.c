#include "params.h"
#include "data.h"
#include "log.h"
#include "util.h"
#include "controller/pipefs_controller.h"

#include "functions.h"
#include "functions_common.h"

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int pipefs_getattr(const char *path, struct stat *statbuf)
{
    log_msg(" \nbb_getattr(path=\"%s\", statbuf=0x%08x)\n", path, statbuf);

    char fpath[PATH_MAX];
    pipefs_fullpath(fpath, path);
    CHECK_SOURCE_PATH(fpath);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
            data->target_suffix);

    if (translated_path) {
        int retstat = stat(translated_path, statbuf);

        if (retstat != 0) {
            retstat = pipefs_error("pipefs_getattr lstat");
            free(translated_path);
            return retstat;
        }

        if (IS_FLAG_SET(data->flags, FLAG_PRELOAD_STAT)) {
            pipefs_controller_preload(data->controller, path, translated_path);
        }
        free(translated_path);

        pipefs_controller_correct_stat_info_file(data->controller, path,
                statbuf);
        return retstat;
    }

    int retstat = lstat(fpath, statbuf);
    if (retstat != 0) {
        retstat = pipefs_error("pipefs_getattr lstat");
    }

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

    log_msg(" \nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
        path, statbuf, fi);

    // On FreeBSD, trying to do anything with the mountpoint ends up
    // opening it, and then using the FD for an fgetattr.  So in the
    // special case of a path of "/", I need to do a getattr on the
    // underlying root directory instead of doing the fgetattr().
    if (!strcmp(path, "/"))
    return pipefs_getattr(path, statbuf);

    struct pipefs_basic_filedata* filedata =
            (struct pipefs_basic_filedata*)(fi->fh);
    struct pipefs_data* data = GET_DATA;

    retstat = fstat(filedata->fd, statbuf);
    if (filedata->data) {
        pipefs_controller_correct_stat_info_fd(data->controller, filedata->data,
                statbuf);
    }

    if (retstat < 0) {
        retstat = pipefs_error("pipefs_fgetattr fstat");
    }

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

        if (real_name && IS_FLAG_SET(data->flags, FLAG_PRELOAD_READDIR)) {
            char canonical_path[PATH_MAX];
            strncpy(canonical_path, path, PATH_MAX);
            if (canonical_path[strlen(canonical_path) - 1] != '/') {
                strncat(canonical_path, "/", PATH_MAX);
            }
            strncat(canonical_path, real_name, PATH_MAX);

            char full_path[PATH_MAX];
            strncpy(full_path, data->rootdir, PATH_MAX);
            strncat(full_path, path, PATH_MAX);
            if (full_path[strlen(full_path) - 1] != '/') {
                strncat(full_path, "/", PATH_MAX);
            }
            strncat(full_path, de->d_name, PATH_MAX);

            pipefs_controller_preload(data->controller, canonical_path,
                    full_path);
        }

        free(real_name);

        if (filler_result != 0) {
            log_msg("    ERROR pipefs_readdir filler:  buffer full");
            return -ENOMEM;
        }
    } while ((de = readdir(dp)) != NULL);

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

    log_msg(" \nbb_open(path\"%s\", fi=0x%08x)\n", path, fi);
    CHECK_SOURCE_PATH(fpath);
    pipefs_fullpath(fpath, path);

    struct pipefs_data* data = GET_DATA;
    char* translated_path = translate_file(fpath, data->source_suffix,
        data->target_suffix);

    int fd = 0;
    if (translated_path) {
        if (fi->flags & (O_WRONLY | O_RDWR | O_CREAT)) {
            free(translated_path);
            log_msg("    error: flags=%04o\n", fi->flags);
            return -EINVAL;
        }
        fd = open(translated_path, fi->flags);
    } else {
        fd = open(fpath, fi->flags);
    }

    if (fd < 0) {
        free(translated_path);
        return pipefs_error("pipefs_open");
    }

    log_msg("  fd=%d\n", fd);

    struct pipefs_basic_filedata* filedata = malloc(sizeof(struct pipefs_basic_filedata));
    memset(filedata, 0, sizeof(struct pipefs_basic_filedata));
    filedata->fd = fd;

    int result = 0;
    if (translated_path) {
        result = pipefs_controller_open(data->controller, path, fd, fi,
                &filedata->data);
        free(translated_path);
    }

    if (result < 0) {
        free(filedata);
    } else {
        fi->fh = (intptr_t)(filedata);
    }

    log_msg("  result = %d\n", result);
    return result;
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
    log_msg(" \nbb_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, "
            "fi=0x%08x)\n", path, buf, size, offset, fi);

    struct pipefs_basic_filedata* filedata =
            (struct pipefs_basic_filedata*)(fi->fh);

    int result = 0;
    if (filedata->data) {
    result = pipefs_controller_read(GET_DATA->controller, filedata->data,
        buf, size, offset);
    } else {
        log_msg("    pread(fd=%d)\n", filedata->fd);
        result = pread(filedata->fd, buf, size, offset);
        if (result < 0) {
            result = pipefs_error("pipefs_read");
        }
    }

    log_msg("  result = %d\n", result);
    return result;
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
    log_msg(" \nbb_release(path=\"%s\", fi=0x%08x)\n", path, fi);

    struct pipefs_basic_filedata* filedata =
            (struct pipefs_basic_filedata*)(fi->fh);

    int result = 0;
    if (filedata->data) {
        close(filedata->fd);
        result = pipefs_controller_release(GET_DATA->controller, path,
            filedata->data);
    } else {
        result = close(filedata->fd);
        if (result < 0) {
            result = pipefs_error("pipefs_close");
        }
    }

    free(filedata);
    return result;
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

    data->controller = pipefs_controller_create(data);

    if (data->pidfile) {
        int pid = getpid();
        log_msg("pidfile = %s\n -- pid = %d", data->pidfile, pid);
        FILE* pidfile = fopen(data->pidfile, "w");
        fprintf(pidfile, "%d", pid);
        fclose(pidfile);
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

    pipefs_controller_destroy(data->controller);

    if (data->pidfile) {
        unlink(data->pidfile);
    }
}

