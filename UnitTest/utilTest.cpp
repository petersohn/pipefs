#include "util.h"
#include <boost/test/unit_test.hpp>
#include <string>

BOOST_AUTO_TEST_SUITE(UtilTest)

BOOST_AUTO_TEST_CASE(translate_suffix_find_suffix)
{
	char* result = translate_suffix("this is first", "first", "second");
	BOOST_CHECK_EQUAL(std::string{result}, "this is second");
	free(result);
}

BOOST_AUTO_TEST_SUITE_END()

