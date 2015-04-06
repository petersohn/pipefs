#ifndef SRC_CACHE_FILEDATA_HPP
#define SRC_CACHE_FILEDATA_HPP

#include <sys/types.h>
#include <memory>

namespace pipefs {

class Cache;

struct FileData {
    int originalFd = 0;
    int fd = 0;
    pid_t pid = 0;
    off_t currentOffset = 0;
	std::shared_ptr<Cache> cache;
};

};


#endif /* SRC_CACHE_FILEDATA_HPP */
