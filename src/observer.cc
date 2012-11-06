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

#include "observable.h"

#include "observer.h"

#define foreach BOOST_FOREACH

Observer::~Observer() {
	foreach(Observable* o, observed_)
		o->remove_observer(this);
	observed_.clear();
}

void Observer::start_observing(const Observable* const_o) {
	Observable* o = const_cast<Observable*>(const_o);
	o->add_observer(this);
	observed_.insert(o);
}

void Observer::stop_observing(const Observable* const_o) {
	Observable* o = const_cast<Observable*>(const_o);
	o->remove_observer(this);
	observed_.erase(o);
}


