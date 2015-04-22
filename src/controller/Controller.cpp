#include "Controller.hpp"
#include "FileData.hpp"
#include "SystemError.hpp"
#include "log.h"
#include "data.h"
#include "SpawnCommand.hpp"

#include <fuse.h>

namespace pipefs {

Controller::Controller(const pipefs_data& data):
    ioThread{}, caches{}, signalHandler{ioThread.getIoService()},
    readLoop{ioThread.getIoService(), data.process_limit},
    command(data.command), seekable(data.seekable),
    useCache(data.cache), cacheSize(data.cache_limit)
{
    ioThread.start();
}

std::shared_ptr<boost::asio::posix::stream_descriptor> Controller::createCommand(
        FileData& fileData, int fd, int flags,
        boost::asio::io_service& ioService)
{
    log_msg("Starting command. fileData = %08x\n", &fileData);
    spawnCommand(command, fd, flags, fileData);
    checkedSystemCall(&close, fd);
    log_msg("    filedata=%08x -- fd=%d, offset=%d\n",
            &fileData, fileData.fd, fileData.currentOffset);
    return std::make_shared<boost::asio::posix::stream_descriptor>(
            ioService, fileData.fd);
}

Controller::~Controller()
{
    readLoop.cancel();
    signalHandler.cancel();
    ioThread.stop();
}

void Controller::createCache(const char* key, int fd, int flags,
        FileData& fileData)
{
    auto cacheResult = caches.get(key);
    fileData.cache = cacheResult.first;
    if (cacheResult.second) {
        log_msg("  New cache.\n");
        using std::placeholders::_1;
        readLoop.add(std::bind(&Controller::createCommand, this,
                std::ref(fileData), checkedSystemCall(&dup, fd), flags, _1),
                fileData.cache);
    } else {
        log_msg("  No new cache.\n");
    }
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
                    std::ref(*data), checkedSystemCall(&dup, fd), fi.flags, _1),
                    data->cache);
        } else {
            spawnCommand(command, fd, fi.flags, *data);
            fi.nonseekable = 1;
        }
    }

    log_msg("    filedata=%08x -- fd=%d, offset=%d\n",
            data.get(), data->fd, data->currentOffset);
    return data.release();
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

}

