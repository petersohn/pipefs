#ifndef SRC_CACHE_BASICCACHES_HPP
#define SRC_CACHE_BASICCACHES_HPP

#include <map>
#include <string>
#include <mutex>
#include <assert.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace pipefs {

template <typename Cache, typename Logger, typename TimeUtil>
class BasicCaches {
public:
	BasicCaches(Logger logger = Logger{}, TimeUtil timeUtil = TimeUtil{}):
		logger(std::move(logger)), timeUtil(std::move(timeUtil))
	{}

	std::pair<std::shared_ptr<Cache>, bool> get(const std::string& key)
	{
		std::unique_lock<std::mutex> lock{mutex};
		auto it = caches.find(key);

		if (it == caches.end()) {
			auto& data = add(key);
			return {data.cache, true};
		}

		auto& data = it->second;

		data.lastAccessed = timeUtil.now();
		++data.usageCount;
		return {data.cache, false};
	}

	void release(const std::string& key)
	{
		std::unique_lock<std::mutex> lock{mutex};
		auto it = caches.find(key);

		if (it != caches.end()) {
			auto& data = it->second;
			assert(data.usageCount != 0);
			--data.usageCount;
			data.lastAccessed = timeUtil.now();
		}
	}

	void cleanup(std::size_t targetSize)
	{
		std::unique_lock<std::mutex> lock{mutex};
		std::size_t totalSize = 0;

		for (const auto& value: caches) {
			totalSize += value.second.cache->getSize();
		}

		logger("Total size occupied by size: %llu\n", totalSize);

		if (totalSize > targetSize) {
			std::vector<typename std::map<std::string, CacheData>::iterator>
					elements;
			elements.reserve(caches.size());
			for (auto it = caches.begin(); it != caches.end(); ++it) {
				elements.push_back(it);
			}

			std::sort(elements.begin(), elements.end(), [](auto lhs, auto rhs) {
					return lhs->second.lastAccessed < rhs->second.lastAccessed;
				});

			for (const auto& it: elements) {
				auto& data = it->second;
				if (data.cache->isFinished() && data.usageCount == 0) {
					logger("Cleared out cache for %s\n", it->first.c_str());
					totalSize -= data.cache->getSize();
					caches.erase(it);
				}

				if (totalSize < targetSize) {
					logger("Size reduced to %llu\n", totalSize);
					return;
				}
			}
		}
	}
private:
	struct CacheData {
		std::shared_ptr<Cache> cache = std::make_shared<Cache>();
		typename TimeUtil::time_point lastAccessed;
		unsigned usageCount;

		CacheData() = default;
		CacheData(const CacheData&) = delete;
		CacheData& operator=(const CacheData&) = delete;
		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;
	};

	std::mutex mutex;
	std::map<std::string, CacheData> caches;
	Logger logger;
	TimeUtil timeUtil;

	CacheData& add(const std::string& key)
	{
		auto emplaceResult = caches.emplace(key, CacheData{});
		assert(emplaceResult.second);

		auto& data = emplaceResult.first->second;
		data.lastAccessed = timeUtil.now();
		data.usageCount = 1;

		return data;
	}
};

};

#endif /* SRC_CACHE_BASICCACHES_HPP */

