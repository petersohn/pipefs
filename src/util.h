#ifndef SRC_UTIL_H
#define SRC_UTIL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int is_suffix(const char* string, const char* suffix, size_t* prefix_length);
char* translate_suffix(const char* string, const char* from, const char* to);
char* translate_file(const char* target_path, const char* suffix_source,
		const char* suffix_target);

#ifdef __cplusplus
}
#endif

#endif /* SRC_UTIL_H */
