
/*
 * Copyright (C) 2007-2019 by the Widelands Development Team
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

#ifdef _WIN32
#include "base/log.h"
#endif
#include "ai/ai_help_structs.h"


// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")

namespace Widelands { // Needed?
class World;
}  // namespace Widelands

using namespace Widelands;

BOOST_AUTO_TEST_SUITE(ai)

BOOST_AUTO_TEST_CASE(free_flag_distance_soft_expiry )
/* Compare with void free_test_function() */
{
	FlagWarehouseDistances fw;
	uint32_t tmp_wh;
  BOOST_CHECK_EQUAL( fw.get_distance(0,0,&tmp_wh), 1000);
  //set_distance(const uint32_t flag_coords, const uint16_t distance,
 //	uint32_t const gametime, uint32_t const nearest_warehouse)
 BOOST_CHECK_EQUAL( fw.set_distance(1,2,0,3), true);
 //get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)
 BOOST_CHECK_EQUAL( fw.get_distance(1,2,&tmp_wh), 2); // distance now 2
 BOOST_CHECK_EQUAL(tmp_wh,3);

 //setting longer distance below soft_expiry time
 BOOST_CHECK_EQUAL( fw.set_distance(1,3,kFlagDistanceExpirationPeriod/3,4), false);
 //distance to 3 not updated
 BOOST_CHECK_EQUAL( fw.get_distance(1,kFlagDistanceExpirationPeriod/3,&tmp_wh), 2);
 BOOST_CHECK_EQUAL(tmp_wh,3);

 //now setting after soft expiry
 BOOST_CHECK_EQUAL( fw.set_distance(1,1,kFlagDistanceExpirationPeriod/3,6),true); // distance set to 1
 BOOST_CHECK_EQUAL( fw.get_distance(1,kFlagDistanceExpirationPeriod/3,&tmp_wh), 1);
 BOOST_CHECK_EQUAL(tmp_wh,6);

}
BOOST_AUTO_TEST_CASE(free_flag_distance_below_expiry )
/* Compare with void free_test_function() */
{
	FlagWarehouseDistances fw;
	uint32_t tmp_wh;
  //set_distance(const uint32_t flag_coords, const uint16_t distance,
 //	uint32_t const gametime, uint32_t const nearest_warehouse)
 BOOST_CHECK_EQUAL( fw.set_distance(1,2,0,3), true);
 //get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)


 //setting longer distance after soft but below expiry time
 BOOST_CHECK_EQUAL( fw.set_distance(1,3,kFlagDistanceExpirationPeriod*2/3,5), true);
  BOOST_CHECK_EQUAL( fw.get_distance(1,kFlagDistanceExpirationPeriod*2/3,&tmp_wh), 3);
 BOOST_CHECK_EQUAL(tmp_wh,5);


}

BOOST_AUTO_TEST_CASE(free_flag_distance_after_expiry )
/* Compare with void free_test_function() */
{
	FlagWarehouseDistances fw;
	uint32_t tmp_wh;
  //set_distance(const uint32_t flag_coords, const uint16_t distance,
 //	uint32_t const gametime, uint32_t const nearest_warehouse)
 BOOST_CHECK_EQUAL( fw.set_distance(1,2,0,3), true);
 //get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)


 //setting longer distance below expiry time
 BOOST_CHECK_EQUAL( fw.set_distance(1,3,2*kFlagDistanceExpirationPeriod,5), true);
 BOOST_CHECK_EQUAL( fw.get_distance(1,3,&tmp_wh), 3);
 BOOST_CHECK_EQUAL(tmp_wh,5);


}

BOOST_AUTO_TEST_CASE(free_flag_distance_expiration_extension )
/* Compare with void free_test_function() */
{
	FlagWarehouseDistances fw;
	uint32_t tmp_wh;
  //set_distance(const uint32_t flag_coords, const uint16_t distance,
 //	uint32_t const gametime, uint32_t const nearest_warehouse)
 BOOST_CHECK_EQUAL( fw.set_distance(1,2,0,3), true);
 //get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)

// Now we are after expiration time
BOOST_CHECK_EQUAL( fw.get_distance(1,kFlagDistanceExpirationPeriod + 3,&tmp_wh), 1000);

 //setting distance 2 time shortly one after another
 BOOST_CHECK_EQUAL( fw.set_distance(1,2,kFlagDistanceExpirationPeriod + 3,5), true);
  BOOST_CHECK_EQUAL( fw.set_distance(1,2,kFlagDistanceExpirationPeriod + 10,5), true);
  // current expiry_time should be 2*kFlagDistanceExpirationPeriod + 10
 BOOST_CHECK_EQUAL( fw.get_distance(1,2*kFlagDistanceExpirationPeriod,&tmp_wh), 2);
 BOOST_CHECK_EQUAL(tmp_wh,5);
  BOOST_CHECK_EQUAL( fw.get_distance(1,2*kFlagDistanceExpirationPeriod+15,&tmp_wh), 1000);


}

BOOST_AUTO_TEST_SUITE_END()
