#ifndef UNITTEST_STUBLOGGER_HPP
#define UNITTEST_STUBLOGGER_HPP

namespace pipefs {

struct StubLogger {
	template <typename... Args>
	void operator()(Args&&... /*args*/)
	{
	}
};

}



#endif /* UNITTEST_STUBLOGGER_HPP */
