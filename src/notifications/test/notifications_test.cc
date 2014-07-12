/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <string>

#define BOOST_TEST_MODULE Notifications
#include <boost/test/unit_test.hpp>

#include "notifications/notifications.h"

struct SimpleNote {
	static constexpr int kUniqueNoteId = 1;

	SimpleNote(const std::string& init_text) : text(init_text) {}

	std::string text;
};

BOOST_AUTO_TEST_SUITE(NotificationsTestSuite)


BOOST_AUTO_TEST_CASE(SimpleTest) {
	std::vector<SimpleNote> received;
	uint32_t id = Notifications::get()->subscribe<SimpleNote>([&received](const SimpleNote& got) {
		received.push_back(got);
	});

	// NOCOM(#sirver): send is not a good name
	Notifications::get()->send(SimpleNote("Hello"));
	Notifications::get()->send(SimpleNote("World"));

	BOOST_CHECK_EQUAL(received.size(), 2);
	BOOST_CHECK_EQUAL("Hello", received[0].text);
	BOOST_CHECK_EQUAL("World", received[1].text);

	Notifications::get()->unsubscribe<SimpleNote>(id);
}

BOOST_AUTO_TEST_SUITE_END()
