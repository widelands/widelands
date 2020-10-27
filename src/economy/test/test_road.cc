/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include <boost/test/unit_test.hpp>

#ifdef _WIN32
#include "base/log.h"
#endif
#include "economy/flag.h"
#include "economy/road.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

/******************/
/* Helper classes */
/******************/
struct TestingFlag : public Widelands::Flag {
	TestingFlag(Widelands::EditorGameBase&, const Widelands::Coords& c) : Widelands::Flag() {
		set_flag_position(c);
	}
};

/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
struct WlTestFixture {
	WlTestFixture() {
#ifdef _WIN32
		set_logging_dir();
#endif
		g_fs = new LayeredFileSystem();

		// We need to add the datadir so we can read the tribes initializations from Lua.

		// Cap the loops to protect against endless loops with crazy setups
		int attempts = 0;

		// Add disk filesystem root dir
		std::string directory_to_add = g_fs->canonicalize_name(".");
		while (!directory_to_add.empty() && directory_to_add.find('/', 4) != std::string::npos) {
			directory_to_add = g_fs->canonicalize_name(directory_to_add + "/..");
			++attempts;
			if (attempts > 50) break;
		}
		g_fs->add_file_system(&FileSystem::create(directory_to_add));

		// Try to find the base + data dir from current working directory (e.g. build/src/economy/test)
		attempts = 0;
		directory_to_add = g_fs->canonicalize_name(g_fs->get_working_directory());
		while (!directory_to_add.empty() && directory_to_add.find('/', 0) != std::string::npos) {
			directory_to_add = g_fs->canonicalize_name(directory_to_add + "/..");
			const std::string candidate(directory_to_add + "/data");
			if (g_fs->file_exists(candidate)) {
				g_fs->add_file_system(&FileSystem::create(candidate));
				break;
			}
			++attempts;
			if (attempts > 50) break;
		}
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

BOOST_AUTO_TEST_SUITE(Road)

/*
 * Simple tests
 */
BOOST_FIXTURE_TEST_CASE(PassabilityTest, SimpleRoadTestsFixture) {
	BOOST_CHECK_EQUAL(r.get_passable(), true);
}
BOOST_FIXTURE_TEST_CASE(CorrectSizeTest, SimpleRoadTestsFixture) {
	BOOST_CHECK_EQUAL(r.get_size(), static_cast<int32_t>(Widelands::BaseImmovable::SMALL));
}
BOOST_FIXTURE_TEST_CASE(InstantiateEditorGameBase, SimpleRoadTestsFixture) {
	BOOST_TEST_MESSAGE(start->get_position().x
	                   << ',' << start->get_position().y << "   " << end->get_position().x << ','
	                   << end->get_position().y << "   " << path.get_start().x << ','
	                   << path.get_start().y);
}

BOOST_AUTO_TEST_SUITE_END()
