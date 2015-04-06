#ifndef SRC_COMMAND_LINE_H
#define SRC_COMMAND_LINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pipefs_data;

size_t parse_size(const char* input);
char** parse_arguments(int argc, char* argv[], struct pipefs_data* data,
        int* argc_out);
void print_usage(const char* program_name);

#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMAND_LINE_H */
