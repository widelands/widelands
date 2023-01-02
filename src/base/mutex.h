/*
 * Copyright (C) 2022-2023 by the Widelands Development Team
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

#ifndef WL_BASE_MUTEX_H
#define WL_BASE_MUTEX_H

#include <functional>
#include <mutex>

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
		kLogicFrame,           ///< The game logic progression.
		kObjects,              ///< MapObjects are being modified.
		kCommands,             ///< The game's command queue is being modified.
		kMessages,             ///< In-game messages are being modified.
		kIBaseVisualizations,  ///< The InteractiveBase's overlays are being updated.
		kI18N,                 ///< The gettext backend.
		kLua,                  ///< Lua scripts.
		kLog,                  ///< Log output backend.

		kLastID  ///< Last entry in the enum, do not use.
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

#endif  // end of include guard: WL_BASE_MUTEX_H
