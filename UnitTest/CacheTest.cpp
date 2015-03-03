#include "cache/Cache.hpp"

#include <boost/test/unit_test.hpp>

using namespace pipefs;

struct CacheFixture {
	static constexpr std::size_t bufferSize = 1000;
	char buffer[bufferSize];
	std::string createString(std::size_t length)
	{
		buffer[length] = 0;
		return buffer;
	}
};

BOOST_FIXTURE_TEST_SUITE(CacheTest, CacheFixture)

BOOST_AUTO_TEST_CASE(write_and_read_data)
{
	std::string data = "this is a text";
	Cache cache;
	cache.write(data.c_str(), data.size());
	BOOST_CHECK_EQUAL(cache.getSize(), data.size());
	int result = cache.read(buffer, bufferSize, 0);
	BOOST_CHECK_EQUAL(result, data.size());
	BOOST_CHECK_EQUAL(createString(result), data);
}

BOOST_AUTO_TEST_CASE(read_not_from_the_beginning)
{
	std::string part1 = "this is part1";
	std::string part2 = "another part of the string";
	std::string data = part1 + part2;
	Cache cache;

	cache.write(data.c_str(), data.size());
	BOOST_CHECK_EQUAL(cache.getSize(), data.size());
	int result = cache.read(buffer, bufferSize, part1.size());
	BOOST_CHECK_EQUAL(result, part2.size());
	BOOST_CHECK_EQUAL(createString(result), part2);
}

BOOST_AUTO_TEST_CASE(read_only_few_bytes)
{
	std::string data = "this is a text";
	Cache cache;

	cache.write(data.c_str(), data.size());
	BOOST_CHECK_EQUAL(cache.getSize(), data.size());
	std::size_t length = 3;
	int result = cache.read(buffer, length, 0);
	BOOST_CHECK_EQUAL(result, length);
	BOOST_CHECK_EQUAL(createString(result), data.substr(0, length));
}

BOOST_AUTO_TEST_CASE(read_only_few_bytes_not_from_beginning)
{
	std::string data = "this is a text";
	Cache cache;

	cache.write(data.c_str(), data.size());
	BOOST_CHECK_EQUAL(cache.getSize(), data.size());
	std::size_t offset = 4;
	std::size_t length = 3;
	int result = cache.read(buffer, length, offset);
	BOOST_CHECK_EQUAL(result, length);
	BOOST_CHECK_EQUAL(createString(result), data.substr(offset, length));
}

BOOST_AUTO_TEST_CASE(multiple_writes)
{
	std::string part1 = "this is part1";
	std::string part2 = "another part of the string";
	std::string data = part1 + part2;
	Cache cache;

	cache.write(part1.c_str(), part1.size());
	cache.write(part2.c_str(), part2.size());
	BOOST_CHECK_EQUAL(cache.getSize(), data.size());
	int result = cache.read(buffer, bufferSize, 0);
	BOOST_CHECK_EQUAL(result, data.size());
	BOOST_CHECK_EQUAL(createString(result), data);
}


BOOST_AUTO_TEST_SUITE_END()

