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

#ifndef WL_BASE_MULTITHREADING_H
#define WL_BASE_MULTITHREADING_H

#include <memory>
#include <mutex>

#include <pthread.h>

// Wrapper for a mutex that can be locked and unlocked using a MutexLock
struct MutexLockHandler {
	explicit MutexLockHandler();
	MutexLockHandler(const MutexLockHandler&) = default;
	~MutexLockHandler() {
	}

	std::recursive_mutex* mutex() const {
		return mutex_.get();
	}

	// get the global mutex, if any
	static MutexLockHandler* get();

	// these two functions are intended to be used ONLY by UI::Panel::do_run
	static MutexLockHandler& push();
	static void pop(MutexLockHandler&);

private:
	std::shared_ptr<std::recursive_mutex> mutex_;
};

/*
 * This struct handles the global mutex. The logic and drawing code are executed in
 * parallel, but some functions assume that the game state doesn't change while they
 * are executing. Create a MutexLock when entering such a critical section. This will
 * suspend the current thread until the lock is available, then claim the lock for
 * this thread. You HAVE TO ensure the MutexLock's deletion as soon as you leave the
 * section, otherwise other threads that need the lock will not be able to resume!
 * In order to prevent the threads suspending each other for too long, claim the
 * lock only for the shortest pieces of code possible.
 * The intended usage is:
 *     non_critical_code();
 *     {
 *         MutexLock m;
 *         critical_code();
 *     }
 *     non_critical_code();
 * Note that mutexes are recursive, that is, a thread may lock a mutex several times,
 * but needs to free it the same number of times before another thread can claim it.
 * Some good explanations here: https://stackoverflow.com/questions/14888027/mutex-lock-threads
 */
struct MutexLock {
	explicit MutexLock();                   // claims the global mutex
	explicit MutexLock(MutexLockHandler*);  // claims a specified other mutex
	~MutexLock();
	bool is_valid() const {
		return mutex_ != nullptr;
	}

private:
	std::recursive_mutex* mutex_;  // not owned
};

#endif  // end of include guard: WL_BASE_MULTITHREADING_H
