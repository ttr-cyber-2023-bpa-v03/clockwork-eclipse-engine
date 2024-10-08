#pragma once

#include <memory>

#include "object.hpp"
#include "event.hpp"

// Forward declarations
namespace sched {
    class runner;
}

// Forward declarations
namespace rendering {
    class render_job;
}

namespace game {
    // Forward declarations
    class write_job;
    class event_pump;

    // The root object of the entire game
    class world : public object {
    public:
        std::shared_ptr<sched::runner> scheduler;

        std::shared_ptr<game::write_job> write_job;

        std::shared_ptr<rendering::render_job> render_job;

        std::shared_ptr<game::event_pump> event_pump;

        std::shared_ptr<game::event<char>> key_down = std::make_shared<game::event<char>>();

        // Returns the singleton instance of the world
        static std::shared_ptr<world>& instance();

        world();

        // Get the maximum framerate of the game's scheduler
        int get_fps();

        // Set the maximum framerate of the game's scheduler
        void set_fps(int fps);

        // Start the scheduler. Refer to sched::runner::start for more information.
        void start(bool floating = false);

        // Stop the scheduler, optionally deciding to only send a signal to the
        // scheduler to stop, for use in jobs to prevent deadlocks. Refer to
        // sched::runner::stop and sched::runner::signal_stop for more information.
        void stop(bool signal_only = false);
    };
}