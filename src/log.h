#ifndef SRC_LOG_H
#define SRC_LOG_H

#include <stdio.h>

FILE* log_open(void);
void log_msg(const char *format, ...);

#endif /* SRC_LOG_H */
