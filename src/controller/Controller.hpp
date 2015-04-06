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

	FileData* open(const char* filename, const std::string& translatedPath,
			struct fuse_file_info& fi);
	int read(FileData* data, void* buffer, std::size_t size, off_t offset);
	void release(const char* filename, FileData* data);
private:
	IOThread ioThread;
	Caches caches;
	SignalHandler signalHandler;
	ReadLoop readLoop;

	const char* command;
	bool seekable;
	bool useCache;
	std::size_t cacheSize;

	void createCache(const char* key, const std::string& translated_path, int flags,
			FileData& fileData);
	std::shared_ptr<boost::asio::posix::stream_descriptor> createCommand(
			FileData& fileData, std::string translatedPath, int flags,
			boost::asio::io_service& ioService);
};

};


#endif /* SRC_CACHE_CONTROLLER_HPP */

