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

#ifndef TERRAIN_DESCRIPTION_H
#define TERRAIN_DESCRIPTION_H

#include <string>

#include <boost/noncopyable.hpp>

#include "logic/widelands.h"
#include "logic/world/resource_description.h"

class LuaTable;

namespace Widelands {

class EditorCategory;
class World;

class TerrainDescription : boost::noncopyable {
public:
	enum Type {
		GREEN = 0,
		DRY = 1,
		WATER =  2,
		ACID = 4,
		MOUNTAIN = 8,
		UNPASSABLE = 16,
	};

	TerrainDescription(const LuaTable& table, const World&);
	~TerrainDescription();

	// The name used internally for this terrain.
	const std::string& name() const;

	// The name showed to users of Widelands. Usually translated.
	const std::string& descname() const;

	// Returns the texture index for this terrain.
	uint32_t get_texture() const;

	// Returns the type of terrain this is (water, walkable, and so on).
	Type get_is() const;

	// Returns the valid resource with the given index.
	Resource_Index get_valid_resource(uint8_t index) const;

	// Returns the number of valid resources.
	int get_num_valid_resources() const;

	// Returns true if this resource can be found in this terrain type.
	bool is_resource_valid(int32_t res) const;

	// Returns the resource index that can by default always be found in this
	// terrain.
	int8_t get_default_resource() const;

	// Returns the default amount of resources you can find in this terrain.
	int32_t get_default_resource_amount() const;

	// Returns the dither layer, i.e. the information in which zlayer this
	// texture should be drawn.
	int32_t dither_layer() const;

	// Returns the editor category.
	const EditorCategory& editor_category() const;

private:
	const std::string name_;
	const std::string descname_;
	const EditorCategory* editor_category_;  // not owned.
	Type is_;
	std::vector<uint8_t> valid_resources_;
	int8_t default_resource_index_;
	int32_t default_resource_amount_;
	const std::vector<std::string> texture_paths_;
	int32_t dither_layer_;
	uint32_t texture_;  //  renderer's texture
};

}  // namespace Widelands

#endif /* end of include guard: TERRAIN_DESCRIPTION_H */
