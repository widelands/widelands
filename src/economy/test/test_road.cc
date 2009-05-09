/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include <exception>
#include <boost/test/unit_test.hpp>

class PlayerImmovable {
};

#include "economy/road.h"
#include "economy/flag.h"

using namespace Widelands;

/******************/
/* Helper classes */
/******************/

// class TestingEditorGameBase : public Editor_Game_Base {
//
// };
//
/*************************************************************************/
/*                                 TESTS                                 */
/*************************************************************************/
BOOST_AUTO_TEST_SUITE(Road)

BOOST_AUTO_TEST_CASE(instantiate_test) {
        Widelands::Road r;
		  Widelands::Flag f1;
		  Widelands::Flag f2;

        // BOOST_CHECK_EQUAL(r.get_passable(), false);

        // TestingEditorGameBase g;
}

BOOST_AUTO_TEST_SUITE_END()

