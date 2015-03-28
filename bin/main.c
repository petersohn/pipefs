#include "params.h"
#include "data.h"
#include "operations.h"
#include "signal_handler.h"
#include "command_line.h"

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Running pipefs as root opens unnacceptable security holes\n");
        return 1;
    }

    struct pipefs_data data;
    memset(&data, 0, sizeof(struct pipefs_data));

    int fuse_argc = 0;
    char** fuse_argv = parse_arguments(argc, argv, &data, &fuse_argc);

    if (!fuse_argv) {
        // help invoked
        return 1;
    }

    if (!data.command) {
        fprintf(stderr, "Specifying command is mandatory.");
        return 1;
    }

    if (!data.rootdir) {
        fprintf(stderr, "Specifying root dir is mandatory.");
        return 1;
    }

    if (!data.source_suffix) {
        fprintf(stderr, "Specifying source suffix is mandatory.");
        return 1;
    }

    if (!data.target_suffix) {
        fprintf(stderr, "Specifying target suffix is mandatory.");
        return 1;
    }

    char real_rootdir[PATH_MAX];
    data.rootdir = realpath(data.rootdir, real_rootdir);

    char real_pidfile[PATH_MAX];
    if (data.pidfile) {
        fclose(fopen(data.pidfile, "w"));
        data.pidfile = realpath(data.pidfile, real_pidfile);
    }
    /*signal_handler_initialize();*/

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    for (int i = 0; i < fuse_argc; ++i) {
        fprintf(stderr, "  -> %s\n", fuse_argv[i]);
    }
    int fuse_stat = fuse_main(fuse_argc, fuse_argv, &pipefs_oper, &data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    free(fuse_argv);

    if (data.pidfile) {
        unlink(data.pidfile);
    }

    return fuse_stat;
}
