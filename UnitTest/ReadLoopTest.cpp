#include "controller/BasicReadLoop.hpp"
#include "MockStreamDescriptor.hpp"
#include "MockCache.hpp"
#include "StubLogger.hpp"

#include <boost/test/unit_test.hpp>

using namespace pipefs;

struct ReadLoopFixture {
	boost::asio::io_service ioService;
	std::shared_ptr<MockCache> cache = std::make_shared<MockCache>();
	BasicReadLoop<MockStreamDescriptor, MockCache, StubLogger>
			readLoopUnderTest{ioService};
};

BOOST_FIXTURE_TEST_SUITE(ReadLoopTest, ReadLoopFixture)

BOOST_AUTO_TEST_CASE(read_is_started)
{
	int fd = 21;
	auto stream = std::make_shared<MockStreamDescriptor>(ioService);

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

}

BOOST_AUTO_TEST_CASE(read_returns_zero_after_read)
{
	int fd = 21;
	auto stream = std::make_shared<MockStreamDescriptor>(ioService);

	std::size_t readSize1 = 2314;
	std::size_t readSize2 = 11;

	mock::sequence seq, cancelSeq, closeSeq;
	MOCK_EXPECT(stream->native_handle).returns(fd);
	MOCK_EXPECT(stream->is_open).returns(true);

	MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
			returns(MockStreamDescriptor::AsyncReadResult{{}, readSize1});
	MOCK_EXPECT(cache->write).once().in(seq).with(mock::any, readSize1);
	MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
			returns(MockStreamDescriptor::AsyncReadResult{{}, readSize2});
	MOCK_EXPECT(cache->write).once().in(seq).with(mock::any, readSize2);
	MOCK_EXPECT(stream->doAsyncRead).once().in(seq).
			returns(MockStreamDescriptor::AsyncReadResult{{}, 0});
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

BOOST_AUTO_TEST_CASE(read_returns_error_after_read)
{
	int fd = 21;
	auto stream = std::make_shared<MockStreamDescriptor>(ioService);

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

BOOST_AUTO_TEST_SUITE_END()

