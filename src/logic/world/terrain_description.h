/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_LOGIC_WORLD_TERRAIN_DESCRIPTION_H
#define WL_LOGIC_WORLD_TERRAIN_DESCRIPTION_H

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "graphic/color.h"
#include "logic/widelands.h"
#include "logic/world/resource_description.h"

class LuaTable;
class Texture;

namespace Widelands {

class EditorCategory;
class World;

/// TerrainTextures have a fixed size and are squares.
constexpr int kTextureSideLength = 64;

class TerrainDescription {
public:
	enum Type {
		kGreen = 0,
		kDry = 1,
		kWater =  2,
		kDead = 4,
		kMountain = 8,
		kImpassable = 16,
	};

	TerrainDescription(const LuaTable& table, const World&);
	~TerrainDescription();

	/// The name used internally for this terrain.
	const std::string& name() const;

	/// The name showed to users of Widelands. Usually translated.
	const std::string& descname() const;


	const std::vector<std::string>& texture_paths() const;

	/// Returns the texture for the given gametime.
	const Texture& get_texture(uint32_t gametime) const;
	void add_texture(std::unique_ptr<Texture> texture);

	// Sets the base minimap color.
	void set_minimap_color(const RGBColor& color);

	// Return the basic terrain colour to be used in the minimap.
	// 'shade' must be a brightness value, i.e. in [-128, 127].
	const RGBColor& get_minimap_color(int shade);

	/// Returns the type of terrain this is (water, walkable, and so on).
	Type get_is() const;

	/// Returns the valid resource with the given index.
	ResourceIndex get_valid_resource(uint8_t index) const;

	/// Returns the number of valid resources.
	int get_num_valid_resources() const;

	/// Returns true if this resource can be found in this terrain type.
	bool is_resource_valid(int32_t res) const;

	/// Returns the resource index that can by default always be found in this
	/// terrain.
	int get_default_resource() const;

	/// Returns the default amount of resources you can find in this terrain.
	int32_t get_default_resource_amount() const;

	/// Returns the dither layer, i.e. the information in which zlayer this
	/// texture should be drawn.
	int32_t dither_layer() const;

	/// Returns the editor category.
	const EditorCategory& editor_category() const;

	/// Parameters for terrain affinity of immovables.
	/// Temperature is in arbitrary units.
	double temperature() const;

	/// Humidity in percent [0, 1].
	double humidity() const;

	/// Fertility in percent [0, 1].
	double fertility() const;

private:
	const std::string name_;
	const std::string descname_;
	const EditorCategory* editor_category_;  ///< not owned.
	Type is_;
	std::vector<uint8_t> valid_resources_;
	int default_resource_index_;
	int default_resource_amount_;
	int dither_layer_;
	int frame_length_;
	double temperature_;
	double fertility_;
	double humidity_;
	std::vector<std::string> texture_paths_;
	std::vector<std::unique_ptr<Texture>> textures_;
	RGBColor    minimap_colors_[256];

	DISALLOW_COPY_AND_ASSIGN(TerrainDescription);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WORLD_TERRAIN_DESCRIPTION_H
