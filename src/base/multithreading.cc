/*
 * Copyright (C) 2020-2024 by the Widelands Development Team
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
#include <iostream>
#include <map>
#include <memory>
#include <set>

#include <SDL_timer.h>

#include "base/log.h"
#include "base/mutex.h"
#include "base/wexception.h"

static const std::thread::id kNoThread;
static std::thread::id initializer_thread(kNoThread);
static std::thread::id logic_thread(kNoThread);

// To protect the global mutex list
std::mutex MutexLock::s_mutex_;

std::vector<std::function<void()>> MutexLock::stay_responsive_;

/* When a thread X is handling a thread-safe-function note sent from another thread Y, and Y is
 * waiting for completion, then we add a pair {X, Y} here to register that for the duration
 * of the note handling, all resources (especially mutexes) owned by Y are also owned by X.
 */
static std::vector<std::pair<std::thread::id, std::thread::id>> acting_as_another_thread;

/** Wrapper around a STL recursive mutex plus some metadata. */
struct MutexRecord {
	std::recursive_mutex mutex;  ///< The actual mutex.
	std::set<std::thread::id>
	   waiting_threads;  ///< The threads that are currently trying to lock this mutex.
	std::thread::id current_owner =
	   kNoThread;  ///< The thread that has currently locked this mutex (may be #kNoThread).
	size_t ownership_count = 0;  ///< How many times this mutex was locked.
};
static std::map<MutexLock::ID, MutexRecord> g_all_mutex_records;

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

static std::string thread_name(const std::thread::id id) {
	if (id == kNoThread) {
		return "No thread";
	}
	if (id == initializer_thread) {
		return "Initializer thread";
	}
	if (id == logic_thread) {
		return "Logic thread";
	}
	return "Auxiliary thread";  // We don't have that many threads currently...
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
			const std::thread::id outer_thread = std::this_thread::get_id();

			volatile bool done = false;

			// Some codepaths want to perform special error handling, so we catch
			// any errors and forward them to the caller in a thread-safe manner.
			// Only if the caller waits for completion of course.
			const std::exception* error = nullptr;

			Notifications::publish(
			   NoteThreadSafeFunction([fn, &done, &error, rethrow_errors, outer_thread]() {
				   const std::thread::id inner_thread = std::this_thread::get_id();
				   acting_as_another_thread.emplace_back(inner_thread, outer_thread);

				   try {
					   fn();
				   } catch (const std::exception& e) {
					   if (rethrow_errors) {
						   error = &e;
					   } else {
						   done = true;
						   acting_as_another_thread.pop_back();
						   throw;
					   }
				   }
				   done = true;
				   acting_as_another_thread.pop_back();
			   }));
			while (!done) {
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

MutexLock::ID MutexLock::last_custom_mutex_ = MutexLock::ID::kLastID;
MutexLock::ID MutexLock::create_custom_mutex() {
	last_custom_mutex_ = static_cast<MutexLock::ID>(static_cast<uint32_t>(last_custom_mutex_) + 1);
#ifdef MUTEX_LOCK_DEBUG
	log_dbg("Create custom mutex #%d.",
	        static_cast<int>(last_custom_mutex_) - static_cast<int>(MutexLock::ID::kLastID));
#endif
	return last_custom_mutex_;
}

static std::string to_string(const MutexLock::ID i) {
	switch (i) {
	case MutexLock::ID::kNone:
		return "None";
	case MutexLock::ID::kMutexInternal:
		return "MutexInternal";
	case MutexLock::ID::kLogicFrame:
		return "LogicFrame";
	case MutexLock::ID::kObjects:
		return "Objects";
	case MutexLock::ID::kCommands:
		return "Commands";
	case MutexLock::ID::kMessages:
		return "Messages";
	case MutexLock::ID::kPathfinding:
		return "Pathfinding";
	case MutexLock::ID::kIBaseVisualizations:
		return "IBaseVisualizations";
	case MutexLock::ID::kLog:
		return "Log";
	case MutexLock::ID::kLua:
		return "Lua";
	case MutexLock::ID::kI18N:
		return "i18n";
	default:
		return std::string("Custom lock #") +
		       std::to_string(static_cast<int>(i) - static_cast<int>(MutexLock::ID::kLastID));
	}
}

constexpr uint32_t kMutexPriorityLockInterval = 2;
constexpr uint32_t kMutexNormalLockInterval = 30;
constexpr uint32_t kMutexLogicFrameLockInterval = 400;

void MutexLock::push_stay_responsive_function(std::function<void()> fn) {
	MutexLock guard(MutexLock::ID::kMutexInternal);
	stay_responsive_.emplace_back(fn);
}

void MutexLock::pop_stay_responsive_function() {
	assert(!stay_responsive_.empty());
	MutexLock guard(MutexLock::ID::kMutexInternal);
	stay_responsive_.pop_back();
}

static std::thread::id prev_self;
static std::thread::id prev_owner;
static MutexLock::ID prev_lock = MutexLock::ID::kNone;

MutexLock::MutexLock(const ID i) : id_(i) {
	if (id_ == ID::kNone) {
		return;
	}

	const uint32_t start_time = SDL_GetTicks();

#ifdef MUTEX_LOCK_DEBUG
	uint32_t counter = 0;
	if (id_ != ID::kLog) {
		log_dbg("Starting to lock mutex %s ...", to_string(id_).c_str());
	} else {
		std::cout << "Starting to lock mutex Log..." << std::endl;
	}
#endif

	const std::thread::id self = std::this_thread::get_id();
	s_mutex_.lock();
	MutexRecord& record = g_all_mutex_records[id_];

	if (record.current_owner != kNoThread) {
		for (const auto& pair : acting_as_another_thread) {
			if (pair.first == self && pair.second == record.current_owner) {
				s_mutex_.unlock();  // Must unlock before verb_log_dbg()
				if (id_ != ID::kLog) {
					if (id_ != prev_lock || self != prev_self || record.current_owner != prev_owner) {
						prev_lock = id_;
						prev_self = self;
						prev_owner = record.current_owner;
						verb_log_dbg("%s skips locking mutex %s owned by wrapping thread %s",
						             thread_name(self).c_str(), to_string(id_).c_str(),
						             thread_name(record.current_owner).c_str());
					}
				} else {
					std::cout << "Skip re-locking Log mutex" << std::endl;
				}
				id_ = ID::kNone;
				return;
			}
		}
	}

	// When several threads are waiting to grab the same mutex, the first one is advantaged
	// by giving it a lower sleep time between attempts. This keeps overall waiting times low.
	// The Logic Frame mutex's extended sleep time is higher because it's locked much longer.
	const bool has_priority = (record.waiting_threads.empty() || is_initializer_thread());

	if (record.waiting_threads.count(self) != 0) {
		if (id_ == ID::kLog) {
			// Above only checked borrowing situations. Here we check for the same thread already
			// waiting somewhere up the stack. Can happen because of stay responsive functions.
			std::cout << thread_name(self) << " is already waiting for mutex kLog, skip locking"
			          << std::endl;
			s_mutex_.unlock();
			id_ = ID::kNone;
			return;
		}

		std::cout << thread_name(self) << " is already waiting for mutex " << to_string(id_)
		          << std::endl;
	}

	assert(record.waiting_threads.count(self) == 0);
	record.waiting_threads.insert(self);
	s_mutex_.unlock();

	const uint32_t sleeptime = has_priority             ? kMutexPriorityLockInterval :
	                           (id_ == ID::kLogicFrame) ? kMutexLogicFrameLockInterval :
                                                         kMutexNormalLockInterval;
	if (!has_priority && record.current_owner != self) {
		SDL_Delay(sleeptime);
	}

	uint32_t last_function_call = 0;
	uint32_t last_log_time = 0;
	while (!record.mutex.try_lock()) {
		const uint32_t now = SDL_GetTicks();
		if (now - start_time > 1000 && now - last_log_time > 1000) {
			last_log_time = now;
			if (id_ != ID::kLog) {
				verb_log_dbg("WARNING: %s locking mutex %s, already waiting for %d ms",
				             thread_name(self).c_str(), to_string(id_).c_str(), now - start_time);
			} else if (g_verbose) {
				// not including format() for the time info
				std::cout << "WARNING: " << thread_name(self) << " locking mutex Log still waiting"
				          << std::endl;
			}
		}

		if (now - last_function_call > sleeptime) {
			if (id_ != MutexLock::ID::kMutexInternal) {
				MutexLock guard(MutexLock::ID::kMutexInternal);
				if (!stay_responsive_.empty()) {
					stay_responsive_.back()();
				} else if (id_ != ID::kLog) {
					verb_log_dbg("WARNING: Mutex locking: No responsiveness function set");
				} else if (g_verbose) {
					std::cout << "WARNING: Mutex locking: No responsiveness function set" << std::endl;
				}
			}

			last_function_call = SDL_GetTicks();

			// Check for deadlocks. Does not account for situations involving more than two threads.
			s_mutex_.lock();
			assert(record.current_owner != self);
			if (record.current_owner == kNoThread) {
				s_mutex_.unlock();
				continue;
			}
			for (const auto& pair : g_all_mutex_records) {
				if (pair.second.current_owner == self &&
				    pair.second.waiting_threads.count(record.current_owner) > 0) {
					// Ouch! Break the deadlock by throwing an exception with a helpful message.
					std::string info = "Deadlock! ";
					info += thread_name(self);
					info += " is trying to lock mutex ";
					info += to_string(id_);
					info += " owned by ";
					info += thread_name(record.current_owner);
					info += ", which is trying to lock ";
					info += to_string(pair.first);
					info += ". First thread owns: ";
					for (const auto& mutex_pair : g_all_mutex_records) {
						if (mutex_pair.second.current_owner == self) {
							info += to_string(mutex_pair.first);
							info += "; ";
						}
					}
					info += "Second thread owns: ";
					for (const auto& mutex_pair : g_all_mutex_records) {
						if (mutex_pair.second.current_owner == record.current_owner) {
							info += to_string(mutex_pair.first);
							info += "; ";
						}
					}

					s_mutex_.unlock();
					if (id_ != ID::kLog) {
						log_err("%s", info.c_str());
					} else {
						std::cout << info << std::endl;
					}
					throw wexception("%s", info.c_str());
				}
			}
			s_mutex_.unlock();
		} else {
			SDL_Delay(sleeptime - (now - last_function_call));
		}

#ifdef MUTEX_LOCK_DEBUG
		++counter;
#endif
	}

	{
		std::lock_guard<std::mutex> guard(s_mutex_);
		assert(record.waiting_threads.count(self) > 0);
		record.waiting_threads.erase(self);
	}

	assert(record.current_owner == kNoThread || record.current_owner == self);
	record.current_owner = self;
	record.ownership_count++;

#ifdef MUTEX_LOCK_DEBUG
	if (id_ != ID::kLog) {
		log_dbg("Locking mutex %s took %ums (%u function calls)", to_string(id_).c_str(),
		        SDL_GetTicks() - start_time, counter);
	} else {
		// not including format() for the time info
		std::cout << "Mutex Log is now locked." << std::endl;
	}
#endif
}
MutexLock::~MutexLock() {
	if (id_ == ID::kNone) {
		return;
	}

#ifdef MUTEX_LOCK_DEBUG
	if (id_ != ID::kLog) {
		log_dbg("Unlocking mutex %s", to_string(id_).c_str());
	} else {
		std::cout << "Unlocking mutex Log" << std::endl;
	}
#endif

	std::lock_guard<std::mutex> guard(s_mutex_);
	MutexRecord& record = g_all_mutex_records.at(id_);

	assert(record.ownership_count > 0);
	--record.ownership_count;
	assert(record.current_owner == std::this_thread::get_id());
	if (record.ownership_count == 0) {
		record.current_owner = kNoThread;
	}

	record.mutex.unlock();
}
