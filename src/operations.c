#include "operations.h"
#include "functions.h"

struct fuse_operations pipefs_oper = {
  .getattr = pipefs_getattr,
  .readlink = pipefs_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = pipefs_mknod,
  .mkdir = pipefs_mkdir,
  .unlink = pipefs_unlink,
  .rmdir = pipefs_rmdir,
  .symlink = pipefs_symlink,
  .rename = pipefs_rename,
  .link = pipefs_link,
  .chmod = pipefs_chmod,
  .chown = pipefs_chown,
  .truncate = pipefs_truncate,
  .utime = pipefs_utime,
  .open = pipefs_open,
  .read = pipefs_read,
  .write = pipefs_write,
  /** Just a placeholder, don't set */ // huh???
  .statfs = pipefs_statfs,
  .flush = pipefs_flush,
  .release = pipefs_release,
  .fsync = pipefs_fsync,
  .opendir = pipefs_opendir,
  .readdir = pipefs_readdir,
  .releasedir = pipefs_releasedir,
  .fsyncdir = pipefs_fsyncdir,
  .init = pipefs_init,
  .destroy = pipefs_destroy,
  .access = pipefs_access,
  .create = pipefs_create,
  .ftruncate = pipefs_ftruncate,
  .fgetattr = pipefs_fgetattr
};


