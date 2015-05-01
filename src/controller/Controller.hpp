#ifndef SRC_CACHE_CONTROLLER_HPP
#define SRC_CACHE_CONTROLLER_HPP

#include "ReadLoop.hpp"
#include "Cache.hpp"
#include "Caches.hpp"
#include "SignalHandler.hpp"
#include "IOThread.hpp"

#include <sys/types.h>
#include <memory>
#include <string>

struct fuse_file_info;
struct pipefs_data;

namespace pipefs {

struct FileData;

class Controller {
public:
    Controller(const pipefs_data& data);
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
    ~Controller();

    FileData* open(const char* filename, int fd, struct fuse_file_info& fi);
    void preload(const char* filename, const char* translatedPath);
    int read(FileData* data, void* buffer, std::size_t size, off_t offset);
    void release(const char* filename, FileData* data);
    void correctStatInfo(const char* filename, struct stat* statInfo);
    void correctStatInfo(FileData* data, struct stat* statInfo);
    void waitUntilFinished(const char* filename);
    void waitUntilFinished(FileData* data);
private:
    IOThread ioThread;
    Caches caches;
    SignalHandler signalHandler;
    ReadLoop readLoop;

    const char* command;
    bool seekable;
    bool useCache;
    std::size_t cacheSize;

    void createCache(const char* key, int fd, int flags, FileData& fileData);
    void addCacheToReadLoop(int fd, int flags,
            const std::shared_ptr<Cache>& cache);
    std::shared_ptr<boost::asio::posix::stream_descriptor> createCommand(
            FileData* fileData, int fd, int flags,
            boost::asio::io_service& ioService);
};

};


#endif /* SRC_CACHE_CONTROLLER_HPP */

