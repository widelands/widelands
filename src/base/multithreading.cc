/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "base/multithreading.h"

#include <atomic>
#include <map>
#include <memory>

#include <SDL_timer.h>

#include "base/log.h"
#include "base/wexception.h"

static const std::thread::id kNoThread;
static std::thread::id initializer_thread(kNoThread);
static std::thread::id logic_thread(kNoThread);

void set_initializer_thread() {
	verb_log_info("Setting initializer thread.");

	if (initializer_thread != kNoThread) {
		throw wexception("attempt to set initializer thread again");
	}

	initializer_thread = std::this_thread::get_id();
}

void set_logic_thread() {
	verb_log_info("Setting logic thread.");

	if (initializer_thread == kNoThread) {
		throw wexception("attempt to set logic thread before initializer thread");
	}
	if (logic_thread != kNoThread) {
		throw wexception("attempt to set logic thread again");
	}
	if (is_initializer_thread()) {
		throw wexception("initializer thread can not be the logic thread");
	}

	logic_thread = std::this_thread::get_id();
}

bool is_initializer_thread() {
	return initializer_thread == std::this_thread::get_id();
}
bool is_logic_thread() {
	return logic_thread == std::this_thread::get_id();
}

uint32_t NoteThreadSafeFunction::next_id_(0);

void NoteThreadSafeFunction::instantiate(const std::function<void()>& fn,
                                         const bool wait_until_completion,
                                         const bool rethrow_errors) {
	if (initializer_thread == kNoThread) {
		throw wexception("NoteThreadSafeFunction::instantiate: initializer thread was not set yet");
	}
	if (is_initializer_thread()) {
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

			Notifications::publish(NoteThreadSafeFunction([fn, &done, &error, rethrow_errors]() {
				try {
					fn();
				} catch (const std::exception& e) {
					if (rethrow_errors) {
						error = &e;
					} else {
						done = true;
						throw;
					}
				}
				done = true;
			}));
			while (!done) {  // NOLINT
				// Wait until the NoteThreadSafeFunction has been handled.
				// Since `done` was passed by address, it will set to
				// `true` when the function has been executed.
				SDL_Delay(2);
			}

			if (error != nullptr) {
				throw *error;
			}
		} else {
			Notifications::publish(NoteThreadSafeFunction(fn));
		}
	}
}

/** Wrapper around a STL recursive mutex plus some metadata. */
struct MutexRecord {
	std::recursive_mutex mutex;  ///< The actual mutex.
	std::atomic_uint nr_waiting_threads = {
	   0};  ///< How many threads are currently trying to lock this mutex.
	std::thread::id current_owner =
	   kNoThread;  ///< The thread that has currently locked this mutex (may be #kNoThread).
	size_t ownership_count = 0;  ///< How many times this mutex was locked.
};
static std::map<MutexLock::ID, MutexRecord> g_mutex;

MutexLock::ID MutexLock::last_custom_mutex_ = MutexLock::ID::kI18N;
MutexLock::ID MutexLock::create_custom_mutex() {
	last_custom_mutex_ = static_cast<MutexLock::ID>(static_cast<uint32_t>(last_custom_mutex_) + 1);
#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Create custom mutex #%u.", static_cast<uint32_t>(last_custom_mutex_));
#endif
	return last_custom_mutex_;
}

static std::string to_string(const MutexLock::ID i) {
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
	case MutexLock::ID::kLog:
		return "Log";
	case MutexLock::ID::kLua:
		return "Lua";
	case MutexLock::ID::kI18N:
		return "i18n";
	}
	return std::string("Custom lock #") + std::to_string(static_cast<unsigned>(i));
}

constexpr uint32_t kMutexPriorityLockInterval = 2;
constexpr uint32_t kMutexNormalLockInterval = 30;
constexpr uint32_t kMutexLogicFrameLockInterval = 400;

// To protect the global mutex list
std::mutex MutexLock::s_mutex_;
MutexLock::MutexLock(ID i) : MutexLock(i, []() {}) {
}
MutexLock::MutexLock(ID i, const std::function<void()>& run_while_waiting) : id_(i) {
	const uint32_t start_time = SDL_GetTicks();
#ifdef MUTEX_LOCK_DEBUG
	uint32_t counter = 0;
	log_dbg("Starting to lock mutex %s (run_while_waiting) ...", to_string(id_).c_str());
#endif

	const std::thread::id self = std::this_thread::get_id();
	s_mutex_.lock();
	MutexRecord& record = g_mutex[id_];
	s_mutex_.unlock();

	// When several threads are waiting to grab the same mutex, the first one is advantaged
	// by giving it a lower sleep time between attempts. This keeps overall waiting times low.
	// The Logic Frame mutex's extended sleep time is higher because it's locked much longer.
	const bool has_priority = (record.nr_waiting_threads.load() == 0 || is_initializer_thread());
	const uint32_t sleeptime = has_priority             ? kMutexPriorityLockInterval :
	                           (id_ == ID::kLogicFrame) ? kMutexLogicFrameLockInterval :
                                                         kMutexNormalLockInterval;
	++record.nr_waiting_threads;
	if (!has_priority && record.current_owner != self) {
		SDL_Delay(sleeptime);
	}

	uint32_t last_function_call = 0;
	while (!record.mutex.try_lock()) {
		const uint32_t now = SDL_GetTicks();
		if (now - start_time > 1000) {
			verb_log_dbg("WARNING: Locking mutex %s, already waiting for %d ms",
			             to_string(id_).c_str(), now - start_time);
		}

		if (now - last_function_call > sleeptime) {
			run_while_waiting();
			last_function_call = SDL_GetTicks();
		} else {
			SDL_Delay(sleeptime - (now - last_function_call));
		}

#ifdef MUTEX_LOCK_DEBUG
		++counter;
#endif
	}

	assert(record.nr_waiting_threads > 0);
	--record.nr_waiting_threads;

	assert(record.current_owner == kNoThread || record.current_owner == self);
	record.current_owner = self;
	record.ownership_count++;

#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Locking mutex %s took %ums (%u function calls)", to_string(id_).c_str(),
	        SDL_GetTicks() - start_time, counter);
#endif
}
MutexLock::~MutexLock() {

#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Unlocking mutex %s", to_string(id_).c_str());
#endif

	s_mutex_.lock();
	MutexRecord& record = g_mutex.at(id_);

	assert(record.ownership_count > 0);
	--record.ownership_count;
	assert(record.current_owner == std::this_thread::get_id());
	if (record.ownership_count == 0) {
		record.current_owner = kNoThread;
	}

	record.mutex.unlock();
	s_mutex_.unlock();
}
