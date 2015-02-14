#ifndef SRC_PROCESS_H
#define SRC_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

struct pipefs_filedata;

int spawn_command(const char* command, const char* input_file, int flags, 
        struct pipefs_filedata* filedata);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PROCESS_H */
