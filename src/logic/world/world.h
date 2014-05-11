/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef WORLD_H
#define WORLD_H

#include <memory>

#include "description_maintainer.h"
#include "logic/bob.h"

namespace Widelands {

class EditorCategory;
class Editor_Game_Base;
class ResourceDescription;
class TerrainDescription;
struct Critter_Bob_Descr;
struct Immovable_Descr;

#define WORLD_NAME_LEN 128
#define WORLD_AUTHOR_LEN 128
#define WORLD_DESCR_LEN 1024

struct World_Descr_Header {
	char name  [WORLD_NAME_LEN];
	char author[WORLD_AUTHOR_LEN];
	char descr [WORLD_DESCR_LEN];
};

struct Resource_Descr : boost::noncopyable {
	Resource_Descr() : m_is_detectable(true), m_max_amount(0) {}
	~Resource_Descr() {}

	void parse(Section &, const std::string &);

	const std::string & name     () const {return m_name;}
	const std::string & descname() const {return m_descname;}

	bool is_detectable() const {return m_is_detectable;}
	int32_t get_max_amount() const {return m_max_amount;}

	const std::string & get_editor_pic(uint32_t amount) const;

private:
	struct Indicator {
		std::string bobname;
		int32_t         upperlimit;
	};
	struct Editor_Pic {
		std::string picname;
		int32_t     upperlimit;
	};

	bool                    m_is_detectable;
	int32_t                 m_max_amount;
	std::string             m_name;
	std::string             m_descname;
	std::vector<Editor_Pic> m_editor_pics;
};

struct Terrain_Descr : boost::noncopyable {
	friend struct World;

	Terrain_Descr
		(char const * directory, Section *, DescriptionMaintainer<Resource_Descr> *);
	~Terrain_Descr();

	void load_graphics();

	uint32_t         get_texture() const {return m_texture;}
	uint8_t        get_is     () const {return m_is;}
	const std::string & name() const {return m_name;}
	const std::string & descname() const {return m_descname;}
	int32_t resource_value(const Resource_Index resource) const {
		return
			resource == get_default_resources() or is_resource_valid(resource) ?
			(get_is() & TERRAIN_UNPASSABLE ? 8 : 1) : -1;
	}

	uint8_t get_num_valid_resources() const {
		return m_nr_valid_resources;
	}

	Resource_Index get_valid_resource(uint8_t index) const {
		return m_valid_resources[index];
	}

	bool is_resource_valid(const int32_t res) const {
		for (int32_t i = 0; i < m_nr_valid_resources; ++i)
			if (m_valid_resources[i] == res)
				return true;
		return false;
	}
	int8_t get_default_resources() const {return m_default_resources;}
	int32_t get_default_resources_amount() const {
		return m_default_amount;
	}
	int32_t dither_layer() const {return m_dither_layer;}

private:
	const std::string m_name;
	const std::string m_descname;
	std::string m_picnametempl;
	uint32_t    m_frametime;
	uint8_t   m_is;

	int32_t m_dither_layer;

	uint8_t         * m_valid_resources;
	uint8_t           m_nr_valid_resources;
	int8_t            m_default_resources;
	int32_t           m_default_amount;
	uint32_t          m_texture; //  renderer's texture
};

>>>>>>> MERGE-SOURCE
struct MapGenInfo;

// This is the in memory descriptions of the world and provides access to
// terrains, immovables and resources.
class World : boost::noncopyable {
public:
	World();
	~World();  // Defined in .cc because all forward declarations are known then.

	// TODO(sirver): Refactor these to only return the description_maintainer so that world
	// becomes a pure container.
	const DescriptionMaintainer<TerrainDescription>& terrains() const;
	TerrainDescription& terrain_descr(Terrain_Index i) const;
	TerrainDescription const* get_ter(char const* const name) const;

	int32_t get_bob(char const* const l) const;
	BobDescr const* get_bob_descr(uint16_t index) const;
	BobDescr const* get_bob_descr(const std::string& name) const;
	int32_t get_nr_bobs() const;

	const DescriptionMaintainer<Immovable_Descr>& immovables() const;
	int32_t get_immovable_index(char const* const l) const;
	int32_t get_nr_immovables() const;
	Immovable_Descr const* get_immovable_descr(int32_t index) const;

	int32_t get_resource(const char* const name) const;
	ResourceDescription const* get_resource(Resource_Index res) const;
	int32_t get_nr_resources() const;
	int32_t safe_resource_index(const char* const warename) const;


	// Add this new resource to the world description.
	void add_resource_type(const LuaTable& table);

	// Add this new terrain to the world description.
	void add_terrain_type(const LuaTable& table);

	// Add a new critter to the world description.
	void add_critter_type(const LuaTable& table);

	// Add a new immovable to the world description.
	void add_immovable_type(const LuaTable& table);

	// Add an editor categories for grouping items in the editor.
	void add_editor_terrain_category(const LuaTable& table);
	void add_editor_immovable_category(const LuaTable& table);

	// Access to the editor categories.
	const DescriptionMaintainer<EditorCategory>& editor_terrain_categories() const;
	const DescriptionMaintainer<EditorCategory>& editor_immovable_categories() const;

	const MapGenInfo& getMapGenInfo() const;

private:
	std::unique_ptr<DescriptionMaintainer<BobDescr>> bobs_;
	std::unique_ptr<DescriptionMaintainer<Immovable_Descr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<TerrainDescription>> terrains_;
	std::unique_ptr<DescriptionMaintainer<ResourceDescription>> resources_;
	std::unique_ptr<DescriptionMaintainer<EditorCategory>> editor_terrain_categories_;
	std::unique_ptr<DescriptionMaintainer<EditorCategory>> editor_immovable_categories_;
	std::unique_ptr<MapGenInfo> mapGenInfo_;
};
}  // namespace Widelands

#endif
