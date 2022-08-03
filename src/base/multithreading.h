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

#ifndef WL_BASE_MULTITHREADING_H
#define WL_BASE_MULTITHREADING_H

#include <memory>
#include <mutex>
#include <thread>

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

// Uncomment this to get masses of log output to debug hangs and deadlocks
// #define MUTEX_LOCK_DEBUG

// Remember that the current thread is the thread that initialized the graphics system.
// This function needs to be called exactly once when Widelands starts.
void set_initializer_thread();
// Whether the current thread is the same that called `set_initializer_thread()` on startup
bool is_initializer_thread();
// Same for the game logic thread
void set_logic_thread();
bool is_logic_thread();

/*
 * Informs the drawing thread to run the given function ASAP.
 * This must not be instantiated directly – use `instantiate()` instead,
 * which will also take care of publishing the note.
 */
struct NoteThreadSafeFunction {
	CAN_BE_SENT_AS_NOTE(NoteId::ThreadSafeFunction)
	static void instantiate(const std::function<void()>&,
	                        bool wait_until_completion,
	                        bool rethrow_errors = true);

	const std::function<void()> run;

	const uint32_t id;

private:
	NoteThreadSafeFunction(const std::function<void()>& f) : run(f), id(next_id_++) {
	}

	static uint32_t next_id_;
};

// Used only by Panel code
struct NoteThreadSafeFunctionHandled {
	CAN_BE_SENT_AS_NOTE(NoteId::ThreadSafeFunctionHandled)

	const uint32_t id;

	explicit NoteThreadSafeFunctionHandled(uint32_t i) : id(i) {
	}
};

/**
 * THREADSAFE is a convenience wrapper macro around NoteThreadSafeFunction::instantiate
 * with a non-void return value. It calls the given function F with the given parameters
 * in the initializer thread, and returns the return value of F implicitly converted to R.
 *
 * For an overloaded function F, you need to use THREADSAFE_T with the explicit signature S of F.
 * For a function with the declaration
 *     int foo (double bar, const std::string& baz)
 * the signature would be
 *     int(*)(double, const std::string&)
 *
 * With newer standards than c++11 this could be done so much more elegantly. Solution based on
 * https://stackoverflow.com/a/36492736 and https://stackoverflow.com/a/37912463
 */
#define THREADSAFE_T(R, S, F, ...) multithreading_impl::wrapper<R, S, F>(__VA_ARGS__)
#define THREADSAFE(R, F, ...) THREADSAFE(R, decltype(&F), F, __VA_ARGS__)
namespace multithreading_impl {
template <typename F, F f> struct WrapperImpl;
template <typename R, typename... Args, R (*f)(Args...)> struct WrapperImpl<R (*)(Args...), f> {
	static R wrap(Args... args) {
		R result;
		NoteThreadSafeFunction::instantiate(
		   [&result, &args...]() { result = f(args...); }, true, true);
		return result;
	}
};
template <typename R, typename F, F f, typename... Args> R wrapper(Args... args) {
	return WrapperImpl<F, f>::wrap(args...);
}
}  // namespace multithreading_impl

/* Ensures that critical pieces of code are executed by only one thread at a time.
 * More precisely: If n pieces of code C1,…,Cn are structured like this:
 *    {
 *        MutexLock m(...);
 *        do_something();
 *    }
 * Then there will never, at any given time, two threads T1 and T2
 * be running any pieces Cx and Cy (1 <= x,y <= n) concurrently.
 * Each thread may, of course, call Cx from within Cy without problems.
 *
 * Use MutexLocks sparingly. They are there to safeguard bottlenecks in the
 * code where concurrency must not be allowed, at the cost of performance.
 */
class MutexLock {
public:
	// Which mutex to lock. Each entry corresponds to a different mutex.
	enum class ID : uint32_t {
		kLogicFrame,
		kObjects,
		kCommands,
		kMessages,
		kIBaseVisualizations,
		kI18N,
		kLua,
		kLog
	};

	static ID create_custom_mutex();
	explicit MutexLock(ID);
	explicit MutexLock(ID, const std::function<void()>& run_while_waiting);
	~MutexLock();

private:
	ID id_;
	static ID last_custom_mutex_;
	static std::mutex s_mutex_;
};

#endif  // end of include guard: WL_BASE_MULTITHREADING_H
