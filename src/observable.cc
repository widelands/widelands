/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <boost/foreach.hpp>

#include "observer.h"
#include "observable.h"

#define foreach BOOST_FOREACH

Observable::~Observable() {
	observers_.clear();
}

void Observable::add_observer(Observer * o) {
	observers_.insert(o);
}

void Observable::remove_observer(Observer * o) {
	observers_.erase(o);
}

void Observable::notify_observers() {
	foreach(Observer * o, observers_)
		o->observed_changed();
}
