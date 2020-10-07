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

#include "ai/ai_help_structs.h"
#include "base/macros.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(warehouse_distance)

BOOST_AUTO_TEST_CASE(flag_distance_soft_expiry) {
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	BOOST_CHECK_EQUAL(fw.get_distance(0, Widelands::Time(0), &tmp_wh), 1000);
	BOOST_CHECK_EQUAL(fw.set_distance(1, 2, Widelands::Time(0), 3), true);
	BOOST_CHECK_EQUAL(fw.get_distance(1, Widelands::Time(2), &tmp_wh), 2);  // distance now 2
	BOOST_CHECK_EQUAL(tmp_wh, 3);

	// setting longer distance below soft_expiry time
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 3, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() / 3), 4),
	   false);
	// distance to 3 not updated
	BOOST_CHECK_EQUAL(
	   fw.get_distance(1, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() / 3), &tmp_wh), 2);
	BOOST_CHECK_EQUAL(tmp_wh, 3);

	// now setting after soft expiry
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 1, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() / 3), 6),
	   true);  // distance set to 1
	BOOST_CHECK_EQUAL(
	   fw.get_distance(1, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() / 3), &tmp_wh), 1);
	BOOST_CHECK_EQUAL(tmp_wh, 6);
}
BOOST_AUTO_TEST_CASE(flag_distance_below_expiry)
/* Compare with void free_test_function() */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	BOOST_CHECK_EQUAL(fw.set_distance(1, 2, Widelands::Time(0), 3), true);

	// setting longer distance after soft but below expiry time
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 3, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() * 2 / 3), 5),
	   true);
	BOOST_CHECK_EQUAL(
	   fw.get_distance(1, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() * 2 / 3), &tmp_wh),
	   3);
	BOOST_CHECK_EQUAL(tmp_wh, 5);
}

BOOST_AUTO_TEST_CASE(flag_distance_after_expiry)
/* Compare with void free_test_function() */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	BOOST_CHECK_EQUAL(fw.set_distance(1, 2, Widelands::Time(0), 3), true);

	// setting longer distance below expiry time
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 3, Widelands::Time(2 * AI::kFlagDistanceExpirationPeriod.get()), 5), true);
	BOOST_CHECK_EQUAL(fw.get_distance(1, Widelands::Time(3), &tmp_wh), 3);
	BOOST_CHECK_EQUAL(tmp_wh, 5);
}

BOOST_AUTO_TEST_CASE(flag_distance_expiration_extension)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	BOOST_CHECK_EQUAL(fw.set_distance(1, 2, Widelands::Time(0), 3), true);
	BOOST_CHECK_EQUAL(fw.set_distance(1, 2, Widelands::Time(0), 3),
	                  false);  // cannot reset the same distance in the same time

	// Now we are after expiration time
	BOOST_CHECK_EQUAL(
	   fw.get_distance(1, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() + 3), &tmp_wh),
	   1000);

	// setting distance 2 time shortly one after another
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 2, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() + 3), 5), true);
	BOOST_CHECK_EQUAL(
	   fw.set_distance(1, 2, Widelands::Time(AI::kFlagDistanceExpirationPeriod.get() + 10), 5),
	   true);
	// current expiry_time should be 2*AI::kFlagDistanceExpirationPeriod + 10
	BOOST_CHECK_EQUAL(
	   fw.get_distance(1, Widelands::Time(2 * AI::kFlagDistanceExpirationPeriod.get()), &tmp_wh), 2);
	BOOST_CHECK_EQUAL(tmp_wh, 5);
	BOOST_CHECK_EQUAL(
	   fw.get_distance(
	      1, Widelands::Time(2 * AI::kFlagDistanceExpirationPeriod.get() + 15), &tmp_wh),
	   1000);
}

BOOST_AUTO_TEST_CASE(flag_distance_road_builtexpiration_extension)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	// No road built on fresh flag
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(1)), false);
	// get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)

	// setting road we dont know about
	fw.set_road_built(1, Widelands::Time(0));
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(1)), false);

	// let fw knows about it
	fw.set_distance(1, 2, Widelands::Time(0), 3);
	fw.set_road_built(1, Widelands::Time(0));
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(1)), true);
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(59999)), true);
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(60001)), false);

	BOOST_CHECK_EQUAL(fw.is_road_prohibited(2, Widelands::Time(60001)), false);
}

BOOST_AUTO_TEST_CASE(flag_distance_old_removal)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	fw.set_distance(1, 2, Widelands::Time(0), 3);
	BOOST_CHECK_EQUAL(fw.count(), 1);
	BOOST_CHECK_EQUAL(fw.remove_old_flag(Widelands::Time(AI::kOldFlagRemoveTime.get() +
	                                                     AI::kFlagDistanceExpirationPeriod.get())),
	                  false);
	BOOST_CHECK_EQUAL(fw.count(), 1);
	BOOST_CHECK_EQUAL(
	   fw.remove_old_flag(Widelands::Time(AI::kOldFlagRemoveTime.get() +
	                                      AI::kFlagDistanceExpirationPeriod.get() + 2)),
	   true);
	BOOST_CHECK_EQUAL(fw.count(), 0);
}

BOOST_AUTO_TEST_CASE(new_flag_road_not_prohibited)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	// let fw knows about it
	BOOST_CHECK_EQUAL(fw.count(), 0);
	fw.set_distance(1, 2, Widelands::Time(0), 3);
	BOOST_CHECK_EQUAL(fw.count(), 1);
	BOOST_CHECK_EQUAL(fw.is_road_prohibited(1, Widelands::Time(1)), false);
}

BOOST_AUTO_TEST_CASE(flag_candidate_init)
/* setting the same distance restart the expiry_period */
{
	AI::FlagCandidates fc = AI::FlagCandidates(10);
	BOOST_CHECK_EQUAL(fc.count(), 0);
}

BOOST_AUTO_TEST_CASE(flag_candidate_winner_score) {
	const uint16_t kCurFlDistToWh = 3;
	const uint16_t kStartFlagToWh = 10;

	const uint16_t kPosRoadDist = 5;
	const uint16_t kCurRoadDistFlToFl = 17;

	const uint32_t kTestedCoords = 11;

	AI::FlagCandidates fc = AI::FlagCandidates(kStartFlagToWh);

	// coord, different economy, distance to wh
	fc.add_flag(kTestedCoords, false, kCurFlDistToWh, 1);
	// setting coords, dist
	BOOST_CHECK_EQUAL(fc.set_cur_road_distance(kTestedCoords, kCurRoadDistFlToFl), true);
	BOOST_CHECK_EQUAL(
	   fc.set_cur_road_distance(1, 5), false);  // we cannot set distance to unknown flag
	BOOST_CHECK(!fc.get_winner());              // road not possible
	// set length of possible road
	BOOST_CHECK_EQUAL(fc.set_road_possible(kTestedCoords, kPosRoadDist), true);
	BOOST_VERIFY(fc.get_winner());
	BOOST_CHECK_EQUAL(fc.get_winner()->start_flag_dist_to_wh, kStartFlagToWh);
	BOOST_CHECK_EQUAL(fc.get_winner()->cand_flag_distance_to_wh, kCurFlDistToWh);
	BOOST_CHECK_EQUAL(fc.get_winner()->flag_to_flag_road_distance, kCurRoadDistFlToFl);
	BOOST_CHECK_EQUAL(fc.get_winner()->possible_road_distance, kPosRoadDist);
	BOOST_CHECK_EQUAL(fc.get_winner()->coords_hash, kTestedCoords);
	BOOST_CHECK_EQUAL(fc.get_winner()->different_economy, false);

	BOOST_CHECK_EQUAL(fc.get_winner()->score(),
	                  +(kStartFlagToWh - kCurFlDistToWh) + (kCurRoadDistFlToFl - 2 * kPosRoadDist));
}
BOOST_AUTO_TEST_CASE(flag_candidates_sorting) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 1);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 1);
	BOOST_CHECK_EQUAL(fc.set_cur_road_distance(0, 5), true);
	BOOST_CHECK_EQUAL(fc.set_cur_road_distance(1, 5), true);
	BOOST_CHECK_EQUAL(fc.set_cur_road_distance(2, 5), true);
	BOOST_CHECK_EQUAL(fc.set_road_possible(0, 4), true);
	BOOST_CHECK_EQUAL(fc.set_road_possible(1, 2), true);
	BOOST_CHECK_EQUAL(fc.set_road_possible(2, 3), true);
	BOOST_CHECK_EQUAL(fc.get_winner()->coords_hash, 1);  // sorted done automatically
	BOOST_CHECK_EQUAL(fc.count(), 3);
}

BOOST_AUTO_TEST_CASE(flag_sort_by_air_distance) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 4);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 2);
	fc.sort_by_air_distance();
	BOOST_CHECK_EQUAL(fc.flags()[0].air_distance, 1);
}

BOOST_AUTO_TEST_CASE(flag_has_candidate) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 4);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 2);
	BOOST_CHECK_EQUAL(fc.has_candidate(1), true);
	BOOST_CHECK_EQUAL(fc.has_candidate(3), false);
}

BOOST_AUTO_TEST_SUITE_END()
