#include "command_line.h"
#include "params.h"
#include "data.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

static struct option options[] = {
    {"help",          no_argument,       NULL, 'h'},
    {"command",       required_argument, NULL, 'c'},
    {"cache",         no_argument,       NULL, 'C'},
    {"cache-limit",   required_argument, NULL, 'L'},
    {"log-file",      required_argument, NULL, 'l'},
    {"pidfile",       required_argument, NULL, 'p'},
    {"root-dir",      required_argument, NULL, 'r'},
    {"seekable",      no_argument,       NULL, 'k'},
    {"source-suffix", required_argument, NULL, 's'},
    {"target-suffix", required_argument, NULL, 't'},
    {NULL, 0, NULL, 0}
};

size_t parse_size(const char* input)
{
    unsigned parsed_result = 0;
    char type[2] = "b";
    int scanf_result = sscanf(input, "%u%[bBkKmMgG]", &parsed_result, type);
    if (scanf_result == EOF) {
	perror("parse_size");
	exit(1);
    }

    if (scanf_result == 0) {
	fprintf(stderr, "Error in size input.\n");
	exit(1);
    }

    size_t result = parsed_result;
    switch (type[0]) {
	case 'b':
	case 'B':
	    break;

	case 'g':
	    result *= 1024;
	    // fallthrough
	case 'm':
	    result *= 1024;
	    // fallthrough
	case 'k':
	    result *= 1024;
	    break;

	case 'G':
	    result *= 1000;
	    // fallthrough
	case 'M':
	    result *= 1000;
	    // fallthrough
	case 'K':
	    result *= 1000;
	    break;
    }

    return result;
}

char** parse_arguments(int argc, char* argv[], struct pipefs_data* data,
		int* argc_out)
{
    int option;
    optind = 0;
    while ((option =
            getopt_long(argc, argv, "hc:C:l:kp:s:t:", options, NULL)) >= 0) {
        switch (option) {
        case 'h':
            print_usage(argv[0]);
            return NULL;
        case 'c':
            data->command = optarg;
            break;
        case 'C':
	    data->seekable = 1;
	    data->cache = 1;
            break;
	case 'L':
	    data->cache_limit = parse_size(optarg);
        case 'k':
	    data->seekable = 1;
            break;
        case 'l':
            log_open(optarg);
            break;
        case 'p':
            data->pidfile = optarg;
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
		    "    --cache            Use caching. Implies --seekable.\n"
		    "    --cache-limit      The maximum total size of the cache.\n"
                    "    --log-file         The file to log to. Optional. If not\n"
                    "                       provided, no logging is performed.\n"
		    "    --pidfile          Store the pid of the process in this file.\n"
                    "    --root-dir         The directory to be mapped.\n"
                    "                       Mandatory.\n"
		    "    --seekable         Allow seeking in the translated files.\n"
                    "    --source-suffix    The suffix of the files to be\n"
                    "                       transformed. Mandatory.\n"
                    "    --target-suffix    The suffix of the transformed files.\n"
                    "                       Mandatory.\n"
                    "\n",
            program_name);
}



