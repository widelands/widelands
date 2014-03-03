/*
 * Copyright (C) 2007-2010 by the Widelands Development Team
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

#include <exception>

#include <boost/test/unit_test.hpp>

#include "economy/flag.h"
#include "economy/road.h"
#include "logic/editor_game_base.h"
#include "logic/instances.h"
#include "logic/player.h"

using namespace Widelands;

/******************/
/* Helper classes */
/******************/
struct TestingFlag : public Flag {
	TestingFlag(Editor_Game_Base &, Coords const c) : Flag() {
		set_flag_position(c);
	}

};
struct TestingMap : public Map {
	TestingMap(int const w, int const h) : Map() {set_size(w, h);}

	virtual void recalc_for_field_area(Area<FCoords>) override {}

};

/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
struct WlTestFixture {
	WlTestFixture() {
	g_fs = new LayeredFileSystem();
	}
	~WlTestFixture() {delete g_fs; g_fs = nullptr;}
};

struct SimpleRoadTestsFixture : public WlTestFixture {
	SimpleRoadTestsFixture() :
		g(nullptr),
		path(Coords(5, 5))
	{
		map = new TestingMap(32, 32);
		g.set_map(map);

		path.append(*map, WALK_E);
		path.append(*map, WALK_E);

		start = new TestingFlag(g, Coords(5, 5));
		end = new TestingFlag(g, Coords(7, 5));
	}
	~SimpleRoadTestsFixture() {
		delete start;
		delete end;
		// Map is deleted by Editor_Game_Base
	}

	TestingMap * map;
	Editor_Game_Base g;
	Road r;
	Path path;
	TestingFlag * start;
	TestingFlag * end;
};

BOOST_AUTO_TEST_SUITE(Road)

/*
 * Simple tests
 */
BOOST_FIXTURE_TEST_CASE(PassabilityTest, SimpleRoadTestsFixture) {
	BOOST_CHECK_EQUAL(r.get_passable(), true);
}
BOOST_FIXTURE_TEST_CASE(CorrectSizeTest, SimpleRoadTestsFixture) {
	BOOST_CHECK_EQUAL(r.get_size(), static_cast<int32_t>(BaseImmovable::SMALL));
}
BOOST_FIXTURE_TEST_CASE(InstantiateEditorGameBase, SimpleRoadTestsFixture) {
	BOOST_MESSAGE
		(start->get_position().x << ',' << start->get_position().y <<
		 "   " << end->get_position().x << ',' << end->get_position().y <<
		 "   " << path.get_start().x << ',' << path.get_start().y);
}

BOOST_AUTO_TEST_SUITE_END()

