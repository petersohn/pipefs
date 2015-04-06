#ifndef UNITTEST_FAKETIMEUTIL_HPP
#define UNITTEST_FAKETIMEUTIL_HPP

#include <chrono>

namespace pipefs {

class FakeTimeUtil {
public:
    using time_point = std::chrono::steady_clock::time_point;

    time_point now() {
        clock += std::chrono::seconds{1};
        return clock;
    }
private:
    time_point clock = std::chrono::steady_clock::now();
};

}


#endif /* UNITTEST_FAKETIMEUTIL_HPP */
