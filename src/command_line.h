#ifndef SRC_COMMAND_LINE_H
#define SRC_COMMAND_LINE_H

struct pipefs_data;

char** parse_arguments(int argc, char* argv[], struct pipefs_data* data,
		int* argc_out);
void print_usage(const char* program_name);

#endif /* SRC_COMMAND_LINE_H */
