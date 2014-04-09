/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "map_io/one_world_legacy_lookup_table.h"

#include "log.h"

OneWorldLegacyLookupTable::OneWorldLegacyLookupTable() :
// RESOURCES - They were all the same for all worlds.
resources_
{
	{"granit", "granite"},
},

// TERRAINS
terrains_
{std::make_pair(
	"greenland", std::map<std::string, std::string>
	{
		// No changes for greenland.
	}),
std::make_pair(
	"blackland", std::map<std::string, std::string>
	{
		{"strand", "wasteland_beach"},
		{"water", "wasteland_water"},
		{"mountain1", "wasteland_mountain1"},
		{"mountain2", "wasteland_mountain2"},
		{"mountain3", "wasteland_mountain3"},
		{"mountain4", "wasteland_mountain4"},
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
		{"strand", "winter_beach"},
		{"ice_flows", "ice_floes"},
		{"ice_flows2", "ice_floes2"},
		{"mountain1", "winter_mountain1"},
		{"mountain2", "winter_mountain2"},
		{"mountain3", "winter_mountain3"},
		{"mountain4", "winter_mountain4"},
		{"water", "winter_water"},
	}),
std::make_pair(
	"desert", std::map<std::string, std::string>
	{
		{"beach", "desert_beach"},
		{"steppe", "desert_steppe"},
		{"wasser", "desert_water"},
	}),
},

// CRITTERS
critters_
{std::make_pair(
	"greenland", std::map<std::string, std::string>
	{
		{"deer", "stag"},
	}),
std::make_pair(
	"blackland", std::map<std::string, std::string>
	{
		// No changes.
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
		{"deer", "stag"},
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
		// No changes.
	}),
},

// IMMOVABLES
immovables_
{std::make_pair(
std::make_pair(
	"blackland", std::map<std::string, std::string>
	{
		{"stones1", "blackland_stones1"},
		{"stones2", "blackland_stones2"},
		{"stones3", "blackland_stones3"},
		{"stones4", "blackland_stones4"},
		{"stones5", "blackland_stones5"},
		{"stones6", "blackland_stones6"},
		{"tree1_t", "aspen_wasteland_sapling"},
		{"tree1_s", "aspen_wasteland_pole"},
		{"tree1_m", "aspen_wasteland_mature"},
		{"tree1", "aspen_wasteland_old"},
		{"tree2_t", "oak_wasteland_sapling"},
		{"tree2_s", "oak_wasteland_pole"},
		{"tree2_m", "oak_wasteland_mature"},
		{"tree2", "oak_wasteland_old"},
		{"tree3_t", "spruce_wasteland_sapling"},
		{"tree3_s", "spruce_wasteland_pole"},
		{"tree3_m", "spruce_wasteland_mature"},
		{"tree3", "spruce_wasteland_old"},

	}),
std::make_pair(
	"desert", std::map<std::string, std::string>
	{
		{"cactus2", "cactus5"},
		{"deadtree2", "deadtree5"},
		{"deadtree3", "deadtree6"},
		{"sstones1", "sstones8"},
		{"sstones2", "sstones9"},
		{"sstones3", "sstones10"},
		{"sstones4", "sstones11"},
		{"sstones5", "sstones12"},
		{"stones1", "desert_stones1"},
		{"stones2", "desert_stones2"},
		{"stones3", "desert_stones3"},
		{"stones4", "desert_stones4"},
		{"stones5", "desert_stones5"},
		{"stones6", "desert_stones6"},
		{"tree6_t", "beech_summer_sapling"},
		{"tree6_s", "beech_summer_pole"},
		{"tree6_m", "beech_summer_mature"},
		{"tree6", "beech_summer_old"},
		{"tree7_t", "larch_summer_sapling"},
		{"tree7_s", "larch_summer_pole"},
		{"tree7_m", "larch_summer_mature"},
		{"tree7", "larch_summer_old"},
		{"tree8_t", "rowan_summer_sapling"},
		{"tree8_s", "rowan_summer_pole"},
		{"tree8_m", "rowan_summer_mature"},
		{"tree8", "rowan_summer_old"},
	}),
	"greenland", std::map<std::string, std::string>
	{
		{"sstones1", "sstones13"},
		{"sstones2", "sstones14"},
		{"sstones3", "sstones15"},
		{"sstones4", "sstones16"},
		{"sstones5", "sstones16"},
		{"stones1", "greenland_stones1"},
		{"stones2", "greenland_stones2"},
		{"stones3", "greenland_stones3"},
		{"stones4", "greenland_stones4"},
		{"stones5", "greenland_stones5"},
		{"stones6", "greenland_stones6"},
		{"tree1_t", "aspen_summer_sapling"},
		{"tree1_s", "aspen_summer_pole"},
		{"tree1_m", "aspen_summer_mature"},
		{"tree1", "aspen_summer_old"},
		{"tree2_t", "oak_summer_sapling"},
		{"tree2_s", "oak_summer_pole"},
		{"tree2_m", "oak_summer_mature"},
		{"tree2", "oak_summer_old"},
		{"tree3_t", "spruce_summer_sapling"},
		{"tree3_s", "spruce_summer_pole"},
		{"tree3_m", "spruce_summer_mature"},
		{"tree3", "spruce_summer_old"},
		{"tree4_t", "alder_summer_sapling"},
		{"tree4_s", "alder_summer_pole"},
		{"tree4_m", "alder_summer_mature"},
		{"tree4", "alder_summer_old"},
		{"tree5_t", "birch_summer_sapling"},
		{"tree5_s", "birch_summer_pole"},
		{"tree5_m", "birch_summer_mature"},
		{"tree5", "birch_summer_old"},
		{"tree6_t", "beech_summer_sapling"},
		{"tree6_s", "beech_summer_pole"},
		{"tree6_m", "beech_summer_mature"},
		{"tree6", "beech_summer_old"},
		{"tree7_t", "larch_summer_sapling"},
		{"tree7_s", "larch_summer_pole"},
		{"tree7_m", "larch_summer_mature"},
		{"tree7", "larch_summer_old"},
		{"tree8_t", "rowan_summer_sapling"},
		{"tree8_s", "rowan_summer_pole"},
		{"tree8_m", "rowan_summer_mature"},
		{"tree8", "rowan_summer_old"},
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
		{"sstones1", "sstones18"},
		{"sstones2", "sstones19"},
		{"sstones3", "sstones20"},
		{"sstones4", "sstones21"},
		{"sstones5", "sstones22"},
		{"stones1", "winterland_stones1"},
		{"stones2", "winterland_stones2"},
		{"stones3", "winterland_stones3"},
		{"stones4", "winterland_stones4"},
		{"stones5", "winterland_stones5"},
		{"stones6", "winterland_stones6"},
		{"tree1_t", "aspen_summer_sapling"},
		{"tree1_s", "aspen_summer_pole"},
		{"tree1_m", "aspen_summer_mature"},
		{"tree1", "aspen_summer_old"},
		{"tree2_t", "oak_summer_sapling"},
		{"tree2_s", "oak_summer_pole"},
		{"tree2_m", "oak_summer_mature"},
		{"tree2", "oak_summer_old"},
		{"tree3_t", "spruce_summer_sapling"},
		{"tree3_s", "spruce_summer_pole"},
		{"tree3_m", "spruce_summer_mature"},
		{"tree3", "spruce_summer_old"},
		{"tree4_t", "maple_winter_sapling"},
		{"tree4_s", "maple_winter_pole"},
		{"tree4_m", "maple_winter_mature"},
		{"tree4", "maple_winter_old"},
		{"tree5_t", "birch_summer_sapling"},
		{"tree5_s", "birch_summer_pole"},
		{"tree5_m", "birch_summer_mature"},
		{"tree5", "birch_summer_old"},
		{"tree6_t", "beech_summer_sapling"},
		{"tree6_s", "beech_summer_pole"},
		{"tree6_m", "beech_summer_mature"},
		{"tree6", "beech_summer_old"},
		{"tree7_t", "larch_summer_sapling"},
		{"tree7_s", "larch_summer_pole"},
		{"tree7_m", "larch_summer_mature"},
		{"tree7", "larch_summer_old"},
		{"tree8_t", "rowan_summer_sapling"},
		{"tree8_s", "rowan_summer_pole"},
		{"tree8_m", "rowan_summer_mature"},
		{"tree8", "rowan_summer_old"},
		// No changes.
	}),
}

{}

std::string OneWorldLegacyLookupTable::lookup_resource(const std::string& resource) const {
	const auto& i = resources_.find(resource);
	if (i == resources_.end()) {
		return resource;
	}
	return i->second;
};

std::string OneWorldLegacyLookupTable::lookup_terrain(const std::string& world,
                                                      const std::string& terrain) const {
	if (!world.empty()) {
		const std::map<std::string, std::string>& world_terrains = terrains_.at(world);
		const auto& i = world_terrains.find(terrain);
		if (i != world_terrains.end()) {
			return i->second;
		}
	}
	return terrain;
}

std::string OneWorldLegacyLookupTable::lookup_critter(const std::string& world,
                                                      const std::string& critter) const {
	if (!world.empty()) {
		const std::map<std::string, std::string>& world_critters = critters_.at(world);
		const auto& i = world_critters.find(critter);
		if (i != world_critters.end()) {
			return i->second;
		}
	}
	return critter;
}

std::string OneWorldLegacyLookupTable::lookup_immovable(const std::string& world,
                                                      const std::string& immovable) const {
	if (!world.empty()) {
		const std::map<std::string, std::string>& world_immovables = immovables_.at(world);
		const auto& i = world_immovables.find(immovable);
		if (i != world_immovables.end()) {
			return i->second;
		}
	}
	return immovable;
}
