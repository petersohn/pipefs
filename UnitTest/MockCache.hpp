#ifndef UNITTEST_MOCKCACHE_HPP
#define UNITTEST_MOCKCACHE_HPP

#include <turtle/mock.hpp>

MOCK_CLASS(MockCache) {
	MOCK_METHOD(write, 2, void(const void*, std::size_t));
	MOCK_CONST_METHOD(read, 3, int(void*, std::size_t, std::size_t));
	MOCK_METHOD(finish, 0, void());
	MOCK_CONST_METHOD(isFinished, 0, bool());
	MOCK_CONST_METHOD(getSize, 0, std::size_t());

	int id;
	MockCache() {
		static int maxId = 0;
		id = maxId++;
	}
};

#endif /* UNITTEST_MOCKCACHE_HPP */
