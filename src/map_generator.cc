/*
 * Copyright (C) 2002-2004, 2006-2010, 2013 by the Widelands Development Team
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

#include "map_generator.h"

#include "editor/tools/editor_increase_resources_tool.h"
#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/findnode.h"
#include "logic/map.h"

#define AVG_ELEVATION   (0x80000000)
#define MAX_ELEVATION   (0xffffffff)
#define MAP_ID_DIGITS   24
#define ISLAND_BORDER   10
#define MAX_ELEVATION_HALF (0x80000000)

namespace Widelands
{

MapGenerator::MapGenerator
	(Map & map, const UniqueRandomMapInfo & mapInfo,
	 Editor_Game_Base & egbase)
	: m_map(map), m_mapInfo(mapInfo), m_egbase(egbase)
{

}

void MapGenerator::generate_bobs
	(std::unique_ptr<uint32_t[]> const * random_bobs,
	 Coords const fc,
	 RNG  &       rng,
	 MapGenAreaInfo::MapGenTerrainType const terrType)
{
	//  Figure out which bob area is due here...

	MapGenInfo & mapGenInfo = m_map.world().getMapGenInfo();

	size_t num = mapGenInfo.getNumBobAreas();
	size_t found = num;
	uint32_t sum_weight = mapGenInfo.getSumBobAreaWeight();
	uint32_t max_val = 0;
	for (size_t ix = 0; ix < num; ++ix) {
		uint32_t val = random_bobs[ix][fc.x + m_mapInfo.w * fc.y];
		val = (val / sum_weight) * mapGenInfo.getBobArea(ix).getWeight();
		if (val >= max_val) {
			found = ix;
			max_val = val;
		}
	}
	if (found >= num)
		return;

	// Figure out if we really need to set a bob here...

	const MapGenBobArea & bobArea = mapGenInfo.getBobArea(found);

	const MapGenBobKind * bobKind = bobArea.getBobKind(terrType);

	if (not bobKind) //  no bobs defined here...
		return;

	uint32_t immovDens = bobArea.getImmovableDensity();
	uint32_t movDens   = bobArea.getMoveableDensity();

	immovDens *= max_val / 100;
	movDens   *= max_val / 100;

	immovDens = immovDens >= MAX_ELEVATION_HALF ? MAX_ELEVATION : immovDens * 2;
	movDens   = movDens   >= MAX_ELEVATION_HALF ? MAX_ELEVATION : movDens   * 2;

	uint32_t val = rng.rand();
	bool set_immovable = (val <= immovDens);
	val = rng.rand();
	bool set_moveable  = (val <= movDens);


	// Set bob according to bob area

	if (set_immovable and (num = bobKind->getNumImmovableBobs()))
		m_egbase.create_immovable
			(fc,
			 bobKind->getImmovableBob
			 	(static_cast<size_t>(rng.rand() / (MAX_ELEVATION / num))),
			 nullptr);

	if (set_moveable and (num = bobKind->getNumMoveableBobs()))
		m_egbase.create_bob
			(fc,
			 m_map.world().get_bob
			 	(bobKind->getMoveableBob
			 	 	(static_cast<size_t>(rng.rand() / (MAX_ELEVATION / num)))
			 	 .c_str()),
			 nullptr);
}

#define set_resource_helper(rnd1, res)                                                             \
	{                                                                                               \
		Resource_Index const res_idx = terr.get_valid_resource(res);                                 \
		uint32_t const max_amount = m_map.world().get_resource(res_idx)->get_max_amount();           \
		uint8_t res_val = static_cast<uint8_t>(rnd1 / (MAX_ELEVATION / max_amount));                 \
		res_val *= static_cast<uint8_t>(m_mapInfo.resource_amount) + 1;                              \
		res_val /= 3;                                                                                \
		if (Editor_Change_Resource_Tool_Callback(fc, m_map, res_idx)) {                              \
			fc.field->set_resources(res_idx, res_val);                                                \
			fc.field->set_starting_res_amount(res_val);                                               \
		}                                                                                            \
	}

void MapGenerator::generate_resources
	(uint32_t            const * const random1,
	 uint32_t            const * const random2,
	 uint32_t            const * const random3,
	 uint32_t            const * const random4,
	 FCoords                     const fc)
{
	// We'll take the "D" terrain at first...
	// TODO: Check how the editor handles this...

	Terrain_Index const tix = fc.field->get_terrains().d;
	const Terrain_Descr & terr = m_map.world().get_ter(tix);
	switch (terr.get_num_valid_resources()) {
	case 1: {
		uint32_t const rnd1 = random1[fc.x + m_mapInfo.w * fc.y];
		set_resource_helper(rnd1, 0);
		break;
	}
	case 2: {
		uint32_t const rnd1 = random1[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd2 = random2[fc.x + m_mapInfo.w * fc.y];
		if (rnd1 > rnd2) {
			set_resource_helper(rnd1, 0)
		} else
			set_resource_helper(rnd2, 1);
		break;
	}
	case 3: {
		uint32_t const rnd1 = random1[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd2 = random2[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd3 = random3[fc.x + m_mapInfo.w * fc.y];
		if        (rnd1 > rnd2 && rnd1 > rnd3) {
			set_resource_helper(rnd1, 0);
		} else if (rnd2 > rnd1 && rnd2 > rnd3) {
			set_resource_helper(rnd2, 1);
		} else
			set_resource_helper(rnd3, 2);
		break;
	}
	case 4: {
		uint32_t const rnd1 = random1[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd2 = random2[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd3 = random3[fc.x + m_mapInfo.w * fc.y];
		uint32_t const rnd4 = random4[fc.x + m_mapInfo.w * fc.y];
		if        (rnd1 > rnd2 && rnd1 > rnd3 && rnd1 > rnd4) {
			set_resource_helper(rnd1, 0);
		} else if (rnd2 > rnd1 && rnd2 > rnd3 && rnd2 > rnd4) {
			set_resource_helper(rnd2, 1);
		} else if (rnd3 > rnd1 && rnd3 > rnd2 && rnd3 > rnd4) {
			set_resource_helper(rnd3, 2);
		} else
			set_resource_helper(rnd4, 3);
		break;
	}
	default:
		break; // currently mountains have the maximum of allowed resources, which is 4
	}
}

/// Translates a random value into a map node height. This method is used
/// within the random map generation methods.
///
/// \param elevation Random value.
/// \param mapGenInfo Map generator information used to translate random values
/// to height information (world specific info).
/// \param c position within map
/// \param mapInfo Information about the random map currently begin created
/// (map specific info).
///
/// \returns A map height value corresponding to elevation.
uint8_t MapGenerator::make_node_elevation
	(double                      const elevation,
	 Coords                      const c)
{
	MapGenInfo & mapGenInfo = m_map.world().getMapGenInfo();

	int32_t const water_h  = mapGenInfo.getWaterShallowHeight();
	int32_t const mount_h  = mapGenInfo.getMountainFootHeight();
	int32_t const summit_h = mapGenInfo.getSummitHeight      ();

	double const water_fac = m_mapInfo.waterRatio;
	double const land_fac  = m_mapInfo.landRatio;

	uint8_t res_h =
		elevation < water_fac            ? water_h :
		elevation < water_fac + land_fac ?
			water_h + 1 +
			((elevation - water_fac) / land_fac) * (mount_h - water_h)
			:
			mount_h +
			((elevation - water_fac - land_fac) / (1 - water_fac - land_fac))
			*
			(summit_h - mount_h);

	//  Handle Map Border in island mode
	if (m_mapInfo.islandMode) {
		int32_t const border_dist =
			std::min
				(std::min<X_Coordinate>(c.x, m_mapInfo.w - c.x),
				 std::min<Y_Coordinate>(c.y, m_mapInfo.h - c.y));
		if (border_dist <= ISLAND_BORDER) {
			res_h =
				static_cast<uint8_t>
					(static_cast<double>(res_h) * border_dist /
					 static_cast<double>(ISLAND_BORDER));
			if (res_h < water_h)
				res_h = water_h;
		}
	}

	return res_h;
}

/**
===============
Generate a "continuous" array of "reasonable" random values.
The array generated is in fact organized in a two-dimensional
way. "Reasonable" means that the values are not purely random.
Neighboring values (in a two-dimensional way) are fitting
together so that such an array can be used to directly generate
height information for mountains, wasteland, resources etc.
"Continuous" means that also value of the left border fit to
the right border values and values of the top border fit to the
bottom border values. This means we have some kind of "endlessly"
repeating set of random values.
What is more, the different heights are weighed so that the
random distribution of all random values in the array is linear.
The minimum valu will be 0, the maximum value will be MAX_ELEVATION,
the average will be AVG_ELEVATION.

w, h: are width and height of the two-dimensional array
      produced. Thus, the array has w * h entries. To access a certain
      "coordinate" in the array, use array[x + w * y] to retrieve the entry.

rng:  is the random number generator to be used.
      This will mostly be the current rng of the random map currently being
      created.
===============
*/
uint32_t * MapGenerator::generate_random_value_map
	(uint32_t const w, uint32_t const h, RNG & rng)
{
	uint32_t const numFields = h * w; //  Size of the resulting array

	uint32_t * const values = new uint32_t[numFields]; //  Array to be filled

	try {
		//  We will do some initing here...

		for (uint32_t ix = 0; ix < numFields; ++ix)
			values[ix] = AVG_ELEVATION;

		//  This will be the first starting random values...

		for (uint32_t x = 0; x < w; x += 16)
			for (uint32_t y = 0; y < h; y += 16) {
				values[x + y * w] = rng.rand();
				if (x % 32 or y % 32) {
					values[x + y * w] += AVG_ELEVATION;
					values[x + y * w] /= 2;
				}
			}

		//  randomize the values

		uint32_t step_x  = std::min(16U, w), step_y  = std::min(16U, h);
		uint32_t max = AVG_ELEVATION, min = AVG_ELEVATION;
		double    ele_fac  = 0.15;

		bool end = false;

		while (!end) {
			for (uint32_t x = 0; x < w; x += step_x) {
				for (uint32_t y = 0; y < h; y += step_y) {
					//  Calculate coordinates of left and bottom left neighbours of
					//  the current node.

					uint32_t right_x = x + step_x;
					uint32_t lower_y = y + step_y;
					if (right_x >= w)
						right_x -= w;
					if (lower_y >= h)
						lower_y -= h;

					//  Get the current values of my neighbor nodes and of my node.

					uint32_t const x_0_y_0 = values[x       + w *       y];
					uint32_t const x_1_y_0 = values[right_x + w *       y];
					uint32_t const x_0_y_1 = values[x       + w * lower_y];
					uint32_t const x_1_y_1 = values[right_x + w * lower_y];

					//  calculate the in-between values

					uint32_t x_new       =
						x_0_y_0 / 2 + x_1_y_0 / 2 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					uint32_t y_new       =
						x_0_y_0 / 2 + x_0_y_1 / 2 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					uint32_t xy_new      =
						x_0_y_0 / 4 + x_1_y_1 / 4 + x_1_y_0 / 4 + x_0_y_1 / 4 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					values[x + step_x / 2 + w * (y)]              =  x_new;
					values[x + step_x / 2 + w * (y + step_y / 2)] = xy_new;
					values[x              + w * (y + step_y / 2)] =  y_new;

					//  see if we have got a new min or max value

					if  (x_new > max)
						max =  x_new;
					if  (y_new > max)
						max =  y_new;
					if (xy_new > max)
						max = xy_new;

					if  (x_new < min)
						min =  x_new;
					if  (y_new < min)
						min =  y_new;
					if (xy_new < min)
						min = xy_new;
				}
			}

			//  preparations for the next iteration
			if (step_y == 2 && step_x == 2)
				end = true;
			step_x /= 2;
			step_y /= 2;
			if (step_x <= 1)
				step_x = 2;
			if (step_y <= 1)
				step_y = 2;
			ele_fac *= 0.9;
		}

		//  make a histogram of the heights

		uint32_t histo[1024];

		for (uint32_t x = 0; x < 1024; ++x)
			histo[x] = 0;

		for (uint32_t x = 0; x < w; ++x)
			for (uint32_t y = 0; y < h; ++y) {
				values[x + y * w] =
					((static_cast<double>(values[x + y * w] - min))
					 /
					 static_cast<double>(max - min))
					*
					MAX_ELEVATION;
				++histo[values[x + y * w] >> 22];
			}

		//  sort the histo out

		double minVals[1024];

		double currVal = 0.0;

		for (uint32_t x = 0; x < 1024; ++x) {
			minVals[x] = currVal;
			currVal +=
				static_cast<double>(histo[x]) / static_cast<double>(numFields);
		}

		//  Adjust the heights so that all height values are equal of density.
		//  This is done to have reliable water/land ratio later on.
		for (uint32_t x = 0; x < w; ++x)
			for (uint32_t y = 0; y < h; ++y)
				values[x + y * w] =
					minVals[values[x + y * w] >> 22]
					*
					static_cast<double>(MAX_ELEVATION);
		return values;
	} catch (...) {
		delete[] values;
		throw;
	}

	return nullptr; // Should not be reached
}


/**
===============
Figures out terrain info for a field in a random map.

mapGenInfo:  Map generator information used to translate
             Random values to height information (world-
             specific info)
x, y:        first coordinate of the current triangle
x1, y1:      second coordinate of the current triangle
x2, y2:      third coordinate of the current triangle
random2:     Random array for generating different
             terrain types on land
random3:     Random array for generating different
             terrain types on land
random4:     Random array for wasteland generation
h1, h2, h3:  Map height information for the three triangle coords
mapInfo:     Information about the random map currently
             begin created (map specific info)
rng:         is the random number generator to be used.
             This will mostly be the current rng of the random map
             currently being created.
terrType:    Returns the terrain-Type fpor this triangle
===============
*/
Terrain_Index MapGenerator::figure_out_terrain
	(uint32_t                  * const random2,
	 uint32_t                  * const random3,
	 uint32_t                  * const random4,
	 Coords const c0, Coords const c1, Coords const c2,
	 uint32_t const h1, uint32_t const h2, uint32_t const h3,
	 RNG                       &       rng,
	 MapGenAreaInfo::MapGenTerrainType & terrType)
{
	MapGenInfo & mapGenInfo = m_map.world().getMapGenInfo();

	uint32_t       numLandAreas      =
		mapGenInfo.getNumAreas(MapGenAreaInfo::atLand);
	uint32_t const numWasteLandAreas =
		mapGenInfo.getNumAreas(MapGenAreaInfo::atWasteland);

	bool isDesert  = false;
	bool isDesertOuter = false;
	uint32_t landAreaIndex = 0;

	uint32_t rand2 =
		random2[c0.x + m_mapInfo.w * c0.y] / 3 +
		random2[c1.x + m_mapInfo.w * c1.y] / 3 +
		random2[c2.x + m_mapInfo.w * c2.y] / 3;
	uint32_t rand3 =
		random3[c0.x + m_mapInfo.w * c0.y] / 3 +
		random3[c1.x + m_mapInfo.w * c1.y] / 3 +
		random3[c2.x + m_mapInfo.w * c2.y] / 3;
	uint32_t rand4 =
		random4[c0.x + m_mapInfo.w * c0.y] / 3 +
		random4[c1.x + m_mapInfo.w * c1.y] / 3 +
		random4[c2.x + m_mapInfo.w * c2.y] / 3;

	//  At first we figure out if it is wasteland or not.

	if        (numWasteLandAreas == 0) {
	} else if (numWasteLandAreas == 1) {
		if (rand4 < (AVG_ELEVATION * m_mapInfo.wastelandRatio)) {
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4 > (AVG_ELEVATION * m_mapInfo.wastelandRatio / 4) * 3;
			landAreaIndex = 0;
		}
	} else {
		if (rand4<(AVG_ELEVATION * m_mapInfo.wastelandRatio * 0.5)) {
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4 > (AVG_ELEVATION * m_mapInfo.wastelandRatio * 0.5 / 4) * 3;
			landAreaIndex = 0;
		}
		else if
			(rand4
			 >
			 (MAX_ELEVATION - AVG_ELEVATION * m_mapInfo.wastelandRatio * 0.5))
		{
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4
				<
				1 - AVG_ELEVATION * m_mapInfo.wastelandRatio * 0.5 / 4 * 3;
			landAreaIndex = 1;
		}
	}

	MapGenAreaInfo::MapGenAreaType    atp = MapGenAreaInfo::atLand;
	MapGenAreaInfo::MapGenTerrainType ttp = MapGenAreaInfo::ttLandLand;

	if (!isDesert) { //  see what kind of land it is

		if      (numLandAreas == 1)
			landAreaIndex = 0;
		else if (numLandAreas == 2) {
			uint32_t const weight1 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getWeight();
			uint32_t const weight2 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 1).getWeight();
			uint32_t const sum     = mapGenInfo.getSumLandWeight();
			if
				(weight1 * (random2[c0.x + m_mapInfo.w * c0.y] / sum)
				 >=
				 weight2 * (AVG_ELEVATION / sum))
				landAreaIndex = 0;
			else
				landAreaIndex = 1;
		} else {
			uint32_t const weight1 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getWeight();
			uint32_t const weight2 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 1).getWeight();
			uint32_t const weight3 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 2).getWeight();
			uint32_t const sum     = mapGenInfo.getSumLandWeight();
			uint32_t const randomX = (rand2 + rand3) / 2;
			if
				(weight1 * (rand2 / sum) > weight2 * (rand3   / sum) &&
				 weight1 * (rand2 / sum) > weight3 * (randomX / sum))
				landAreaIndex = 0;
			else if
				(weight2 * (rand3 / sum) > weight1 * (rand2   / sum) &&
				 weight2 * (rand3 / sum) > weight3 * (randomX / sum))
				landAreaIndex = 1;
			else
				landAreaIndex = 2;
		}

		atp = MapGenAreaInfo::atLand;
		ttp = MapGenAreaInfo::ttLandLand;
	} else {
		atp = MapGenAreaInfo::atWasteland;
		ttp = MapGenAreaInfo::ttWastelandInner;
	}

	//  see whether it is water

	uint32_t const coast_h   = mapGenInfo.getLandCoastHeight();
	if (h1 <= coast_h && h2 <= coast_h && h3 <= coast_h) { //  water or coast...
		atp = MapGenAreaInfo::atLand;
		ttp = MapGenAreaInfo::ttLandCoast;

		uint32_t const ocean_h   = mapGenInfo.getWaterOceanHeight();
		uint32_t const shelf_h   = mapGenInfo.getWaterShelfHeight();
		uint32_t const shallow_h = mapGenInfo.getWaterShallowHeight();

		//  TODO: The heights can not be lower than water-Shallow --
		//  TODO: there will never be an ocean yet

		if        (h1 <= ocean_h   && h2 <= ocean_h   && h3 <= ocean_h)   {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterOcean;
		} else if (h1 <= shelf_h   && h2 <= shelf_h   && h3 <= shelf_h)   {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterShelf;
		} else if (h1 <= shallow_h && h2 <= shallow_h && h3 <= shallow_h) {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterShallow;
		}
	} else { //  it is not water
		uint32_t const upper_h = mapGenInfo.getLandUpperHeight   ();
		uint32_t const foot_h  = mapGenInfo.getMountainFootHeight();
		uint32_t const mount_h = mapGenInfo.getMountainHeight    ();
		uint32_t const snow_h  = mapGenInfo.getSnowHeight        ();
		if        (h1 >= snow_h  && h2 >= snow_h  && h3 >= snow_h)  {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsSnow;
		} else if (h1 >= mount_h && h2 >= mount_h && h3 >= mount_h) {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsMountain;
		} else if (h1 >= foot_h  && h2 >= foot_h  && h3 >= foot_h)  {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsFoot;
		} else if (h1 >= upper_h && h2 >= upper_h && h3 >= upper_h) {
			atp = MapGenAreaInfo::atLand;
			ttp = MapGenAreaInfo::ttLandUpper;
		}
	}

	//  Aftermath for land/Wasteland.

	uint32_t usedLandIndex = landAreaIndex;
	if (atp != MapGenAreaInfo::atLand && atp != MapGenAreaInfo::atWasteland)
		usedLandIndex = 0;
	else if (isDesert) {
		atp = MapGenAreaInfo::atWasteland;
		ttp =
			ttp == MapGenAreaInfo::ttLandCoast || isDesertOuter ?
			MapGenAreaInfo::ttWastelandOuter : MapGenAreaInfo::ttWastelandInner;
	}

	// Return terrain type
	terrType = ttp;

	//  Figure out which terrain to use at this point in the map...
	return
		mapGenInfo.getArea(atp, usedLandIndex).getTerrain
			(ttp,
			 rng.rand()
			 %
			 mapGenInfo.getArea(atp, usedLandIndex).getNumTerrains(ttp));

}


void MapGenerator::create_random_map()
{
	//  Init random number generator with map number

	//  We will use our own random number generator here so we do not influence
	//  someone else...
	RNG rng;

	rng.seed(m_mapInfo.mapNumber);

	//  get world generator info
	MapGenInfo & mapGenInfo = m_map.world().getMapGenInfo();

	//  Create a "raw" random elevation matrix.
	//  We will transform this into reasonable elevations and terrains later on.

	std::unique_ptr<uint32_t[]> elevations
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));

	//  for land stuff
	std::unique_ptr<uint32_t[]> random2
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));
	std::unique_ptr<uint32_t[]> random3
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));

	//  for desert/land
	std::unique_ptr<uint32_t[]> random4
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));

	// for resources
	std::unique_ptr<uint32_t[]> random_rsrc_1
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));
	std::unique_ptr<uint32_t[]> random_rsrc_2
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));
	std::unique_ptr<uint32_t[]> random_rsrc_3
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));
	std::unique_ptr<uint32_t[]> random_rsrc_4
		(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));

	// for bobs
	std::unique_ptr<std::unique_ptr<uint32_t[]> []> random_bobs
		(new std::unique_ptr<uint32_t[]> [mapGenInfo.getNumBobAreas()]);

	for (size_t ix = 0; ix < mapGenInfo.getNumBobAreas(); ++ix)
		random_bobs[ix].reset
			(generate_random_value_map(m_mapInfo.w, m_mapInfo.h, rng));

	//  Now we have generated a lot of random data!!
	//  Lets use it !!!
	iterate_Map_FCoords(m_map, m_mapInfo, fc)
		fc.field->set_height
			(make_node_elevation
				(static_cast<double>(elevations[fc.x + m_mapInfo.w * fc.y])
				 /
				 static_cast<double>(MAX_ELEVATION),
				 fc));

	//  Now lets set the terrain right according to the heights.

	iterate_Map_FCoords(m_map, m_mapInfo, fc) {
		//  Calculate coordinates of left and bottom left neighbours of the
		//  current node.

		//  ... Treat "even" and "uneven" row numbers differently
		uint32_t const x_dec = fc.y % 2 == 0;

		uint32_t right_x       = fc.x + 1;
		uint32_t lower_y       = fc.y + 1;
		uint32_t lower_x       = fc.x - x_dec;
		uint32_t lower_right_x = fc.x - x_dec + 1;

		if       (lower_x >  m_mapInfo.w)       lower_x += m_mapInfo.w;
		if       (right_x >= m_mapInfo.w)       right_x -= m_mapInfo.w;
		if       (lower_x >= m_mapInfo.w)       lower_x -= m_mapInfo.w;
		if (lower_right_x >= m_mapInfo.w) lower_right_x -= m_mapInfo.w;
		if       (lower_y >= m_mapInfo.h)       lower_y -= m_mapInfo.h;

		//  get the heights of my neighbour nodes and of my current node

		uint8_t height_x0_y0 =
			fc.field                            ->get_height();
		uint8_t height_x1_y0 =
			m_map[Coords(right_x,          fc.y)].get_height();
		uint8_t height_x0_y1 =
			m_map[Coords(lower_x,       lower_y)].get_height();
		uint8_t height_x1_y1 =
			m_map[Coords(lower_right_x, lower_y)].get_height();

		MapGenAreaInfo::MapGenTerrainType terrType;

		fc.field->set_terrain_d
			(figure_out_terrain
				(random2.get(), random3.get(), random4.get(),
				 fc, Coords(lower_x, lower_y), Coords(lower_right_x, lower_y),
				 height_x0_y0, height_x0_y1, height_x1_y1,
				 rng, terrType));

		fc.field->set_terrain_r
			(figure_out_terrain
				(random2.get(), random3.get(), random4.get(),
				 fc, Coords(right_x, fc.y), Coords(lower_right_x, lower_y),
				 height_x0_y0, height_x1_y0, height_x1_y1,
				 rng, terrType));

		//  set resources for this field
		generate_resources
			(random_rsrc_1.get(), random_rsrc_2.get(),
			 random_rsrc_3.get(), random_rsrc_4.get(),
			 fc);

		// set bobs and immovables for this field
		generate_bobs(random_bobs.get(), fc, rng, terrType);
	}

	//  Aftermaths...
	m_map.recalc_whole_map();

	// Care about players and place their start positions
	const std::string tribe = m_map.get_scenario_player_tribe(1);
	const std::string ai    = m_map.get_scenario_player_ai(1);
	m_map.set_nrplayers(m_mapInfo.numPlayers);
	FindNodeSize functor(FindNodeSize::sizeBig);
	Coords playerstart;

	// Build a basic structure how player start positions are placed
	uint8_t line[3];
	uint8_t rows = 1, lines = 1;
	if (m_mapInfo.numPlayers > 1) {
		++lines;
		if (m_mapInfo.numPlayers > 2) {
			++rows;
			if (m_mapInfo.numPlayers > 4) {
				++lines;
				if (m_mapInfo.numPlayers > 6) {
					++rows;
				}
			}
		}
	}
	line[0] = line[1] = line[2] = rows;
	if (rows * lines > m_mapInfo.numPlayers) {
		--line[1];
		if (rows * lines - 1 > m_mapInfo.numPlayers)
			--line[2];
	}

	// Random placement of starting positions
	assert(m_mapInfo.numPlayers);
	std::vector<Player_Number> pn(m_mapInfo.numPlayers);
	for (Player_Number n = 1; n <= m_mapInfo.numPlayers; ++n) {
		bool okay = false;
		// This is a kinda dump algorithm -> we generate a random number and increase it until it fits.
		// However it's working and simple ;) - if you've got a better idea, feel free to fix it.
		Player_Number x = rng.rand() % m_mapInfo.numPlayers;
		while (!okay) {
			okay = true;
			++x; // Player_Number begins at 1 not at 0
			for (Player_Number p = 1; p < n; ++p) {
				if (pn[p - 1] == x) {
					okay = false;
					x = x % m_mapInfo.numPlayers;
					break;
				}
			}
		}
		pn[n - 1] = x;
	}

	for (Player_Number n = 1; n <= m_mapInfo.numPlayers; ++n) {
		// Set scenario information - needed even if it's not a scenario
		m_map.set_scenario_player_name(n, "Random Player");
		m_map.set_scenario_player_tribe(n, tribe);
		m_map.set_scenario_player_ai(n, ai);
		m_map.set_scenario_player_closeable(n, false);

		// Calculate wished coords for player starting position
		if (line[0] + 1 > pn[n - 1]) {
			// X-Coordinates
			playerstart.x  = m_mapInfo.w * (line[0] * line[0] + 1 - pn[n - 1] * pn[n - 1]);
			playerstart.x /= line[0] * line[0] + 1;
			// Y-Coordinates
			if (lines == 1)
				playerstart.y = m_mapInfo.h / 2;
			else
				playerstart.y = m_mapInfo.h / 7 + ISLAND_BORDER;
		} else if (line[0] + line[1] + 1 > pn[n - 1]) {
			// X-Coordinates
			uint8_t pos = pn[n - 1] - line[0];
			playerstart.x  = m_mapInfo.w;
			playerstart.x *= line[1] * line[1] + 1 - pos * pos;
			playerstart.x /= line[1] * line[1] + 1;
			// Y-Coordinates
			if (lines == 3)
				playerstart.y = m_mapInfo.h / 2;
			else
				playerstart.y = m_mapInfo.h - m_mapInfo.h / 7 - ISLAND_BORDER;
		} else {
			// X-Coordinates
			uint8_t pos = pn[n - 1] - line[0] - line[1];
			playerstart.x  = m_mapInfo.w;
			playerstart.x *= line[2] * line[2] + 1 - pos * pos;
			playerstart.x /= line[2] * line[2] + 1;
			// Y-Coordinates
			playerstart.y = m_mapInfo.h - m_mapInfo.h / 7 - ISLAND_BORDER;
		}

		// Now try to find a place as near as possible to the wished
		// starting position
		std::vector<Coords> coords;
		m_map.find_fields
			(Area<FCoords>(m_map.get_fcoords(playerstart), 20),
			 &coords, functor);

		// Take the nearest ones
		uint32_t min_distance = -1;
		Coords coords2;
		for (uint16_t i = 0; i < coords.size(); ++i) {
			uint32_t test = m_map.calc_distance(coords[i], playerstart);
			if (test < min_distance) {
				min_distance = test;
				coords2 = coords[i];
			}
		}

		if (coords.empty()) {
			// TODO inform players via popup
			log("WARNING: Could not find a suitable place for player %u\n", n);
			// Let's hope that one is at least on dry ground.
			coords2 = playerstart;
		}

		// Finally set the found starting position
		m_map.set_starting_pos(n, coords2);
	}
}

/**
===============
Converts a character out of a mapId-String into an integer value.
Valid characters are 'a'-'z' (or 'A'-'Z') and '2'-'9'. 'i' and 'o'
(or 'I' and 'O') are not valid.
The character is treated case-insensitive.

num:          Number to convert
Return value: The resulting number (0-31) or -1 if the character
              was no legal character.
===============
*/

int  UniqueRandomMapInfo::mapIdCharToNumber(char ch)
{
	if ((ch == '0') || (ch == 'o') || (ch == 'O'))
		return 22;
	else if
		((ch == '1') || (ch == 'l') || (ch == 'L') ||
		 (ch == 'I') || (ch == 'i') || (ch == 'J') || (ch == 'j'))
		return 23;
	else if (ch >= 'A' && ch < 'O') {
		char res = ch - 'A';
		if (ch > 'I')
			--res;
		if (ch > 'J')
			--res;
		if (ch > 'L')
			--res;
		if (ch > 'O')
			--res;
		return res;
	}
	else if (ch >= 'a' && ch <= 'z') {
		char res = ch - 'a';
		if (ch > 'i')
			--res;
		if (ch > 'j')
			--res;
		if (ch > 'l')
			--res;
		if (ch > 'o')
			--res;
		return res;
	}
	else if (ch >= '2' && ch <= '9')
		return 24 + ch - '2';
	return -1;
}

/**
===============
Converts an integer number (0-31) to a characted usable in
a map id string.

num:          Number to convert
Return value: The converted value as a character
===============
*/
char UniqueRandomMapInfo::mapIdNumberToChar(int32_t const num)
{
	if (num == 22)
		return '0';
	else if (num == 23)
		return '1';
	else if ((0 <= num) && (num < 22)) {
		char result =  num + 'a';
		if (result >= 'i')
			++result;
		if (result >= 'j')
			++result;
		if (result >= 'l')
			++result;
		if (result >= 'o')
			++result;
		return result;
	} else if ((24 <= num) && (num < 32))
		return (num - 24) + '2';
	else
		return '?';
}

/**
===============
Fills a UniqueRandomMapInfo structure from a given Map-id-string.

mapIdString:  Map-Id-String
mapInfo_out:  UniqueRandomMapInfo-Structure to be filled
Return value: true if the map-id-string was valid, false otherwise
===============
*/

bool UniqueRandomMapInfo::setFromIdString
	(UniqueRandomMapInfo & mapInfo_out, const std::string & mapIdString,
	 const std::vector<std::string> & worlds)
{
	//  check string

	if (mapIdString.length() != MAP_ID_DIGITS + MAP_ID_DIGITS / 4 - 1)
		return false;

	for (uint32_t ix = 4; ix < MAP_ID_DIGITS; ix += 5)
		if (mapIdString[ix] != '-')
			return false;

	//  convert digits to values

	int32_t nums[MAP_ID_DIGITS];

	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix) {
		int const num = mapIdCharToNumber(mapIdString[ix + (ix / 4)]);
		if (num < 0)
			return false;
		nums[ix] = num;
	}

	//  get xxor start value

	int32_t xorr = nums[MAP_ID_DIGITS - 1];

	for (int32_t ix = MAP_ID_DIGITS - 1; ix >= 0; --ix) {
		nums[ix] = nums[ix] ^ xorr;
		xorr -= 7;
		xorr -= ix;
		if (xorr < 0)
			xorr &= 0x0000001f;
	}

	//  check if xxor was right
	if (nums[MAP_ID_DIGITS - 1])
		return false;

	//  check if version number is 1
	if (nums[MAP_ID_DIGITS - 2] != 1)
		return false;

	//  check if csm is right
	if (nums[MAP_ID_DIGITS - 3] != 0x15)
		return false;


	//  convert map number
	mapInfo_out.mapNumber =
		(nums[0])       |
		(nums[1] <<  5) |
		(nums[2] << 10) |
		(nums[3] << 15) |
		(nums[4] << 20) |
		(nums[5] << 25) |
		((nums[6] & 3) << 30);

	// Convert amount of resources
	mapInfo_out.resource_amount =
		static_cast<Widelands::UniqueRandomMapInfo::Resource_Amount>
			((nums[6] & 0xc) >> 2);

	if
		(mapInfo_out.resource_amount >
		 Widelands::UniqueRandomMapInfo::raHigh)
			return false;

	//  Convert map size
	mapInfo_out.w = nums[7] * 16 + 64;
	mapInfo_out.h = nums[8] * 16 + 64;

	//  Convert water percent
	mapInfo_out.waterRatio = static_cast<double>(nums[9]) / 20.0;
	//  Convert land percent
	mapInfo_out.landRatio  = static_cast<double>(nums[10]) / 20.0;
	//  Convert wasteland percent
	mapInfo_out.wastelandRatio  = static_cast<double>(nums[11]) / 20.0;
	//  Number of players
	mapInfo_out.numPlayers      = nums[12];
	//  Island mode
	mapInfo_out.islandMode      = (nums[13] == 1) ? true : false;

	// World name hash
	uint16_t nameHash = nums[14];
	nameHash |= nums[15] << 5;
	nameHash |= nums[16] << 10;
	nameHash |= nums[17] << 15;

	for (size_t idx = 0; idx<worlds.size(); idx++)
		if (generateWorldNameHash(worlds[idx]) == nameHash) {
			mapInfo_out.worldName = worlds[idx];
			return true;
		}

	return false; // No valid world name found
}

/**
===============
Generates an ID-String for map generation.
The ID-String is an encoded version of the
information in a UniqueMapInfo structure.
Thus, the ID_String will contain all info
necessary to re-create a given random map.

mapIdsString_out: Output buffer for the resulting
                  Map-ID-String
mapInfo:     Information about the random map currently
             begin created (map specific info)
===============
*/
void UniqueRandomMapInfo::generateIdString
	(std::string & mapIdsString_out, const UniqueRandomMapInfo & mapInfo)
{
	//  Init
	assert(mapInfo.w <= 560);
	assert(mapInfo.h <= 560);
	assert(mapInfo.waterRatio >= 0.0);
	assert(mapInfo.waterRatio <= 1.0);
	assert(mapInfo.landRatio >= 0.0);
	assert(mapInfo.landRatio <= 1.0);
	assert(mapInfo.wastelandRatio >= 0.0);
	assert(mapInfo.wastelandRatio <= 1.0);
	assert(mapInfo.resource_amount <= Widelands::UniqueRandomMapInfo::raHigh);

	mapIdsString_out = "";
	int32_t nums[MAP_ID_DIGITS];
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix)
		nums[ix] = 0;

	// Generate world name hash
	uint16_t nameHash = generateWorldNameHash(mapInfo.worldName);

	//  Convert map random number
	nums [0] =  mapInfo.mapNumber        & 31;
	nums [1] = (mapInfo.mapNumber >>  5) & 31;
	nums [2] = (mapInfo.mapNumber >> 10) & 31;
	nums [3] = (mapInfo.mapNumber >> 15) & 31;

	nums [4] = (mapInfo.mapNumber >> 20) & 31;
	nums [5] = (mapInfo.mapNumber >> 25) & 31;
	nums [6] = (mapInfo.mapNumber >> 30) &  3;

	// Convert amount of resources
	nums [6] |= (mapInfo.resource_amount & 3) << 2;
	//  Convert width
	nums [7] = (mapInfo.w - 64) / 16;


	//  Convert height
	nums [8] = (mapInfo.h - 64) / 16;
	//  Convert water percent
	nums [9] = (mapInfo.waterRatio + 0.025) * 20.0;
	//  Convert land  percent
	nums[10] = (mapInfo.landRatio + 0.025)  * 20.0;
	//  Convert wasteland percent
	nums[11] = (mapInfo.wastelandRatio + 0.025)  * 20.0;

	//  Set number of islands
	nums[12] = mapInfo.numPlayers;
	//  Island mode
	nums[13] = mapInfo.islandMode ? 1 : 0;
	// World name hash (16 bit)
	nums[14] = nameHash & 31;
	nums[15] = (nameHash >> 5) & 31;

	nums[16] = (nameHash >> 10) & 31;
	nums[17] = (nameHash >> 15) & 1;

	//  Set id csm
	nums[MAP_ID_DIGITS - 3] = 0x15;
	//  Set id version number
	nums[MAP_ID_DIGITS - 2] = 0x01;
	//  Last number intentionally left blank
	nums[MAP_ID_DIGITS - 1] = 0x00;


	//  Nox xor everything
	//  This lets it look better
	//  Every change in a digit will result in a complete id change

	int32_t xorr = 0x0a;
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix)
		xorr = xorr ^ nums[ix];

	for (int32_t ix = MAP_ID_DIGITS - 1; ix >= 0; --ix) {
		nums[ix] = nums[ix] ^ xorr;
		xorr -= 7;
		xorr -= ix;
		if (xorr < 0)
			xorr &= 0x0000001f;
	}

	//  translate it to ASCII
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix) {
		mapIdsString_out += mapIdNumberToChar(nums[ix]);
		if (ix % 4 == 3 && ix != MAP_ID_DIGITS - 1)
			mapIdsString_out += "-";
	}
}


uint16_t Widelands::UniqueRandomMapInfo::generateWorldNameHash
	(const std::string & name)
{
	// This is only a simple digest algorithm. Thats enough for our purposes.

	uint16_t hash = 0xa5a5;
	int32_t posInHash = 0;

	for (size_t idx = 0; idx<name.size(); idx++) {
		hash ^= static_cast<uint8_t>(name[idx] & 0xff) << posInHash;
		posInHash ^= 8;
	}

	hash ^= (name.size() & 0xff) << 4;

	return hash;
}

// TODO: Also take mountain and water areas into bob generation
// (we have ducks and chamois)
// TODO: Move other map generation functions from Map to MapGenerator
// TODO: Define the "none"-bob to weigh other bobs lower within BobKinds...
// TODO: Clean up code
// TODO: Improve mapgenconf files for nicer generated worlds
// TODO: MapGen: Bob generation, configurable in mapgenconf
// TODO: MapGen: How to handle "Bob layers" ???
// TODO: MapGen: Resource generation, configurable in mapgenconf
// TODO: MapGen: Check out sample map
// TODO: MapGen: Generate Start positions
// TODO: MapGen: How to handle height profile in make_blah...
// TODO: MapGen: Display something else than
// TODO:         "Preparing..." when generating map...
// TODO: MapGen: Allow up to 3 different water areas

};
