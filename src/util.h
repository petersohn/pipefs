#ifndef SRC_UTIL_H
#define SRC_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

char* translate_suffix(const char* string, const char* from, const char* to);
char* translate_file(const char* target_path, const char* suffix_source,
		const char* suffix_target);

#ifdef __cplusplus
}
#endif

#endif /* SRC_UTIL_H */
