/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "logic/map_objects/descriptions_compatibility_table.h"

// Whenever we break savegame compatibility, we can empty these maps
DescriptionsCompatibilityTable::DescriptionsCompatibilityTable()
   :  // Workers
     workers_{},
     // Wares
     wares_{
        // {"old_name", "new_name"},
     },
     // Buildings
     buildings_{
        {"amazons_cassava_root_plantation", "amazons_cassava_plantation"},
     },
     // Ships
     ships_{} {
}

const std::string& DescriptionsCompatibilityTable::lookup_entry(
   const std::string& entry, const std::map<std::string, std::string>& table) const {
	const auto& i = table.find(entry);
	if (i != table.end()) {
		return i->second;
	}
	return entry;
}

const std::string& DescriptionsCompatibilityTable::lookup_worker(const std::string& worker) const {
	return lookup_entry(worker, workers_);
}

const std::string& DescriptionsCompatibilityTable::lookup_ware(const std::string& ware) const {
	return lookup_entry(ware, wares_);
}

const std::string&
DescriptionsCompatibilityTable::lookup_building(const std::string& building) const {
	return lookup_entry(building, buildings_);
}

const std::string& DescriptionsCompatibilityTable::lookup_ship(const std::string& ship) const {
	return lookup_entry(ship, ships_);
}

PostOneWorldLegacyLookupTable::PostOneWorldLegacyLookupTable() :
critters_
{
		// All critters are always needed for map compatibility
	{"elk", "moose"},
},
immovables_
{
		// World immovables are always needed for map compatibility
	{"blackland_stones1", "blackland_rocks1"},
	{"blackland_stones2", "blackland_rocks2"},
	{"blackland_stones3", "blackland_rocks3"},
	{"blackland_stones4", "blackland_rocks4"},
	{"blackland_stones5", "blackland_rocks5"},
	{"blackland_stones6", "blackland_rocks6"},
	{"desert_stones1", "desert_rocks1"},
	{"desert_stones2", "desert_rocks2"},
	{"desert_stones3", "desert_rocks3"},
	{"desert_stones4", "desert_rocks4"},
	{"desert_stones5", "desert_rocks5"},
	{"desert_stones6", "desert_rocks6"},
	{"winterland_stones1", "winterland_rocks1"},
	{"winterland_stones2", "winterland_rocks2"},
	{"winterland_stones3", "winterland_rocks3"},
	{"winterland_stones4", "winterland_rocks4"},
	{"winterland_stones5", "winterland_rocks5"},
	{"winterland_stones6", "winterland_rocks6"},
	{"greenland_stones1", "greenland_rocks1"},
	{"greenland_stones2", "greenland_rocks2"},
	{"greenland_stones3", "greenland_rocks3"},
	{"greenland_stones4", "greenland_rocks4"},
	{"greenland_stones5", "greenland_rocks5"},
	{"greenland_stones6", "greenland_rocks6"},
		// Tribe immovables below - can be deleted whenever we break map compatibility
	{"cassavarootfield_harvested", "cassavafield_harvested"},
	{"cassavarootfield_ripe", "cassavafield_ripe"},
	{"cassavarootfield_medium", "cassavafield_medium"},
	{"cassavarootfield_small", "cassavafield_small"},
	{"cassavarootfield_tiny", "cassavafield_tiny"},
},
resources_
{
		// All resources are always needed for map compatibility
	{"granite", "resource_stones"},
	{"stones", "resource_stones"},
	{"gold", "resource_gold"},
	{"iron", "resource_iron"},
	{"coal", "resource_coal"},
	{"water", "resource_water"},
	{"fish", "resource_fish"}
},

// Renamed German spelled terrains into English
terrains_
{
		// All terrains are always needed for map compatibility
	{"wiese1", "summer_meadow1"},
	{"wiese2", "summer_meadow2"},
	{"wiese3", "summer_meadow3"},
	{"wiese4", "summer_meadow4"},
	{"steppe", "summer_steppe"},
	{"steppe_kahl", "summer_steppe_barren"},
	{"bergwiese", "summer_mountain_meadow"},
	{"berg1", "summer_mountain1"},
	{"berg2", "summer_mountain2"},
	{"berg3", "summer_mountain3"},
	{"berg4", "summer_mountain4"},
	{"sumpf", "summer_swamp"},
	{"strand", "summer_beach"},
	{"schnee", "summer_snow"},
	{"wasser", "summer_water"},
	{"ashes", "ashes1"},
	{"taiga", "winter_tundra"},
	{"tundra", "winter_taiga1"},
	{"tundra2", "winter_taiga2"},
	{"tundra3", "winter_taiga3"},
}
{
}

std::string PostOneWorldLegacyLookupTable::lookup_resource(const std::string& resource) const {
	return lookup_entry(resource, resources_);
}

std::string PostOneWorldLegacyLookupTable::lookup_terrain(const std::string& terrain) const {
	return lookup_entry(terrain, terrains_);
}

std::string PostOneWorldLegacyLookupTable::lookup_critter(const std::string& critter) const {
	return lookup_entry(critter, critters_);
}

std::string PostOneWorldLegacyLookupTable::lookup_immovable(const std::string& immovable) const {
	return lookup_entry(immovable, immovables_);
}

OneWorldLegacyLookupTable::OneWorldLegacyLookupTable(const std::string& old_world_name)
   : old_world_name_(old_world_name),
     // RESOURCES - They were all the same for all worlds.
     // All resources are always needed for map compatibility
     resources_{{"granit", "resource_stones"}, {"stones", "resource_stones"},
                {"gold", "resource_gold"},     {"iron", "resource_iron"},
                {"coal", "resource_coal"},     {"water", "resource_water"},
                {"fish", "resource_fish"}},

     // TERRAINS
     // All terrains are always needed for map compatibility
     terrains_{
        std::make_pair("greenland",
                       std::map<std::string, std::string>{
                          // Renamed German spelled terrains into English
                          // This has to be here to get scenarios work
                          {"ashes", "ashes1"},
                          {"wiese1", "summer_meadow1"},
                          {"wiese2", "summer_meadow2"},
                          {"wiese3", "summer_meadow3"},
                          {"wiese4", "summer_meadow4"},
                          {"steppe", "summer_steppe"},
                          {"steppe_kahl", "summer_steppe_barren"},
                          {"bergwiese", "summer_mountain_meadow"},
                          {"berg1", "summer_mountain1"},
                          {"berg2", "summer_mountain2"},
                          {"berg3", "summer_mountain3"},
                          {"berg4", "summer_mountain4"},
                          {"sumpf", "summer_swamp"},
                          {"strand", "summer_beach"},
                          {"schnee", "summer_snow"},
                          {"wasser", "summer_water"},
                          {"taiga", "winter_tundra"},
                          {"tundra", "winter_taiga1"},
                          {"tundra2", "winter_taiga2"},
                          {"tundra3", "winter_taiga3"},
                       }),
        std::make_pair("blackland",
                       std::map<std::string, std::string>{
                          {"ashes", "ashes1"},
                          {"mountain1", "wasteland_mountain1"},
                          {"mountain2", "wasteland_mountain2"},
                          {"mountain3", "wasteland_mountain3"},
                          {"mountain4", "wasteland_mountain4"},
                          {"strand", "wasteland_beach"},
                          {"water", "wasteland_water"},
                          {"taiga", "winter_tundra"},
                          {"tundra", "winter_taiga1"},
                          {"tundra2", "winter_taiga2"},
                          {"tundra3", "winter_taiga3"},
                       }),
        std::make_pair("winterland",
                       std::map<std::string, std::string>{
                          {"ashes", "ashes1"},
                          {"ice_flows", "ice_floes"},
                          {"ice_flows2", "ice_floes2"},
                          {"mountain1", "winter_mountain1"},
                          {"mountain2", "winter_mountain2"},
                          {"mountain3", "winter_mountain3"},
                          {"mountain4", "winter_mountain4"},
                          {"strand", "winter_beach"},
                          {"water", "winter_water"},
                          {"taiga", "winter_tundra"},
                          {"tundra", "winter_taiga1"},
                          {"tundra2", "winter_taiga2"},
                          {"tundra3", "winter_taiga3"},
                       }),
        std::make_pair("desert",
                       std::map<std::string, std::string>{
                          {"ashes", "ashes1"},
                          {"beach", "desert_beach"},
                          {"steppe", "desert_steppe"},
                          {"wasser", "desert_water"},
                          {"taiga", "winter_tundra"},
                          {"tundra", "winter_taiga1"},
                          {"tundra2", "winter_taiga2"},
                          {"tundra3", "winter_taiga3"},
                       }),
     },

     // CRITTERS
     // All critters are always needed for map compatibility
     critters_{
        std::make_pair("greenland",
                       std::map<std::string, std::string>{
                          {"deer", "stag"},
                          {"elk", "moose"},
                       }),
        std::make_pair("blackland",
                       std::map<std::string, std::string>{
                          // No changes.
                       }),
        std::make_pair("winterland",
                       std::map<std::string, std::string>{
                          {"deer", "stag"},
                          {"elk", "moose"},
                       }),
        std::make_pair("desert",
                       std::map<std::string, std::string>{
                          // No changes.
                       }),
     },

     // IMMOVABLES
     immovables_{
        std::make_pair(
           "blackland",
           std::map<std::string, std::string>{
              // World immovables are always needed for map compatibility
              {"stones1", "blackland_rocks1"},
              {"stones2", "blackland_rocks2"},
              {"stones3", "blackland_rocks3"},
              {"stones4", "blackland_rocks4"},
              {"stones5", "blackland_rocks5"},
              {"stones6", "blackland_rocks6"},
              {"sstones1", "standing_stone1_wasteland"},
              {"sstones2", "standing_stone2_wasteland"},
              {"sstones3", "standing_stone3_wasteland"},
              {"sstones4", "standing_stone4_wasteland"},
              {"sstones5", "standing_stone5_wasteland"},
              {"sstones6", "standing_stone6"},
              {"sstones7", "standing_stone7"},
              {"tree1", "umbrella_red_wasteland_old"},
              {"tree1_m", "umbrella_red_wasteland_mature"},
              {"tree1_s", "umbrella_red_wasteland_pole"},
              {"tree1_t", "umbrella_red_wasteland_sapling"},
              {"tree2", "mushroom_dark_wasteland_old"},
              {"tree2_m", "mushroom_dark_wasteland_mature"},
              {"tree2_s", "mushroom_dark_wasteland_pole"},
              {"tree2_t", "mushroom_dark_wasteland_sapling"},
              {"tree3", "twine_wasteland_old"},
              {"tree3_m", "twine_wasteland_mature"},
              {"tree3_s", "twine_wasteland_pole"},
              {"tree3_t", "twine_wasteland_sapling"},
              {"tree4", "umbrella_green_wasteland_old"},
              {"tree4_m", "umbrella_green_wasteland_mature"},
              {"tree4_s", "umbrella_green_wasteland_pole"},
              {"tree4_t", "umbrella_green_wasteland_sapling"},
              {"tree5", "mushroom_red_wasteland_old"},
              {"tree5_m", "mushroom_red_wasteland_mature"},
              {"tree5_s", "mushroom_red_wasteland_pole"},
              {"tree5_t", "mushroom_red_wasteland_sapling"},
              {"tree6", "mushroom_green_wasteland_old"},
              {"tree6_m", "mushroom_green_wasteland_mature"},
              {"tree6_s", "mushroom_green_wasteland_pole"},
              {"tree6_t", "mushroom_green_wasteland_sapling"},
              {"tree7", "cirrus_wasteland_old"},
              {"tree7_m", "cirrus_wasteland_mature"},
              {"tree7_s", "cirrus_wasteland_pole"},
              {"tree7_t", "cirrus_wasteland_sapling"},
              {"tree8", "liana_wasteland_old"},
              {"tree8_m", "liana_wasteland_mature"},
              {"tree8_s", "liana_wasteland_pole"},
              {"tree8_t", "liana_wasteland_sapling"},
              // Tribe immovables below - can be deleted whenever we break map compatibility
           }),
        std::make_pair(
           "desert",
           std::map<std::string, std::string>{
              // World immovables are always needed for map compatibility
              {"deadtree2", "deadtree5"},
              {"deadtree3", "deadtree6"},
              {"sstones1", "standing_stone1_desert"},
              {"sstones2", "standing_stone2_desert"},
              {"sstones3", "standing_stone3_desert"},
              {"sstones4", "standing_stone4_desert"},
              {"sstones5", "standing_stone5_desert"},
              {"sstones6", "standing_stone6"},
              {"sstones7", "standing_stone7"},
              {"stones1", "desert_rocks1"},
              {"stones2", "desert_rocks2"},
              {"stones3", "desert_rocks3"},
              {"stones4", "desert_rocks4"},
              {"stones5", "desert_rocks5"},
              {"stones6", "desert_rocks6"},
              {"tree1", "palm_date_desert_old"},
              {"tree1_m", "palm_date_desert_mature"},
              {"tree1_s", "palm_date_desert_pole"},
              {"tree1_t", "palm_date_desert_sapling"},
              {"tree2", "palm_borassus_desert_old"},
              {"tree2_m", "palm_borassus_desert_mature"},
              {"tree2_s", "palm_borassus_desert_pole"},
              {"tree2_t", "palm_borassus_desert_sapling"},
              {"tree3", "palm_coconut_desert_old"},
              {"tree3_m", "palm_coconut_desert_mature"},
              {"tree3_s", "palm_coconut_desert_pole"},
              {"tree3_t", "palm_coconut_desert_sapling"},
              {"tree4", "palm_roystonea_desert_old"},
              {"tree4_m", "palm_roystonea_desert_mature"},
              {"tree4_s", "palm_roystonea_desert_pole"},
              {"tree4_t", "palm_roystonea_desert_sapling"},
              {"tree5", "palm_oil_desert_old"},
              {"tree5_m", "palm_oil_desert_mature"},
              {"tree5_s", "palm_oil_desert_pole"},
              {"tree5_t", "palm_oil_desert_sapling"},
              {"tree6", "beech_summer_old"},
              {"tree6_m", "beech_summer_mature"},
              {"tree6_s", "beech_summer_pole"},
              {"tree6_t", "beech_summer_sapling"},
              {"tree7", "larch_summer_old"},
              {"tree7_m", "larch_summer_mature"},
              {"tree7_s", "larch_summer_pole"},
              {"tree7_t", "larch_summer_sapling"},
              {"tree8", "rowan_summer_old"},
              {"tree8_m", "rowan_summer_mature"},
              {"tree8_s", "rowan_summer_pole"},
              {"tree8_t", "rowan_summer_sapling"},
              // Tribe immovables below - can be deleted whenever we break map compatibility
           }),
        std::make_pair(
           "greenland",
           std::map<std::string, std::string>{
              // World immovables are always needed for map compatibility
              {"sstones1", "standing_stone1_summer"}, {"sstones2", "standing_stone2_summer"},
              {"sstones3", "standing_stone3_summer"}, {"sstones4", "standing_stone4_summer"},
              {"sstones5", "standing_stone5_summer"}, {"sstones6", "standing_stone6"},
              {"sstones7", "standing_stone7"},        {"stones1", "greenland_rocks1"},
              {"stones2", "greenland_rocks2"},        {"stones3", "greenland_rocks3"},
              {"stones4", "greenland_rocks4"},        {"stones5", "greenland_rocks5"},
              {"stones6", "greenland_rocks6"},        {"tree1", "aspen_summer_old"},
              {"tree1_m", "aspen_summer_mature"},     {"tree1_s", "aspen_summer_pole"},
              {"tree1_t", "aspen_summer_sapling"},    {"tree2", "oak_summer_old"},
              {"tree2_m", "oak_summer_mature"},       {"tree2_s", "oak_summer_pole"},
              {"tree2_t", "oak_summer_sapling"},      {"tree3", "spruce_summer_old"},
              {"tree3_m", "spruce_summer_mature"},    {"tree3_s", "spruce_summer_pole"},
              {"tree3_t", "spruce_summer_sapling"},   {"tree4", "alder_summer_old"},
              {"tree4_m", "alder_summer_mature"},     {"tree4_s", "alder_summer_pole"},
              {"tree4_t", "alder_summer_sapling"},    {"tree5", "birch_summer_old"},
              {"tree5_m", "birch_summer_mature"},     {"tree5_s", "birch_summer_pole"},
              {"tree5_t", "birch_summer_sapling"},    {"tree6", "beech_summer_old"},
              {"tree6_m", "beech_summer_mature"},     {"tree6_s", "beech_summer_pole"},
              {"tree6_t", "beech_summer_sapling"},    {"tree7", "larch_summer_old"},
              {"tree7_m", "larch_summer_mature"},     {"tree7_s", "larch_summer_pole"},
              {"tree7_t", "larch_summer_sapling"},    {"tree8", "rowan_summer_old"},
              {"tree8_m", "rowan_summer_mature"},     {"tree8_s", "rowan_summer_pole"},
              {"tree8_t", "rowan_summer_sapling"},
              // Tribe immovables below - can be deleted whenever we break map compatibility
           }),
        std::make_pair(
           "winterland",
           std::map<std::string, std::string>{
              // World immovables are always needed for map compatibility
              {"sstones1", "standing_stone1_winter"},
              {"sstones2", "standing_stone2_winter"},
              {"sstones3", "standing_stone3_winter"},
              {"sstones4", "standing_stone4_winter"},
              {"sstones5", "standing_stone5_winter"},
              {"sstones6", "standing_stone6"},
              {"sstones7", "standing_stone7"},
              {"stones1", "winterland_rocks1"},
              {"stones2", "winterland_rocks2"},
              {"stones3", "winterland_rocks3"},
              {"stones4", "winterland_rocks4"},
              {"stones5", "winterland_rocks5"},
              {"stones6", "winterland_rocks6"},
              {"track", "track_winter"},
              {"tree1", "aspen_summer_old"},
              {"tree1_m", "aspen_summer_mature"},
              {"tree1_s", "aspen_summer_pole"},
              {"tree1_t", "aspen_summer_sapling"},
              {"tree2", "oak_summer_old"},
              {"tree2_m", "oak_summer_mature"},
              {"tree2_s", "oak_summer_pole"},
              {"tree2_t", "oak_summer_sapling"},
              {"tree3", "spruce_summer_old"},
              {"tree3_m", "spruce_summer_mature"},
              {"tree3_s", "spruce_summer_pole"},
              {"tree3_t", "spruce_summer_sapling"},
              {"tree4", "maple_winter_old"},
              {"tree4_m", "maple_winter_mature"},
              {"tree4_s", "maple_winter_pole"},
              {"tree4_t", "maple_winter_sapling"},
              {"tree5", "birch_summer_old"},
              {"tree5_m", "birch_summer_mature"},
              {"tree5_s", "birch_summer_pole"},
              {"tree5_t", "birch_summer_sapling"},
              {"tree6", "beech_summer_old"},
              {"tree6_m", "beech_summer_mature"},
              {"tree6_s", "beech_summer_pole"},
              {"tree6_t", "beech_summer_sapling"},
              {"tree7", "larch_summer_old"},
              {"tree7_m", "larch_summer_mature"},
              {"tree7_s", "larch_summer_pole"},
              {"tree7_t", "larch_summer_sapling"},
              {"tree8", "rowan_summer_old"},
              {"tree8_m", "rowan_summer_mature"},
              {"tree8_s", "rowan_summer_pole"},
              {"tree8_t", "rowan_summer_sapling"},
              // Tribe immovables below - can be deleted whenever we break map compatibility
           }),
     }

{
}

const std::string& OneWorldLegacyLookupTable::lookup_world_entry(
   const std::string& entry,
   const std::map<std::string, std::map<std::string, std::string>>& table) const {
	const std::map<std::string, std::string>& world_entries = table.at(old_world_name_);
	const auto& i = world_entries.find(entry);
	if (i != world_entries.end()) {
		return i->second;
	}
	return entry;
}

std::string OneWorldLegacyLookupTable::lookup_resource(const std::string& resource) const {
	return lookup_entry(resource, resources_);
}

std::string OneWorldLegacyLookupTable::lookup_terrain(const std::string& terrain) const {
	return lookup_world_entry(terrain, terrains_);
}

std::string OneWorldLegacyLookupTable::lookup_critter(const std::string& critter) const {
	return lookup_world_entry(critter, critters_);
}

std::string OneWorldLegacyLookupTable::lookup_immovable(const std::string& immovable) const {
	return lookup_world_entry(immovable, immovables_);
}
