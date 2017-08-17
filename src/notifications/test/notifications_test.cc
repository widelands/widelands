/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include <memory>
#include <string>

#define BOOST_TEST_MODULE Notifications
#include <boost/test/unit_test.hpp>

#include "base/macros.h"
#include "notifications/notifications.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")

struct SimpleNote {
	CAN_BE_SENT_AS_NOTE(100)

	explicit SimpleNote(const std::string& init_text) : text(init_text) {
	}

	std::string text;
};

BOOST_AUTO_TEST_SUITE(NotificationsTestSuite)

BOOST_AUTO_TEST_CASE(SimpleTest) {
	std::vector<SimpleNote> received1;
	auto subscriber1 = Notifications::subscribe<SimpleNote>(
	   [&received1](const SimpleNote& got) { received1.push_back(got); });

	Notifications::publish(SimpleNote("Hello"));

	std::vector<SimpleNote> received2;
	auto subscriber2 = Notifications::subscribe<SimpleNote>(
	   [&received2](const SimpleNote& got) { received2.push_back(got); });

	Notifications::publish(SimpleNote("World"));

	BOOST_CHECK_EQUAL(received1.size(), 2);
	BOOST_CHECK_EQUAL("Hello", received1[0].text);
	BOOST_CHECK_EQUAL("World", received1[1].text);

	BOOST_CHECK_EQUAL(received2.size(), 1);
	BOOST_CHECK_EQUAL("World", received2[0].text);
}

BOOST_AUTO_TEST_SUITE_END()
