#include "controller/BasicCaches.hpp"

#include "MockCache.hpp"
#include "StubLogger.hpp"
#include "FakeTimeUtil.hpp"

#include <boost/test/unit_test.hpp>

using namespace pipefs;

struct CachesFixture {
    BasicCaches<MockCache, StubLogger, FakeTimeUtil> caches;
};

BOOST_FIXTURE_TEST_SUITE(CachesTest, CachesFixture)

BOOST_AUTO_TEST_CASE(add_then_get_same)
{
    std::string key = "filename";
    auto result1 = caches.get(key);
    BOOST_CHECK(result1.second);
    int id1 = result1.first->id;
    auto result2 = caches.get(key);
    BOOST_CHECK(!result2.second);
    int id2 = result2.first->id;
    result1.first.reset();
    result2.first.reset();

    BOOST_CHECK_EQUAL(id1, id2);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id1), 0);
}

BOOST_AUTO_TEST_CASE(add_then_get_different)
{
    std::string key1 = "filename1";
    std::string key2 = "filename2";
    auto result1 = caches.get(key1);
    BOOST_CHECK(result1.second);
    int id1 = result1.first->id;
    auto result2 = caches.get(key2);
    BOOST_CHECK(result2.second);
    int id2 = result2.first->id;
    result1.first.reset();
    result2.first.reset();

    BOOST_CHECK_NE(id1, id2);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id1), 0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id2), 0);
}

BOOST_AUTO_TEST_CASE(getIfPresent_returns_correct_value_for_existing_key)
{
    std::string key1 = "filename1";
    auto result1 = caches.get(key1);
    auto cache1 = result1.first;
    BOOST_CHECK_EQUAL(caches.getIfPresent(key1).get(), cache1.get());
}

BOOST_AUTO_TEST_CASE(getIfPresent_returns_null_for_non_existing_key)
{
    std::string key1 = "filename1";
    std::string key2 = "filename2";
    caches.get(key1);
    BOOST_CHECK_EQUAL(caches.getIfPresent(key2).get(), (MockCache*)nullptr);
}

BOOST_AUTO_TEST_CASE(cache_remains_active_after_release)
{
    std::string key = "filename";
    auto result = caches.get(key);
    int id = result.first->id;
    result.first.reset();

    caches.release(key);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 0);
}

BOOST_AUTO_TEST_CASE(cleanup_cleans_up_entries)
{
    std::string key = "filename";
    auto result = caches.get(key);
    int id = result.first->id;
    MOCK_EXPECT(result.first->getSize).returns(10);
    MOCK_EXPECT(result.first->isFinished).returns(true);
    result.first.reset();

    caches.release(key);
    caches.cleanup(0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 1);
}

BOOST_AUTO_TEST_CASE(cleanup_does_not_clean_up_entries_that_are_not_released)
{
    std::string key = "filename";
    auto result = caches.get(key);
    int id = result.first->id;
    MOCK_EXPECT(result.first->getSize).returns(10);
    MOCK_EXPECT(result.first->isFinished).returns(true);
    result.first.reset();

    caches.cleanup(0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 0);
}

BOOST_AUTO_TEST_CASE(cleanup_does_not_clean_up_entries_that_are_not_finished)
{
    std::string key = "filename";
    auto result = caches.get(key);
    int id = result.first->id;
    MOCK_EXPECT(result.first->getSize).returns(10);
    MOCK_EXPECT(result.first->isFinished).returns(false);
    result.first.reset();

    caches.release(key);
    caches.cleanup(0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 0);
}

BOOST_AUTO_TEST_CASE(cleanup_does_not_clean_up_entries_if_there_is_enough_space)
{
    std::string key = "filename";
    auto result = caches.get(key);
    int id = result.first->id;
    MOCK_EXPECT(result.first->getSize).returns(10);
    MOCK_EXPECT(result.first->isFinished).returns(true);
    result.first.reset();

    caches.release(key);
    caches.cleanup(20);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 0);
}

BOOST_AUTO_TEST_CASE(cleanup_cleans_up_entries_only_after_the_appropriate_number_of_releases)
{
    std::string key = "filename";
    auto result1 = caches.get(key);
    auto result2 = caches.get(key);
    int id = result1.first->id;
    MOCK_EXPECT(result1.first->getSize).returns(10);
    MOCK_EXPECT(result1.first->isFinished).returns(true);
    result1.first.reset();
    result2.first.reset();

    caches.release(key);
    caches.cleanup(0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 0);
    caches.release(key);
    caches.cleanup(0);
    BOOST_CHECK_EQUAL(MockCache::destroyedIds.count(id), 1);
}

BOOST_AUTO_TEST_CASE(cleanup_cleans_up_old_entries_until_necessary)
{
    std::string key1 = "filename1";
    std::string key2 = "filename2";
    std::string key3 = "filename3";
    auto result1 = caches.get(key1);
    auto result2 = caches.get(key2);
    auto result3 = caches.get(key3);

    MOCK_EXPECT(result1.first->getSize).returns(10);
    MOCK_EXPECT(result1.first->isFinished).returns(true);
    int id1 = result1.first->id;

    MOCK_EXPECT(result2.first->getSize).returns(10);
    MOCK_EXPECT(result2.first->isFinished).returns(true);
    int id2 = result2.first->id;

    MOCK_EXPECT(result3.first->getSize).returns(10);
    MOCK_EXPECT(result3.first->isFinished).returns(true);
    int id3 = result3.first->id;

    caches.release(key1);
    caches.release(key2);
    caches.release(key3);

    caches.cleanup(25);

    auto result1_2 = caches.get(key1);
    auto result2_2 = caches.get(key2);
    auto result3_2 = caches.get(key3);

    BOOST_CHECK(result1_2.second);
    BOOST_CHECK_NE(result1_2.first->id, id1);
    BOOST_CHECK(!result2_2.second);
    BOOST_CHECK_EQUAL(result2_2.first->id, id2);
    BOOST_CHECK(!result3_2.second);
    BOOST_CHECK_EQUAL(result3_2.first->id, id3);
}

BOOST_AUTO_TEST_SUITE_END()


