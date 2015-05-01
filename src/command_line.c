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
    {"preload",       required_argument, NULL, 'o'},
    {"process-limit", required_argument, NULL, 'P'},
    {"root-dir",      required_argument, NULL, 'r'},
    {"seekable",      no_argument,       NULL, 'k'},
    {"source-suffix", required_argument, NULL, 's'},
    {"target-suffix", required_argument, NULL, 't'},
    {"wait-on-stat",  no_argument,       NULL, 'w'},
    {NULL, 0, NULL, 0}
};

struct preload_info {
    const char* name;
    flag_type flag;
};

static struct preload_info preload_infos[] = {
    {"stat",     FLAG_PRELOAD_STAT},
    {"readdir",  FLAG_PRELOAD_READDIR},
    {NULL, 0}
};

static void parse_preload(const char* value, flag_type* flags)
{
    char* valueCopy = strdup(value); // strtok() sux
    char delimiter[2] = ",";
    char* save_ptr = NULL;
    char* token = strtok_r(valueCopy, delimiter, &save_ptr);

    while (token) {
        int tokenFound = 0;
        for (int i = 0; preload_infos[i].name; ++i) {
            if (strcmp(token, preload_infos[i].name) == 0) {
                ADD_FLAG(*flags, preload_infos[i].flag);
                tokenFound = 1;
                break;
            }
        }

        if (!tokenFound) {
            fprintf(stderr, "Warning: invalid preload flag `%s'\n", token);
        }

        token = strtok_r(NULL, delimiter, &save_ptr);
    }

    free(valueCopy);
}

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
            getopt_long(argc, argv, "hc:C:l:ko:p:P:s:t:", options, NULL)) >= 0) {
        switch (option) {
        case 'h':
            print_usage(argv[0]);
            return NULL;
        case 'c':
            data->command = optarg;
            break;
        case 'C':
            ADD_FLAG(data->flags, FLAG_CACHE);
            break;
        case 'L':
            data->cache_limit = parse_size(optarg);
            break;
        case 'k':
            ADD_FLAG(data->flags, FLAG_SEEKABLE);
            break;
        case 'l':
            log_open(optarg);
            break;
        case 'o':
            parse_preload(optarg, &data->flags);
            break;
        case 'p':
            data->pidfile = optarg;
            break;
        case 'P':
            data->process_limit = atoi(optarg);
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
        case 'w':
            ADD_FLAG(data->flags, FLAG_WAIT_ON_STAT);
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
            "    --help                 Print a help message then exit.\n\n"
            "    --command <cmd>        The filter command. Mandatory.\n\n"
            "    --cache                Use caching. Implies --seekable.\n\n"
            "    --cache-limit <num>   The maximum total size of the cache.\n\n"
            "    --log-file <filename>  The file to log to. Optional. If not\n"
            "                           provided, no logging is performed.\n\n"
            "    --pidfile <filename>   Store the pid of the process in this file.\n\n"
            "    --preload <values>     Preload the file before opening it.\n"
            "                           Only works with --cache. Multiple values\n"
            "                           can be separated by commas.\n"
            "                           Possible values: stat, readdir\n\n"
            "    --root-dir <dir>       The directory to be mapped.\n"
            "                           Mandatory.\n\n"
            "    --seekable             Allow seeking in the translated files.\n\n"
            "    --source-suffix <val>  The suffix of the files to be\n"
            "                           transformed. Mandatory.\n\n"
            "    --target-suffix <val>  The suffix of the transformed files.\n"
            "                           Mandatory.\n\n"
            "    --wait-on-stat         When running stat, wait until the read is\n"
            "                           finished. Only works with --cache.\n"
            "                           Implies --preload=stat.\n\n",
            program_name);
}



