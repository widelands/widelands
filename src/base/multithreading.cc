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

#include <list>
#include <memory>
#include <thread>

#include "base/wexception.h"

static std::unique_ptr<std::thread::id> initializer_thread;

void set_initializer_thread() {
	if (initializer_thread) {
		throw wexception("attempt to set initializer thread again");
	}
	initializer_thread.reset(new std::thread::id(std::this_thread::get_id()));
}

bool is_initializer_thread_set() {
	return initializer_thread != nullptr;
}

bool is_initializer_thread(const bool set_if_not_set_yet) {
	if (!initializer_thread.get()) {
		if (set_if_not_set_yet) {
			set_initializer_thread();
		} else {
			return false;
		}
	}
	return *initializer_thread == std::this_thread::get_id();
}

void NoteDelayedCheck::instantiate(std::function<void()> fn) {
	if (!is_initializer_thread_set()) {
		throw wexception("NoteDelayedCheck::instantiate: initializer thread was not set yet");
	} else if (is_initializer_thread(false)) {
		// The initializer thread may run the desired function directly. Publishing
		// it might result in a deadlock if the caller needs to wait for a result.
		fn();
	} else {
		// All other threads must ask it politely to do this for them.
		Notifications::publish(NoteDelayedCheck(fn));
	}
}

MutexLock::MutexLock(const bool optional) : MutexLock(MutexLockHandler::get(), optional) {
}
MutexLock::MutexLock(MutexLockHandler* m, const bool optional) : mutex_(m ? m->mutex() : nullptr) {
	if (mutex_) {
		if (optional) {
			if (!mutex_->try_lock()) {
				mutex_ = nullptr;
			}
		} else {
			mutex_->lock();
		}
	}
}
MutexLock::~MutexLock() {
	if (mutex_) {
		mutex_->unlock();
	}
}

static std::list<MutexLockHandler> handlers;

MutexLockHandler* MutexLockHandler::get() {
	return handlers.empty() ? nullptr : &handlers.back();
}

MutexLockHandler& MutexLockHandler::push() {
	handlers.push_back(MutexLockHandler());
	return handlers.back();
}

void MutexLockHandler::pop(MutexLockHandler& h) {
	if (handlers.empty()) {
		throw wexception("MutexLockHandler::pop(): Stack is empty");
	}
	if (handlers.back().mutex() != h.mutex()) {
		throw wexception("MutexLockHandler::pop(): Mismatch");
	}
	handlers.pop_back();
}

MutexLockHandler::MutexLockHandler() : mutex_(new std::recursive_mutex()) {
}
