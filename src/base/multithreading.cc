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
