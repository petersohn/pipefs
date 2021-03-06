#include "Controller.hpp"
#include "FileData.hpp"
#include "SystemError.hpp"
#include "log.h"
#include "data.h"
#include "SpawnCommand.hpp"

#include <util/Finally.hpp>

#include <fuse.h>

namespace pipefs {

namespace {

auto finallyCloseFd(int fd) {
    return util::finally([fd]() { ::close(fd); });
}

}

Controller::Controller(const pipefs_data& data):
    ioThread{},
    caches{},
    signalHandler{std::make_shared<SignalHandler>(ioThread.getIoService())},
    readLoop{ioThread.getIoService(), data.process_limit},
    command(data.command),
    seekable(IS_FLAG_SET(data.flags, FLAG_SEEKABLE)),
    useCache(IS_FLAG_SET(data.flags, FLAG_CACHE)),
    cacheSize(data.cache_limit)
{
    signalHandler->start();
    ioThread.start();
}

std::shared_ptr<boost::asio::posix::stream_descriptor> Controller::createCommand(
        FileData* fileData, int inputFd, int flags,
        boost::asio::io_service& ioService)
{
    auto fdClose = finallyCloseFd(inputFd);
    log_msg("Starting command. fileData = %08x\n", fileData);
    int outputFd = spawnCommand(command, inputFd, flags, fileData);

    if (fileData) {
        log_msg("    filedata=%08x -- fd=%d, offset=%d\n",
                fileData, fileData->fd, fileData->currentOffset);
    } else {
        log_msg("    fd=%d\n", outputFd);
    }

    return std::make_shared<boost::asio::posix::stream_descriptor>(
            ioService, outputFd);
}

Controller::~Controller()
{
    readLoop.cancel();
    signalHandler->cancel();
    ioThread.stop();
}

void Controller::createCache(const char* key, int fd, int flags,
        FileData& fileData)
{
    auto cacheResult = caches.get(key);
    fileData.cache = cacheResult.first;
    if (cacheResult.second) {
        log_msg("  New cache.\n");
        addCacheToReadLoop(fd, flags, fileData.cache);
    } else {
        log_msg("  No new cache.\n");
    }
}

void Controller::addCacheToReadLoop(int fd, int flags,
        const std::shared_ptr<Cache>& cache)
{
    using std::placeholders::_1;
    readLoop.add(std::bind(&Controller::createCommand, this, nullptr,
            checkedSystemCall(&dup, fd), flags, _1), cache);
}

FileData* Controller::open(const char* filename, int fd,
        struct fuse_file_info& fi)
{
    fi.direct_io = 1;

    auto data = std::make_unique<FileData>();

    if (useCache) {
        createCache(filename, fd, fi.flags, *data);
        fi.nonseekable = 0;
    } else {
        if (seekable) {
            fi.nonseekable = 0;
            data->cache = std::make_shared<Cache>();
            using std::placeholders::_1;
            readLoop.add(std::bind(&Controller::createCommand, this,
                    data.get(), checkedSystemCall(&dup, fd), fi.flags, _1),
                    data->cache);
        } else {
            spawnCommand(command, fd, fi.flags, data.get());
            fi.nonseekable = 1;
        }
    }

    log_msg("    filedata=%08x -- fd=%d, offset=%d\n",
            data.get(), data->fd, data->currentOffset);
    return data.release();
}

void Controller::preload(const char* filename, const char* translatedPath)
{
    if (useCache) {
        log_msg("  preload('%s', '%s')\n", filename, translatedPath);

        auto cacheResult = caches.get(filename);
        if (cacheResult.second) {
            log_msg("    New cache.\n");
            int flags = O_RDONLY;
            int fd = ::open(translatedPath, flags);

            if (fd < 0) {
                log_msg("    Error: cannot open file. Error message = %s\n",
                        strerror(errno));
                return;
            }

            auto closeFd = finallyCloseFd(fd);
            addCacheToReadLoop(fd, flags, cacheResult.first);
        } else {
            log_msg("    No new cache.\n");
        }
    }
}

int Controller::read(FileData* data, void* buffer, std::size_t size, off_t offset)
{
    log_msg("    filedata=%08x -- fd=%d, offset=%d\n",
            data, data->fd, data->currentOffset);

    if (data->cache) {
        log_msg("    cache_read()\n");
        return data->cache->read(buffer, size, offset);
    } else {
        if (offset != data->currentOffset) {
            log_msg("    bad offset, expected=%d\n", data->currentOffset);
            throwSystemError(ESPIPE);
        }

        log_msg("    read()\n");
        int result = checkedSystemCall(&::read, data->fd, buffer, size);

        if (result > 0) {
            data->currentOffset += result;
        }

        return result;
    }
}

void Controller::release(const char* filename, FileData* data)
{
    std::unique_ptr<FileData> fileData{data};

    if (useCache) {
        caches.release(filename);
        if (cacheSize > 0) {
            caches.cleanup(cacheSize);
        }
    } else if (seekable) {
        readLoop.remove(fileData->fd);
    } else {
        checkedSystemCall(&close, fileData->fd);
    }
}

namespace {

void modifyStatInfo(const Cache* cache, struct stat* statInfo)
{
    std::size_t size = (cache && cache->isFinished()) ? cache->getSize() : 0;

    log_msg("    correct stat info. Calculated size = %d. cache = 0x%08x\n",
            size, cache);
    statInfo->st_mode = S_IFREG | (statInfo->st_mode & 0444);
    statInfo->st_size = size;
    statInfo->st_blocks = size / 512 + ((size % 512) ? 1 : 0);
}

} // unnamed namespace

void Controller::correctStatInfo(const char* filename, struct stat* statInfo)
{
    Cache* cache = nullptr;

    if (useCache) {
        cache = caches.getIfPresent(filename).get();
    }

    modifyStatInfo(cache, statInfo);
}

void Controller::correctStatInfo(FileData* data, struct stat* statInfo)
{
    modifyStatInfo(data->cache.get(), statInfo);
}

void Controller::waitUntilFinished(const char* filename)
{
    auto cache = caches.getIfPresent(filename);
    assert(cache);
    cache->waitUntilFinished();
}

void Controller::waitUntilFinished(FileData* data)
{
    data->cache->waitUntilFinished();
}

} // namespace pipefs

