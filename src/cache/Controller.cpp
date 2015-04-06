#include "Controller.hpp"
#include "FileData.hpp"
#include "SystemError.hpp"
#include "log.h"
#include "SpawnCommand.hpp"

#include <fuse.h>

namespace pipefs {

std::shared_ptr<boost::asio::posix::stream_descriptor> Controller::createCommand(
        FileData& fileData, const char* translatedPath, int flags,
        boost::asio::io_service& ioService)
{
    spawnCommand(command, translatedPath, flags, fileData);
    return std::make_shared<boost::asio::posix::stream_descriptor>(
            ioService, fileData.fd);
}

Controller::~Controller()
{
    readLoop.cancel();
    signalHandler.cancel();
    ioThread.stop();
}

void Controller::createCache(const char* key, const char* translatedPath,
        int flags, FileData& fileData)
{
    auto cacheResult = caches.get(key);
    fileData.cache = cacheResult.first;
    if (cacheResult.second) {
        log_msg("  New cache.\n");
        using std::placeholders::_1;
        readLoop.add(std::bind(&Controller::createCommand, this,
                std::ref(fileData), translatedPath, flags, _1), fileData.cache);
    } else {
        log_msg("  No new cache.\n");
    }
}

FileData* Controller::open(const char* filename, const char* translatedPath,
        struct fuse_file_info& fi)
{
    if (fi.flags & (O_WRONLY | O_RDWR | O_CREAT)) {
        log_msg("    error: flags=%04o\n", fi.flags);
        throwSystemError(EINVAL);
    }

    fi.direct_io = 1;

    auto data = std::make_unique<FileData>();

    if (useCache) {
        createCache(filename, translatedPath, fi.flags, *data);
        fi.nonseekable = 0;
    } else {
        if (seekable) {
            fi.nonseekable = 0;
            data->cache = std::make_shared<Cache>();
            using std::placeholders::_1;
            readLoop.add(std::bind(&Controller::createCommand, this,
                    std::ref(*data), translatedPath, fi.flags, _1),
		    data->cache);
        } else {
            spawnCommand(command, translatedPath, fi.flags, *data);
            fi.nonseekable = 1;
        }
    }

    log_msg("    filedata=%08x -- fd=%d, original_fd=%d, offset=%d\n",
	    data.get(), data->fd, data->originalFd, data->currentOffset);
    return data.release();
}

int Controller::read(FileData* data, void* buffer, std::size_t size, off_t offset)
{
    log_msg("    filedata=%08x -- fd=%d, original_fd=%d, offset=%d\n",
	    data, data->fd, data->originalFd, data->currentOffset);

    if (data->cache) {
	log_msg("    cache_read()\n");
    return data->cache->read(buffer, size, offset);
    } else {
	if (offset != data->currentOffset) {
	log_msg("    bad offset, expected=%d\n", data->currentOffset);
	throwSystemError(ESPIPE);
	}

    log_msg("    read()\n");
    int result = ::read(data->fd, buffer, size);

    if (result < 0) {
	throwError();
    }

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
	int result = ::close(fileData->fd);
	if (result < 0) {
	    throwError();
	}
    }
}

}

