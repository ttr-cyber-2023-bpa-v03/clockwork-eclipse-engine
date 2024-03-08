#pragma once

#if defined(_WIN32)

#include <functional>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

// Breaks std::min and std::max. Thanks Microsoft for making horrible macros!
#undef min
#undef max

namespace platform {
	using affinity_mask = DWORD_PTR;

	void set_thread_affinity(std::thread& thread, affinity_mask mask);

	using signal_handler = std::function<void(int)>;
    inline void on_close(signal_handler callback) {} // dummy function

	std::string executable_path();

	void open_url(const std::string& url) {} // dummy function

	void dump_and_exit() {} // dummy function
}

#else

#pragma message ("Included Windows system abstraction layer on a non-windows platform. This is probably a mistake.")

#endif