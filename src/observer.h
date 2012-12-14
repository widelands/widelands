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

#ifndef OBSERVER_H
#define OBSERVER_H

#include <set>

#include <boost/utility.hpp>

class Observable;

/*
 * An Observer watches an Observable and gets notified when it
 * changes. This could have been done with boost::signal, but that proved
 * to be too slow.
 *
 * Note that an Observable must life longer than all its Observers.
 */
class Observer : boost::noncopyable {
public:
	virtual ~Observer();

	virtual void observed_changed() = 0;

protected:
	void start_observing(const Observable *);
	void stop_observing(const Observable *);

private:
	std::set<Observable *> observed_;
};

#endif /* end of include guard: OBSERVER_H */

