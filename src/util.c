#include "util.h"
#include <string.h>
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
