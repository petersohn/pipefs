#ifndef SRC_CACHE_CACHES_HPP
#define SRC_CACHE_CACHES_HPP

#include "Cache.hpp"

#include <map>
#include <string>
#include <mutex>
#include <chrono>

namespace pipefs {

class Caches {
public:
	std::pair<Cache&, bool> get(const std::string& key);
	void release(const std::string& key);
	void cleanup(std::size_t targetSize);
private:
	struct CacheData {
		Cache cache;
		std::chrono::steady_clock::time_point lastAccessed;
		unsigned usageCount;

		CacheData() = default;
		CacheData(const CacheData&) = delete;
		CacheData& operator=(const CacheData&) = delete;
		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;
	};

	std::mutex mutex;
	std::map<std::string, CacheData> caches;

	CacheData& add(const std::string& key);
};

};

#endif /* SRC_CACHE_CACHES_HPP */
