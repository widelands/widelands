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
	"greenland", std::map<std::string, std::string>
	{
		// No changes.
	}),
std::make_pair(
	"blackland", std::map<std::string, std::string>
	{
		// No changes.
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
		// No changes.
	}),
std::make_pair(
	"winterland", std::map<std::string, std::string>
	{
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
