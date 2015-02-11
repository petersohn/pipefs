#include "util.h"
#include <boost/test/unit_test.hpp>
#include <string>
#include <fstream>
#include <unistd.h>

BOOST_AUTO_TEST_SUITE(UtilTest)

struct ResultStringFixture {
	char* result = nullptr;

	~ResultStringFixture()
	{
		free(result);
	}
};

BOOST_FIXTURE_TEST_SUITE(translate_suffix_test, ResultStringFixture)

BOOST_AUTO_TEST_CASE(find_suffix)
{
	result = translate_suffix("this is first", "first", "second");
	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(std::string{result}, "this is second");
}

BOOST_AUTO_TEST_CASE(no_suffix)
{
	result = translate_suffix("this is first", "fir", "second");
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(too_long_suffix)
{
	result = translate_suffix("this is first", "this is firstt", "second");
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE_END()

struct TranslateFileFixture: ResultStringFixture {
	const char* filename = "existing_file.xxx";
	TranslateFileFixture()
	{
		std::ofstream f(filename, std::ios::out);
		f << "text" << std::endl;
	}
	~TranslateFileFixture()
	{
		unlink(filename);
	}
};

BOOST_FIXTURE_TEST_SUITE(translate_file_test, TranslateFileFixture)

BOOST_AUTO_TEST_CASE(succcessful_translation)
{
	result = translate_file("existing_file.txt", "xxx", "txt");
	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(std::string{result}, filename);
}

BOOST_AUTO_TEST_CASE(file_not_exists)
{
	result = translate_file("not_existing_file.txt", "xxx", "txt");
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(bad_source_suffix)
{
	result = translate_file("existing_file.txt", "aaa", "txt");
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(bad_target_suffix)
{
	result = translate_file("existing_file.txt", "xxx", "aaa");
	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE_END()

