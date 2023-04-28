/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <vector>

#include "base/test.h"
#include "notifications/notifications.h"

TEST_EXECUTABLE(notifications)

struct SimpleNote {
	CAN_BE_SENT_AS_NOTE(100)

	explicit SimpleNote(const std::string& init_text) : text(init_text) {
	}

	std::string text;
};

TESTSUITE_START(NotificationsTestSuite)

TESTCASE(SimpleTest) {

	std::vector<SimpleNote> received1;
	auto subscriber1 = Notifications::subscribe<SimpleNote>(
	   [&received1](const SimpleNote& got) { received1.push_back(got); });

	Notifications::publish(SimpleNote("Hello"));

	std::vector<SimpleNote> received2;
	auto subscriber2 = Notifications::subscribe<SimpleNote>(
	   [&received2](const SimpleNote& got) { received2.push_back(got); });

	Notifications::publish(SimpleNote("World"));

	check_equal(received1.size(), 2);
	check_equal("Hello", received1[0].text);
	check_equal("World", received1[1].text);

	check_equal(received2.size(), 1);
	check_equal("World", received2[0].text);
}

TESTSUITE_END()
