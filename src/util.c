#include "util.h"
#include "params.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

int is_suffix(const char* string, const char* suffix, size_t* prefix_length)
{
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);

    if (string_len < suffix_len) {
        return 0;
    }

    size_t local_prefix_len = string_len - suffix_len;

    if (strcmp(string + local_prefix_len, suffix) != 0) {
        return 0;
    }

    if (prefix_length) {
        *prefix_length = local_prefix_len;
    }

    return 1;
}

char* translate_suffix(const char* string, const char* from, const char* to)
{
    size_t prefix_length = 0;

    if (!is_suffix(string, from, &prefix_length)) {
        return NULL;
    }

    char* result = malloc(prefix_length + strlen(to) + 1);
    strncpy(result, string, prefix_length);
    strcpy(result + prefix_length, to);
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
