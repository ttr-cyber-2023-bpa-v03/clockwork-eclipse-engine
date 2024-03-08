#if defined (__linux__)

#include "iostream"
#include "util/logger.hpp"
#include <bitset>
#include <list>
#include <pthread.h>

#include "game/world.hpp"
#include "sched/runner.hpp"

#include "linux.hpp"
#include <sched.h>
#include <list>
#include <csignal>
#include <unordered_map>

namespace platform {
    void set_thread_affinity(std::thread& thread, const affinity_mask mask) {
        // oh yeah jit go brrr and crash so i commented this out
        
        // Linux uses the same concept but they have to be different and use a struct for
        // some reason, so we just do a cast.

        /*std::cout << "set: " << std::setfill('0') << std::setw(4) << std::hex << mask << std::endl;

        cpu_set_t set_th;

        CPU_ZERO(&set_th);
        pthread_getaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &set_th);
        std::cout << "old: " << std::setfill('0') << std::setw(4) << std::hex << *reinterpret_cast<std::uintptr_t*>(&set_th) << std::endl;

        const auto set = *reinterpret_cast<const cpu_set_t*>(&mask);
        if (pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &set) != 0)
            throw std::runtime_error("Failed to set thread affinity");

        CPU_ZERO(&set_th);
        pthread_getaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &set_th);
        std::cout << "new: " << std::setfill('0') << std::setw(4) << std::hex << *reinterpret_cast<std::uintptr_t*>(&set_th) << std::endl;

        // uh make sure the process can actually use the cores
        //affinity_mask max_mask = 0xFFFFFFFFFFFFFFFF;
        //auto max_set = *reinterpret_cast<cpu_set_t*>(&max_mask);
        //sched_setaffinity(getpid(), sizeof(cpu_set_t), &max_set);*/
    }

    // This is a workaround for the fact that std::signal does not support passing
    // contextually relevant data to the callback function. If i could take a guess, it's
    // because the callback is a C-style function pointer, and C doesn't know what a
    // lambda is. At least this isn't cursed code.
    struct signal_dispatcher {
        static std::unordered_map<int, signal_handler> callbacks;

        // This code will overwrite other handlers if they exist
        static void handle(int sig, signal_handler&& callback) {
            callbacks[sig] = std::move(callback);
            std::signal(sig, [](int signal) {
                callbacks[signal](signal);
            });
        }
    };

    std::unordered_map<int, signal_handler> signal_dispatcher::callbacks;

    void on_close(signal_handler callback) {
        signal_dispatcher::handle(SIGINT, std::move(callback));
        signal_dispatcher::handle(SIGTERM, std::move(callback));
    }

    std::string executable_path() {
        std::string path_str(PATH_MAX, '\0');
        const auto path_len = readlink("/proc/self/exe", path_str.data(), PATH_MAX);

        if (path_len == -1)
            throw std::runtime_error("readlink failed");
        path_str.resize(path_len);
        
        return path_str;
    }

    void dump_and_exit() {
        // Raise SIGABRT to generate a core dump
        raise(SIGABRT);
    }

    void open_url(const std::string& url) {
        // This is a bit cursed but it works...just please use xdg-open or handlr
        util::log::send(util::log_level::info, "Opening URL: " + url);
        std::system(std::string("xdg-open " + url).c_str());
    }
}

#endif