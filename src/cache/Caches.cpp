#include "Caches.hpp"

#include "log.h"

#include <assert.h>
#include <algorithm>
#include <vector>

namespace pipefs {

namespace {

auto now() {
	return std::chrono::steady_clock::now();
}

}

Caches::CacheData& Caches::add(const std::string& key)
{
	auto emplaceResult = caches.emplace(key, CacheData{});
	assert(emplaceResult.second);

	auto& data = emplaceResult.first->second;
	data.lastAccessed = now();
	data.usageCount = 1;

	return data;
}

std::pair<Cache&, bool> Caches::get(const std::string& key)
{
	std::unique_lock<std::mutex> lock{mutex};
	auto it = caches.find(key);

	if (it == caches.end()) {
		auto& data = add(key);
		return {data.cache, true};
	}

	auto& data = it->second;

	data.lastAccessed = now();
	++data.usageCount;
	return {data.cache, false};
}

void Caches::release(const std::string& key)
{
	std::unique_lock<std::mutex> lock{mutex};
	auto it = caches.find(key);

	if (it != caches.end()) {
		auto& data = it->second;
		assert(data.usageCount != 0);
		--data.usageCount;
		data.lastAccessed = now();
	}
}

void Caches::cleanup(std::size_t targetSize)
{
	std::unique_lock<std::mutex> lock{mutex};
	std::size_t totalSize = 0;

	for (const auto& value: caches) {
		totalSize += value.second.cache.getSize();
	}

	log_msg("Total size occupied by size: %llu\n", totalSize);

	if (totalSize > targetSize) {
		std::vector<std::map<std::string, CacheData>::iterator> elements;
		elements.reserve(caches.size());
		for (auto it = caches.begin(); it != caches.end(); ++it) {
			elements.push_back(it);
		}

		std::sort(elements.begin(), elements.end(), [](auto lhs, auto rhs) {
				return lhs->second.lastAccessed < rhs->second.lastAccessed;
			});

		for (const auto& it: elements) {
			auto& data = it->second;
			if (data.usageCount == 0) {
				log_msg("Cleared out cache for %s\n", it->first.c_str());
				totalSize -= data.cache.getSize();
				caches.erase(it);
			}

			if (totalSize < targetSize) {
				log_msg("Size reduced to %llu\n", totalSize);
				return;
			}
		}
	}
}

}

