/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#include "base/test.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

/******************/
/* Helper classes */
/******************/
struct TestingFlag : public Widelands::Flag {
	TestingFlag(Widelands::EditorGameBase&, const Widelands::Coords& c) {
		set_flag_position(c);
	}
};

/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
struct WlTestFixture {
	WlTestFixture() {
		g_fs = new LayeredFileSystem();
	}
	~WlTestFixture() {
		delete g_fs;
		g_fs = nullptr;
	}
};

struct SimpleRoadTestsFixture : public WlTestFixture {
	SimpleRoadTestsFixture() : g(nullptr), path(Widelands::Coords(5, 5)) {
		g.mutable_map()->set_size(32, 32);
		path.append(g.map(), Widelands::WALK_E);
		path.append(g.map(), Widelands::WALK_E);

		start = new TestingFlag(g, Widelands::Coords(5, 5));
		end = new TestingFlag(g, Widelands::Coords(7, 5));
	}
	~SimpleRoadTestsFixture() {
		delete start;
		delete end;
		// Map is deleted by EditorGameBase
	}

	DISALLOW_COPY_AND_ASSIGN(SimpleRoadTestsFixture);

	Widelands::EditorGameBase g;
	Widelands::Road r;
	Widelands::Path path;
	TestingFlag* start;
	TestingFlag* end;
};

TESTSUITE_START(Road)

/*
 * Simple tests
 */

TESTCASE(passability) {
	SimpleRoadTestsFixture f;
	check_equal(f.r.get_passable(), true);
}
TESTCASE(correct_size) {
	SimpleRoadTestsFixture f;
	check_equal(f.r.get_size(), static_cast<int32_t>(Widelands::BaseImmovable::SMALL));
}

TESTSUITE_END()
