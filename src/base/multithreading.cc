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

#include "base/wexception.h"

MutexLock::MutexLock() : MutexLock(MutexLockHandler::get()) {
}
MutexLock::MutexLock(MutexLockHandler* m) : mutex_(m ? m->mutex() : nullptr) {
	if (mutex_) {
		pthread_mutex_lock(mutex_);
	}
}
MutexLock::~MutexLock() {
	if (mutex_) {
		pthread_mutex_unlock(mutex_);
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

MutexLockHandler::MutexLockHandler() : mutex_(new pthread_mutex_t(PTHREAD_MUTEX_INITIALIZER)) {
	pthread_mutexattr_t a;
	pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mutex_.get(), &a);
}
