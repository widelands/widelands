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
#include <thread>

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

// Remember the current thread as the initializer thread. Throws an exception if this is already
// set.
void set_initializer_thread();
// Whether the current thread is the initializer thread. If the initializer thread
// has not been set yet and the argument is `true`, calls `set_initializer_thread()`.
bool is_initializer_thread(bool set_if_not_set_yet);
bool is_initializer_thread_set();

// Wrapper for a mutex that can be locked and unlocked using a MutexLock
struct MutexLockHandler {
	explicit MutexLockHandler();
	MutexLockHandler(const MutexLockHandler&) = default;
	~MutexLockHandler() {
	}

	std::recursive_mutex* mutex() const {
		return mutex_.get();
	}

	// The global mutex
	static MutexLockHandler g_mutex;

private:
	std::unique_ptr<std::recursive_mutex> mutex_;
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
 * If the `optional` parameter is `true`, the constructor returns immediately without
 * locking the mutex if it is already being held by another thread. In this case,
 * `is_valid()` will return `false`.
 * Some good explanations here: https://stackoverflow.com/questions/14888027/mutex-lock-threads
 */
struct MutexLock {
	explicit MutexLock(bool optional = false);             // claims the global mutex
	explicit MutexLock(MutexLockHandler*, bool optional);  // claims a specified other mutex
	~MutexLock();
	bool is_valid() const {
		return mutex_ != nullptr;
	}

private:
	MutexLockHandler* mutex_;  // not owned!
};

/*
 * Informs the drawing thread to run the given function ASAP. Intended for
 * image-checking functions, such as MapObjectDescr::check_representative_image().
 * This must not be instantiated directly – use `instantiate()` instead,
 * which will also take care of publishing the note.
 */
struct NoteDelayedCheck {
	CAN_BE_SENT_AS_NOTE(NoteId::DelayedCheck)
	static void instantiate(const void*, const std::function<void()>&, bool wait_until_completion);

	static void set_interrupt(const std::thread::id&, bool);

	const void* caller;
	const std::function<void()> run;

private:
	NoteDelayedCheck(const void* c, const std::function<void()>& f) : caller(c), run(f) {
	}
};
// Informs the caller that any pending NoteDelayedCheck's requested by this object must NOT
// be executed any more. The destructor of EVERY class that may dispatch a NoteDelayedCheck
// MUST publish such a notification!
struct NoteDelayedCheckCancel {
	CAN_BE_SENT_AS_NOTE(NoteId::DelayedCheckCancel)

	const void* caller;

	NoteDelayedCheckCancel(const void* c) : caller(c) {
	}
};

#endif  // end of include guard: WL_BASE_MULTITHREADING_H
