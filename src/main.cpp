#include "SDL.h"
#include "SDL_messagebox.h"
#include "game/world.hpp"
#include "game/write_job.hpp"

#include "sched/job.hpp"
#include "sched/runner.hpp"
#include "sched/worker.hpp"

#include "rendering/render_job.hpp"
#include "rendering/renderables/text_box.hpp"

#include "platform/current.hpp"

#include "util/uri_tools.hpp"
#include "util/logger.hpp"

#include <stdexcept>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

void exception_filter() {
	const std::string msg = "An unhandled exception has occurred. The program will now exit.";
	const std::string question = "Would you like to report this via email?";

	// Make sure we note the main message in the log
	util::log::send(util::log_level::fatal, msg);

	// Determine what the exception is
	try {
		// Propagate the exception to a try/catch block
        std::rethrow_exception(std::current_exception());
	}
	catch (std::exception& ex) {
		// Log what the exception says
		util::log::send(util::log_level::error, "Unhandled exception: {}", ex.what());
    }
	catch (...) {
		// If anything gets thrown that isn't an exception, we will just log it
		util::log::send(util::log_level::error, "Unhandled exception: Unknown");
	}

	// Show a message box so the app doesn't just disappear into the void, defaulting to
	// 'No' if the message box fails to show
	auto next_action = 1; // mb_btns[1]
	{
		const std::string mb_text = msg + "\n" + question;

		const SDL_MessageBoxButtonData mb_btns[] = {
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes" },
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No" },
		};

		SDL_MessageBoxData mb{
			SDL_MESSAGEBOX_ERROR,
			nullptr,
			"Fatal Error",
			mb_text.c_str(),
			sizeof(mb_btns) / sizeof(mb_btns[0]), // number of buttons
			mb_btns,
			nullptr,
		};

		// If the message box fails to show, we will just log and continue
		if (SDL_ShowMessageBox(&mb, &next_action) < 0)
			util::log::send(util::log_level::error, "SDL_ShowMessageBox failed: ", SDL_GetError());
	}

	// The user wants to report the issue. We will construct a mailto link and open it
	if (next_action == 0) {
		// Construct the body of the email
		std::stringstream body{};
		body << "Please describe what you were doing when the crash occurred:\n\n";

		auto log_path = util::log::log_path();
		if (log_path.has_value()) {
			// There is a log file, but we can't attach it using a mailto link from the
			// research I have done.
			body << "Please attach the log file to this email. It is located at " << log_path.value();
		}

		// Construct the mailto link and open it
		util::mailto mail{ 
			"realnickk1@gmail.com", // Just going to use my dev email for now
			"steampunk-game - Crash Report",
			body.str()
		};
		platform::open_url(mail.to_string());
	}

	// Dump the memory and exit. Ripperoni.
	platform::dump_and_exit();
}

void init_world() {
	auto world = game::world::instance();

	// If we get an interrupt signal, we will stop the scheduler to allow for a graceful
	// shutdown of the game
	platform::on_close([world](int signal) {
		// Signal a scheduler stop and hope for the best
		// 9/10 times this will work perfectly fine but if you have a long running job
		// it will cause a hang
		util::log::send(util::log_level::info, "Interrupt received");
		world->stop(true);
	});

	// Start the scheduler, which will also block this thread until the scheduler is
	// gracefully stopped
	world->set_fps(0);
	world->start();

	// If we get here, the scheduler has been stopped (potentially by an interrupt signal)
	// and we can safely exit
	util::log::send(util::log_level::info, "Scheduler stop");
}

int main(int argc, char* argv[]) {
	// This will handle sneaky exceptions.
	std::set_terminate(exception_filter);

	// Initialize the log system, and write to console if we are in debug mode
	util::log::init(true);

	init_world();

	// This is the desired "normal" exit point of the program. Here, we clean up whatever
	// may be left over and exit.
	SDL_Quit();

	return 0;
}
