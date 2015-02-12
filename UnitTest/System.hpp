#ifndef UNITTEST_SYSTEM_HPP
#define UNITTEST_SYSTEM_HPP

#define CHECKED_SYSCALL(syscall, result) do { \
	(result) = (syscall); \
	if ((result) == -1 && errno != EINTR) { \
		BOOST_FAIL(#result " failed: " << strerror(errno)); \
	} \
} while ((result) == -1)



#endif /* UNITTEST_SYSTEM_HPP */
