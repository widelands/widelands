/*
 * Copyright (C) 2020-2023 by the Widelands Development Team
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
	explicit NoteThreadSafeFunction(const std::function<void()>& f) : run(f), id(next_id_++) {
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

#endif  // end of include guard: WL_BASE_MULTITHREADING_H
