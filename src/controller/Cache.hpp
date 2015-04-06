#ifndef SRC_CACHE_CACHE_HPP
#define SRC_CACHE_CACHE_HPP

#include <vector>
#include <mutex>
#include <condition_variable>

namespace pipefs {

class Cache {
public:
    void write(const void* buf, std::size_t length);
    int read(void* buf, std::size_t length, std::size_t position) const;
    void finish();
    bool isFinished() const;
    std::size_t getSize() const;

    Cache() = default;
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache(Cache&& other): data(std::move(other.data)), finished(other.finished)
    {}
    Cache& operator=(Cache&&) = default;

private:
    std::vector<char> data;
    bool finished = false;
    mutable std::mutex mutex;
    mutable std::condition_variable readWaiter;
};

}

#endif /* SRC_CACHE_CACHE_HPP */
