/*
 * Copyright (C) 2020 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "base/multithreading.h"

#include <map>
#include <memory>

#include <SDL_timer.h>

#include "base/log.h"
#include "base/wexception.h"

// Uncomment this to get masses of log output to debug hangs and deadlocks
// #define MUTEX_LOCK_DEBUG

static std::unique_ptr<std::thread::id> initializer_thread;

void set_initializer_thread() {
	log_info("Setting initializer thread.");

	if (initializer_thread) {
		throw wexception("attempt to set initializer thread again");
	}

	initializer_thread.reset(new std::thread::id(std::this_thread::get_id()));
}

bool is_initializer_thread() {
	return (initializer_thread != nullptr) && (*initializer_thread == std::this_thread::get_id());
}

uint32_t NoteThreadSafeFunction::next_id_(0);

void NoteThreadSafeFunction::instantiate(const std::function<void()>& fn,
                                         const bool wait_until_completion) {
	if (!initializer_thread) {
		throw wexception("NoteThreadSafeFunction::instantiate: initializer thread was not set yet");
	} else if (is_initializer_thread()) {
		// The initializer thread may run the desired function directly. Publishing
		// it might result in a deadlock if the caller needs to wait for a result.
		fn();
	} else {
		// All other threads must ask it politely to do this for them.
		if (wait_until_completion) {
			bool done = false;

			// Some codepaths want to perform special error handling, so we catch
			// any errors and forward them to the caller in a thread-safe manner.
			// Only if the caller waits for completion of course.
			const std::exception* error = nullptr;

			Notifications::publish(NoteThreadSafeFunction([fn, &done, &error]() {
				try {
					fn();
				} catch (const std::exception& e) {
					error = &e;
				}
				done = true;
			}));
			while (!done) {
				// Wait until the NoteThreadSafeFunction has been handled.
				// Since `done` was passed by address, it will set to
				// `true` when the function has been executed.
				SDL_Delay(2);
			}

			if (error) {
				throw *error;
			}
		} else {
			Notifications::publish(NoteThreadSafeFunction(fn));
		}
	}
}

// TODO(Nordfriese): All mutexes are global. If there is a bottleneck in the future where
// a mutex needs to be locked only on one specific object really, then each object of
// this class should be given a mutex on its own. Performance! Currently there are no
// places where one mutex per object would be enough, so this is not implemented yet.
static std::map<MutexLock::ID, std::recursive_mutex> g_mutex;

#ifdef MUTEX_LOCK_DEBUG
static std::string to_string(MutexLock::ID i) {
	switch (i) {
	case MutexLock::ID::kLogicFrame:
		return "LogicFrame";
	case MutexLock::ID::kObjects:
		return "Objects";
	case MutexLock::ID::kCommands:
		return "Commands";
	case MutexLock::ID::kMessages:
		return "Messages";
	case MutexLock::ID::kIBaseVisualizations:
		return "IBaseVisualizations";
	}
	NEVER_HERE();
}
#endif

MutexLock::MutexLock(ID i) : id_(i) {

#ifdef MUTEX_LOCK_DEBUG
	uint32_t time = 0;
	time = SDL_GetTicks();
	log_dbg("Starting to lock mutex %s ...", to_string(id_).c_str());
#endif

	g_mutex[id_].lock();

#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Locking mutex %s took %ums", to_string(id_).c_str(), SDL_GetTicks() - time);
#endif

}
MutexLock::MutexLock(ID i, const std::function<void()>& run_while_waiting) : id_(i) {

#ifdef MUTEX_LOCK_DEBUG
	uint32_t time = 0, counter = 0;
	time = SDL_GetTicks();
	log_dbg("Starting to lock mutex %s (run_while_waiting) ...", to_string(id_).c_str());
#endif

	while (!g_mutex[id_].try_lock()) {
		run_while_waiting();
		SDL_Delay(2);

#ifdef MUTEX_LOCK_DEBUG
		++counter;
#endif

	}

#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Locking mutex %s took %ums (%u function calls)", to_string(id_).c_str(), SDL_GetTicks() - time, counter);
#endif

}
MutexLock::~MutexLock() {

#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Unlocking mutex %s", to_string(id_).c_str());
#endif

	g_mutex.at(id_).unlock();
}
