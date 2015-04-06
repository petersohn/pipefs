#include "controller/BasicReadLoop.hpp"
#include "MockStreamDescriptor.hpp"
#include "MockCache.hpp"
#include "StubLogger.hpp"

#include <boost/test/unit_test.hpp>

using namespace pipefs;

struct ReadLoopFixture {
    boost::asio::io_service ioService;
    using ReadLoop = BasicReadLoop<MockStreamDescriptor, MockCache, StubLogger>;
};

BOOST_FIXTURE_TEST_SUITE(ReadLoopTest, ReadLoopFixture)

BOOST_AUTO_TEST_CASE(read_is_started)
{
    int fd = 21;
    auto cache = std::make_shared<MockCache>();
    auto stream = std::make_shared<MockStreamDescriptor>(ioService);
    ReadLoop readLoopUnderTest{ioService, 0};

    mock::sequence seq, closeSeq, cancelSeq;
    MOCK_EXPECT(stream->native_handle).returns(fd);
    MOCK_EXPECT(stream->is_open).returns(true);

    MOCK_EXPECT(stream->doAsyncRead).once().in(seq).in(cancelSeq).in(closeSeq).
            returns(MockStreamDescriptor::AsyncReadResult{{}, 0});
    MOCK_EXPECT(cache->finish).at_least(1).in(seq);
    MOCK_EXPECT(stream->cancel).in(cancelSeq);
    MOCK_EXPECT(stream->close).at_least(1).in(closeSeq);

    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream;
        }, cache);

    ioService.run();
}

namespace {

void addExpectationsForStreamReading(
        std::shared_ptr<MockStreamDescriptor> stream,
        std::shared_ptr<MockCache> cache, mock::sequence& seq,
        mock::sequence& cancelSeq, mock::sequence& closeSeq, int fd,
        const std::vector<std::size_t>& readSizes)
{
    MOCK_EXPECT(stream->native_handle).returns(fd);
    MOCK_EXPECT(stream->is_open).returns(true);

    for (std::size_t readSize: readSizes) {
        MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
                returns(MockStreamDescriptor::AsyncReadResult{{}, readSize});
        MOCK_EXPECT(cache->write).once().in(seq).with(mock::any, readSize);
    }

    MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
            returns(MockStreamDescriptor::AsyncReadResult{{}, 0});
    MOCK_EXPECT(cache->finish).at_least(1).in(seq, cancelSeq, closeSeq);

    MOCK_EXPECT(stream->cancel).in(cancelSeq);
    MOCK_EXPECT(stream->close).at_least(1).in(closeSeq);
}

}

BOOST_AUTO_TEST_CASE(read_returns_zero_after_read)
{
    int fd = 21;
    auto cache = std::make_shared<MockCache>();
    auto stream = std::make_shared<MockStreamDescriptor>(ioService);
    ReadLoop readLoopUnderTest{ioService, 0};

    mock::sequence seq, cancelSeq, closeSeq;
    addExpectationsForStreamReading(stream, cache, seq, cancelSeq, closeSeq, fd,
            {2132, 512});

    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream;
        }, cache);
    ioService.run();
}

BOOST_AUTO_TEST_CASE(read_returns_error_after_read)
{
    int fd = 21;
    auto cache = std::make_shared<MockCache>();
    auto stream = std::make_shared<MockStreamDescriptor>(ioService);
    ReadLoop readLoopUnderTest{ioService, 0};

    std::size_t readSize1 = 624;

    mock::sequence seq, cancelSeq, closeSeq;
    MOCK_EXPECT(stream->native_handle).returns(fd);
    MOCK_EXPECT(stream->is_open).returns(true);

    MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
            returns(MockStreamDescriptor::AsyncReadResult{{}, readSize1});
    MOCK_EXPECT(cache->write).once().in(seq).with(mock::any, readSize1);
    MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
            returns(MockStreamDescriptor::AsyncReadResult{
                    boost::asio::error::operation_aborted, 1});
    // TODO maybe it is not good enough to finish here?
    MOCK_EXPECT(cache->finish).at_least(1).in(seq, cancelSeq, closeSeq);

    MOCK_EXPECT(stream->cancel).in(cancelSeq);
    MOCK_EXPECT(stream->close).at_least(1).in(closeSeq);

    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream;
        }, cache);
    ioService.run();
}

BOOST_AUTO_TEST_CASE(read_handles_multiple_streams_simultaneously)
{
    int fd1 = 21;
    auto cache1 = std::make_shared<MockCache>();
    auto stream1 = std::make_shared<MockStreamDescriptor>(ioService);
    int fd2 = 22;
    auto cache2 = std::make_shared<MockCache>();
    auto stream2 = std::make_shared<MockStreamDescriptor>(ioService);
    ReadLoop readLoopUnderTest{ioService, 0};

    mock::sequence seq1, cancelSeq1, closeSeq1;
    addExpectationsForStreamReading(stream1, cache1, seq1, cancelSeq1,
            closeSeq1, fd1, {23432, 234, 130});
    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream1;
        }, cache1);

    mock::sequence seq2, cancelSeq2, closeSeq2;
    addExpectationsForStreamReading(stream2, cache2, seq2, cancelSeq2,
            closeSeq2, fd2, {23432, 234, 130});
    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream2;
        }, cache2);

    ioService.run();
}

BOOST_AUTO_TEST_CASE(reads_are_serialized_when_there_is_a_limit_to_simultaneous_process_number)
{
    int fd1 = 21;
    auto cache1 = std::make_shared<MockCache>();
    auto stream1 = std::make_shared<MockStreamDescriptor>(ioService);
    int fd2 = 22;
    auto cache2 = std::make_shared<MockCache>();
    auto stream2 = std::make_shared<MockStreamDescriptor>(ioService);
    ReadLoop readLoopUnderTest{ioService, 1};

    mock::sequence seq, cancelSeq, closeSeq;
    addExpectationsForStreamReading(stream1, cache1, seq, cancelSeq,
            closeSeq, fd1, {23432, 234, 130});
    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream1;
        }, cache1);

    addExpectationsForStreamReading(stream2, cache2, seq, cancelSeq,
            closeSeq, fd2, {23432, 234, 130});
    readLoopUnderTest.add([&](boost::asio::io_service& ioService)
        {
            BOOST_CHECK_EQUAL(&ioService, &this->ioService);
            return stream2;
        }, cache2);

    ioService.run();
}

BOOST_AUTO_TEST_SUITE_END()

