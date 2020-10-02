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

#include "base/wexception.h"

static std::unique_ptr<std::thread::id> initializer_thread;

void set_initializer_thread() {
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
			Notifications::publish(NoteThreadSafeFunction([fn, &done]() {
				fn();
				done = true;
			}));
			while (!done) {
				SDL_Delay(5);
			}
		} else {
			Notifications::publish(NoteThreadSafeFunction(fn));
		}
	}
}

static std::map<MutexLock::ID, std::recursive_mutex> g_mutex;

MutexLock::MutexLock(ID i) : id_(i) {
	g_mutex[id_].lock();
}
MutexLock::MutexLock(ID i, std::function<void()> run_while_waiting) : id_(i) {
	while (!g_mutex[id_].try_lock()) {
		run_while_waiting();
		SDL_Delay(5);
	}
}
MutexLock::~MutexLock() {
	g_mutex.at(id_).unlock();
}
