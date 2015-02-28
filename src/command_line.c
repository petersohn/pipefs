#include "command_line.h"
#include "params.h"
#include "data.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
/*
static int parse_argument_type(const char* argument_name, char** output,
        int argc, char* argv[], int* i)
{
    if (strcmp(argv[*i], argument_name) == 0) {
        if (++*i >= argc) {
            return 1;
        }

        *output = argv[*i];
        return 1;
    }

    return 0;
}

char** parse_arguments(int argc, char* argv[], struct pipefs_data* data,
        int* argc_out)
{
    *argc_out = 0;
    memset(data, 0, sizeof(struct pipefs_data));
    size_t alloc_size = argc * sizeof(char*);
    char** result = malloc(alloc_size);
    memset(result, 0, sizeof(char*));

    char* log_file_name = NULL;

    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            free(result);
            return NULL;
        }

        if (parse_argument_type("--source-suffix", &data->source_suffix,
                    argc, argv, &i) ||
            parse_argument_type("--target-suffix", &data->target_suffix,
                    argc, argv, &i) ||
            parse_argument_type("--root-dir", &data->rootdir,
                    argc, argv, &i) ||
            parse_argument_type("--log-file", &log_file_name,
                    argc, argv, &i) ||
            parse_argument_type("--command", &data->command,
                    argc, argv, &i)) {
            continue;
        }

        result[*argc_out] = argv[i];
        ++*argc_out;
    }

    if (log_file_name) {
        data->logfile = log_open(log_file_name);
    }

    return result;
}
*/

static struct option options[] = {
    {"help",          no_argument,       NULL, 'h'},
    {"command",       required_argument, NULL, 'c'},
    {"log-file",      required_argument, NULL, 'l'},
    {"root-dir",      required_argument, NULL, 'r'},
    {"source-suffix", required_argument, NULL, 's'},
    {"target-suffix", required_argument, NULL, 't'},
};

char** parse_arguments(int argc, char* argv[], struct pipefs_data* data,
		int* argc_out)
{
    int option;
    optind = 0;
    while ((option =
            getopt_long(argc, argv, "hc:l:s:t:", options, NULL)) >= 0) {
        switch (option) {
        case 'h':
            print_usage(argv[0]);
            return NULL;
        case 'c':
            data->command = optarg;
            break;
        case 'l':
            data->logfile = log_open(optarg);
            break;
        case 'r':
            data->rootdir = optarg;
            break;
        case 's':
            data->source_suffix = optarg;
            break;
        case 't':
            data->target_suffix = optarg;
            break;
        default:
            fprintf(stderr, "Invalid argument found.\n");
            print_usage(argv[0]);
            return NULL;
            break;
        }
    }

    unsigned num_args = argc - optind + 1;
    *argc_out = num_args;
    char** result = malloc(num_args * sizeof(char*));
    result[0] = argv[0];
    for (size_t i = 1; i < num_args; ++i) {
        result[i] = argv[optind + i - 1];
    }

    return result;
}

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:  %s options... [ -- mountoptions... ] mountpoint\n"
                    "\n"
                    "Options:\n"
                    "    --help             Print a help message then exit.\n"
                    "    --command          The filter command. Mandatory.\n"
                    "    --root-dir         The directory to be mapped.\n"
                    "                       Mandatory.\n"
                    "    --source-suffix    The suffix of the files to be\n"
                    "                       transformed. Mandatory.\n"
                    "    --target-suffix    The suffix of the transformed files.\n"
                    "                       Mandatory.\n"
                    "    --log-file         The file to log to. Optional. If not\n"
                    "                       provided, no logging is performed.\n"
                    "\n"
                    "FUSE and mount options are also accepted.\n",
            program_name);
}



