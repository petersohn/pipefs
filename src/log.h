#ifndef SRC_LOG_H
#define SRC_LOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE* log_open(const char* filename);
void log_msg(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* SRC_LOG_H */
