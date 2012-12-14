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

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <set>

#include <boost/utility.hpp>

class Observer;

/*
 * An item that can be observed by an Observer. See comments there as well.
 */
class Observable : boost::noncopyable {
public:
	virtual ~Observable();

	void add_observer(Observer *);
	void remove_observer(Observer *);

protected:
	void notify_observers();

private:
	std::set<Observer *> observers_;
};

#endif /* end of include guard: OBSERVABLE_H */

