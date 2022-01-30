/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_TERRAIN_DESCRIPTION_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_TERRAIN_DESCRIPTION_H

#include <map>

#include "base/macros.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "logic/widelands.h"

class LuaTable;
class Texture;

namespace Widelands {
class Descriptions;

/// TerrainTextures have a fixed size and are squares.
constexpr int kTextureSideLength = 64;

class TerrainDescription {
public:
	enum class Is {
		kArable = 0,
		kWalkable = 1,
		kWater = 2,
		kUnreachable = 4,
		kMineable = 8,
		kUnwalkable = 16,
	};

	struct Type {
		explicit Type(TerrainDescription::Is init_is);

		TerrainDescription::Is is;
		const char* descname;
		const Image* icon;
	};

	/**
	 * Arbitrary multiplicator for dither layers to prevent terrains from different
	 * add-ons from sharing the same layer. This value puts a limit on the number
	 * of add-ons that can safely be enabled at the same time.
	 */
	static constexpr uint16_t kMaxDitherLayerDisambiguator = 100;

	TerrainDescription(const LuaTable& table,
	                   Descriptions& descriptions,
	                   uint16_t dither_layer_disambiguator);
	~TerrainDescription() = default;

	/// The name used internally for this terrain.
	const std::string& name() const;

	/// The name showed to users of Widelands. Usually translated.
	const std::string& descname() const;

	const std::vector<std::string>& texture_paths() const;

	/// Returns the texture for the given gametime.
	const Image& get_texture(uint32_t gametime) const;
	void add_texture(const Image* texture);

	// Sets the base minimap color.
	void set_minimap_color(const RGBColor& color);

	// Return the basic terrain colour to be used in the minimap.
	// 'shade' must be a brightness value, i.e. in [-128, 127].
	const RGBColor& get_minimap_color(int shade) const;

	/// Returns the type of terrain this is (water, walkable, and so on).
	Is get_is() const;
	/// Returns a list of the types that match get_is()
	const std::vector<TerrainDescription::Type> get_types() const;

	/// Returns the valid resource with the given index.
	DescriptionIndex get_valid_resource(DescriptionIndex index) const;

	/// Returns the number of valid resources.
	size_t get_num_valid_resources() const;

	/// Returns the the valid resources.
	std::vector<DescriptionIndex> valid_resources() const;

	/// Returns true if this resource can be found in this terrain type.
	bool is_resource_valid(DescriptionIndex res) const;

	/// Returns the resource index that can by default always be found in this
	/// terrain.
	DescriptionIndex get_default_resource() const;

	/// Returns the default amount of resources you can find in this terrain.
	ResourceAmount get_default_resource_amount() const;

	/// Returns the dither layer, i.e. the information in which zlayer this
	/// texture should be drawn.
	int32_t dither_layer() const;

	/// Parameters for terrain affinity of immovables.
	/// Temperature is in arbitrary units.
	int temperature() const;

	/// Humidity, ranging from 0 to 1000.
	int humidity() const;

	/// Fertility, ranging from 0 to 1000.
	int fertility() const;

	// The terrain which certain workers can transform this terrain into.
	std::string enhancement(const std::string& category) const;
	void set_enhancement(const std::string& category, const std::string& terrain);

	void replace_textures(const LuaTable&);

private:
	const std::string name_;
	const std::string descname_;
	Is is_;
	std::vector<DescriptionIndex> valid_resources_;
	DescriptionIndex default_resource_index_;
	int default_resource_amount_;
	int dither_layer_;
	int frame_length_;
	int temperature_;
	int fertility_;
	int humidity_;
	std::map<std::string, std::string> enhancement_;
	std::vector<std::string> texture_paths_;
	std::vector<const Image*> textures_;
	RGBColor minimap_colors_[256];

	DISALLOW_COPY_AND_ASSIGN(TerrainDescription);
};

inline TerrainDescription::Is operator|(TerrainDescription::Is left, TerrainDescription::Is right) {
	return TerrainDescription::Is(static_cast<int>(left) | static_cast<int>(right));
}
inline int operator&(TerrainDescription::Is left, TerrainDescription::Is right) {
	return static_cast<int>(left) & static_cast<int>(right);
}

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_TERRAIN_DESCRIPTION_H
