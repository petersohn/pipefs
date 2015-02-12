#ifndef SRC_PROCESS_H
#define SRC_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

int spawn_command(const char* command, const char* input_file);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PROCESS_H */
