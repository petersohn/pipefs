#include "params.h"
#include "data.h"
#include "log.h"
#include "operations.h"
#include "signal_handler.h"

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void print_usage(const char* program_name)
{
    fprintf(stderr, "usage:  %s [FUSE and mount options] rootDir mountPoint\n",
            program_name);
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct pipefs_data data;

    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Running pipefs as root opens unnacceptable security holes\n");
        return 1;
    }

    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
    print_usage(argv[0]);

    data.rootdir = realpath(argv[argc-2], NULL);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    data.logfile = log_open();

    signal_handler_initialize();
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &pipefs_oper, &data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
