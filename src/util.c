#include "util.h"
#include "params.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

char* translate_suffix(const char* string, const char* from, const char* to)
{
	size_t string_len = strlen(string);
	size_t suffix_len = strlen(from);

	if (string_len < suffix_len) {
		return NULL;
	}

	size_t prefix_len = string_len - suffix_len;

	if (strcmp(string + prefix_len, from) != 0) {
		return NULL;
	}

	char* result = malloc(prefix_len + strlen(to) + 1);
	strncpy(result, string, prefix_len);
	strcpy(result + prefix_len, to);
	return result;
}

char* translate_file(const char* target_path, const char* suffix_source,
		const char* suffix_target)
{
	char* source_path = translate_suffix(target_path, suffix_target,
			suffix_source);

	if (!source_path) {
		return NULL;
	}

	struct stat data;
	int retstat = stat(source_path, &data);
	if (retstat != 0) {
		free(source_path);
		return NULL;
	}

	if (data.st_mode & S_IFDIR) {
		free(source_path);
		return NULL;
	}

	return source_path;
}
