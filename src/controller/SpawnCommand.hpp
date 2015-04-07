#ifndef SRC_CACHE_SPAWNCOMMAND_HPP
#define SRC_CACHE_SPAWNCOMMAND_HPP


namespace pipefs {

struct FileData;

void spawnCommand(const char* command, int fd, int flags, FileData& fileData);

};


#endif /* SRC_CACHE_SPAWNCOMMAND_HPP */

