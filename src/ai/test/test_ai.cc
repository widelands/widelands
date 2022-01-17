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

#include "ai/ai_help_structs.h"
#include "base/test.h"

TESTSUITE_START(warehouse_distance)

TESTCASE(flag_distance_soft_expiry) {
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	check_equal(fw.get_distance(0, Time(0), &tmp_wh), 1000);
	check_equal(fw.set_distance(1, 2, Time(0), 3), true);
	check_equal(fw.get_distance(1, Time(2), &tmp_wh), 2);  // distance now 2
	check_equal(tmp_wh, 3);

	// setting longer distance below soft_expiry time
	check_equal(fw.set_distance(1, 3, Time(AI::kFlagDistanceExpirationPeriod.get() / 3), 4), false);
	// distance to 3 not updated
	check_equal(fw.get_distance(1, Time(AI::kFlagDistanceExpirationPeriod.get() / 3), &tmp_wh), 2);
	check_equal(tmp_wh, 3);

	// now setting after soft expiry
	check_equal(fw.set_distance(1, 1, Time(AI::kFlagDistanceExpirationPeriod.get() / 3), 6),
	            true);  // distance set to 1
	check_equal(fw.get_distance(1, Time(AI::kFlagDistanceExpirationPeriod.get() / 3), &tmp_wh), 1);
	check_equal(tmp_wh, 6);
}
TESTCASE(flag_distance_below_expiry)
/* Compare with void free_test_function() */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	check_equal(fw.set_distance(1, 2, Time(0), 3), true);

	// setting longer distance after soft but below expiry time
	check_equal(
	   fw.set_distance(1, 3, Time(AI::kFlagDistanceExpirationPeriod.get() * 2 / 3), 5), true);
	check_equal(
	   fw.get_distance(1, Time(AI::kFlagDistanceExpirationPeriod.get() * 2 / 3), &tmp_wh), 3);
	check_equal(tmp_wh, 5);
}

TESTCASE(flag_distance_after_expiry)
/* Compare with void free_test_function() */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	check_equal(fw.set_distance(1, 2, Time(0), 3), true);

	// setting longer distance below expiry time
	check_equal(fw.set_distance(1, 3, Time(2 * AI::kFlagDistanceExpirationPeriod.get()), 5), true);
	check_equal(fw.get_distance(1, Time(3), &tmp_wh), 3);
	check_equal(tmp_wh, 5);
}

TESTCASE(flag_distance_expiration_extension)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	uint32_t tmp_wh;
	check_equal(fw.set_distance(1, 2, Time(0), 3), true);
	check_equal(fw.set_distance(1, 2, Time(0), 3),
	            false);  // cannot reset the same distance in the same time

	// Now we are after expiration time
	check_equal(
	   fw.get_distance(1, Time(AI::kFlagDistanceExpirationPeriod.get() + 3), &tmp_wh), 1000);

	// setting distance 2 time shortly one after another
	check_equal(fw.set_distance(1, 2, Time(AI::kFlagDistanceExpirationPeriod.get() + 3), 5), true);
	check_equal(fw.set_distance(1, 2, Time(AI::kFlagDistanceExpirationPeriod.get() + 10), 5), true);
	// current expiry_time should be 2*AI::kFlagDistanceExpirationPeriod + 10
	check_equal(fw.get_distance(1, Time(2 * AI::kFlagDistanceExpirationPeriod.get()), &tmp_wh), 2);
	check_equal(tmp_wh, 5);
	check_equal(
	   fw.get_distance(1, Time(2 * AI::kFlagDistanceExpirationPeriod.get() + 15), &tmp_wh), 1000);
}

TESTCASE(flag_distance_road_builtexpiration_extension)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	// No road built on fresh flag
	check_equal(fw.is_road_prohibited(1, Time(1)), false);
	// get_distance(const uint32_t flag_coords, uint32_t gametime, uint32_t* nw)

	// setting road we dont know about
	fw.set_road_built(1, Time(0));
	check_equal(fw.is_road_prohibited(1, Time(1)), false);

	// let fw knows about it
	fw.set_distance(1, 2, Time(0), 3);
	fw.set_road_built(1, Time(0));
	check_equal(fw.is_road_prohibited(1, Time(1)), true);
	check_equal(fw.is_road_prohibited(1, Time(59999)), true);
	check_equal(fw.is_road_prohibited(1, Time(60001)), false);

	check_equal(fw.is_road_prohibited(2, Time(60001)), false);
}

TESTCASE(flag_distance_old_removal)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	fw.set_distance(1, 2, Time(0), 3);
	check_equal(fw.count(), 1);
	check_equal(fw.remove_old_flag(
	               Time(AI::kOldFlagRemoveTime.get() + AI::kFlagDistanceExpirationPeriod.get())),
	            false);
	check_equal(fw.count(), 1);
	check_equal(fw.remove_old_flag(
	               Time(AI::kOldFlagRemoveTime.get() + AI::kFlagDistanceExpirationPeriod.get() + 2)),
	            true);
	check_equal(fw.count(), 0);
}

TESTCASE(new_flag_road_not_prohibited)
/* setting the same distance restart the expiry_period */
{
	AI::FlagWarehouseDistances fw;
	// let fw knows about it
	check_equal(fw.count(), 0);
	fw.set_distance(1, 2, Time(0), 3);
	check_equal(fw.count(), 1);
	check_equal(fw.is_road_prohibited(1, Time(1)), false);
}

TESTCASE(flag_candidate_init)
/* setting the same distance restart the expiry_period */
{
	AI::FlagCandidates fc = AI::FlagCandidates(10);
	check_equal(fc.count(), 0);
}

TESTCASE(flag_candidate_winner_score) {
	const uint16_t kCurFlDistToWh = 3;
	const uint16_t kStartFlagToWh = 10;

	const uint16_t kPosRoadDist = 5;
	const uint16_t kCurRoadDistFlToFl = 17;

	const uint32_t kTestedCoords = 11;

	AI::FlagCandidates fc = AI::FlagCandidates(kStartFlagToWh);

	// coord, different economy, distance to wh
	fc.add_flag(kTestedCoords, false, kCurFlDistToWh, 1);
	// setting coords, dist
	check_equal(fc.set_cur_road_distance(kTestedCoords, kCurRoadDistFlToFl), true);
	check_equal(fc.set_cur_road_distance(1, 5), false);  // we cannot set distance to unknown flag
	check_equal(fc.get_winner(), nullptr);               // road not possible
	// set length of possible road
	check_equal(fc.set_road_possible(kTestedCoords, kPosRoadDist), true);
	check_equal(fc.get_winner() != nullptr, true);
	check_equal(fc.get_winner()->start_flag_dist_to_wh, kStartFlagToWh);
	check_equal(fc.get_winner()->cand_flag_distance_to_wh, kCurFlDistToWh);
	check_equal(fc.get_winner()->flag_to_flag_road_distance, kCurRoadDistFlToFl);
	check_equal(fc.get_winner()->possible_road_distance, kPosRoadDist);
	check_equal(fc.get_winner()->coords_hash, kTestedCoords);
	check_equal(fc.get_winner()->different_economy, false);

	check_equal(fc.get_winner()->score(),
	            +(kStartFlagToWh - kCurFlDistToWh) + (kCurRoadDistFlToFl - 2 * kPosRoadDist));
}
TESTCASE(flag_candidates_sorting) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 1);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 1);
	check_equal(fc.set_cur_road_distance(0, 5), true);
	check_equal(fc.set_cur_road_distance(1, 5), true);
	check_equal(fc.set_cur_road_distance(2, 5), true);
	check_equal(fc.set_road_possible(0, 4), true);
	check_equal(fc.set_road_possible(1, 2), true);
	check_equal(fc.set_road_possible(2, 3), true);
	check_equal(fc.get_winner()->coords_hash, 1);  // sorted done automatically
	check_equal(fc.count(), 3);
}

TESTCASE(flag_sort_by_air_distance) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 4);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 2);
	fc.sort_by_air_distance();
	check_equal(fc.flags()[0].air_distance, 1);
}

TESTCASE(flag_has_candidate) {
	AI::FlagCandidates fc = AI::FlagCandidates(10);

	fc.add_flag(0, false, 10, 4);
	fc.add_flag(1, false, 10, 1);
	fc.add_flag(2, false, 10, 2);
	check_equal(fc.has_candidate(1), true);
	check_equal(fc.has_candidate(3), false);
}

TESTSUITE_END()
