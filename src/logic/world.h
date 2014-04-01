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

#include "logic/bob.h"
#include "description_maintainer.h"
#include "logic/immovable.h"
#include "logic/widelands.h"
#include "logic/worlddata.h"

struct Section;

namespace Widelands {

class Editor_Game_Base;

#define WORLD_NAME_LEN 128
#define WORLD_AUTHOR_LEN 128
#define WORLD_DESCR_LEN 1024

struct World_Descr_Header {
	char name  [WORLD_NAME_LEN];
	char author[WORLD_AUTHOR_LEN];
	char descr [WORLD_DESCR_LEN];
};

struct Resource_Descr : boost::noncopyable {
	typedef Resource_Index Index;
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

	typedef Terrain_Index Index;
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

struct MapGenInfo;

/// Holds world and area specific information for the map generator.
/// Areas are: Water, Land, Wasteland and Mountains.
struct MapGenAreaInfo {
	static int split_string(std::vector<std::string> & strs, std::string & str);

	enum MapGenAreaType {
		atWater,
		atLand,
		atWasteland,
		atMountains
	};

	enum MapGenTerrainType {
		ttWaterOcean,
		ttWaterShelf,
		ttWaterShallow,

		ttLandCoast,
		ttLandLand,
		ttLandUpper,

		ttWastelandInner,
		ttWastelandOuter,

		ttMountainsFoot,
		ttMountainsMountain,
		ttMountainsSnow
	};

	void parseSection (World *, Section &, MapGenAreaType areaType);
	size_t getNumTerrains(MapGenTerrainType) const;
	Terrain_Index getTerrain(MapGenTerrainType terrType, uint32_t index) const;
	uint32_t getWeight() const {return m_weight;}

private:

	void readTerrains
		(std::vector<Terrain_Index> & list, Section &, char const * value_name);

	std::vector<Terrain_Index>  m_Terrains1; //  ocean, coast, inner or foot
	std::vector<Terrain_Index>  m_Terrains2; //  shelf, land, outer or mountain
	std::vector<Terrain_Index>  m_Terrains3; //  shallow, upper, snow

	uint32_t m_weight;

	MapGenAreaType m_areaType;

	World * m_world;

};

struct MapGenBobKind {

	void parseSection (Section &);

	size_t getNumImmovableBobs() const {return m_ImmovableBobs.size();}
	size_t getNumMoveableBobs() const {return m_MoveableBobs.size();}

	const std::string & getImmovableBob(size_t index) const {
		return m_ImmovableBobs[index];
	};
	const std::string & getMoveableBob(size_t index) const {
		return m_MoveableBobs[index];
	};

private:
	std::vector<std::string> m_ImmovableBobs;
	std::vector<std::string> m_MoveableBobs;

};

struct MapGenBobArea {

	void parseSection (Section &, MapGenInfo & mapGenInfo);

	uint32_t getWeight() const {return m_Weight;};
	const MapGenBobKind * getBobKind
		(MapGenAreaInfo::MapGenTerrainType terrType) const;

	uint8_t getImmovableDensity() const {return m_Immovable_Density;};
	uint8_t getMoveableDensity() const {return m_Moveable_Density;};

private:
	uint32_t        m_Weight;
	uint8_t         m_Immovable_Density; // In percent
	uint8_t         m_Moveable_Density;  // In percent
	const MapGenBobKind * m_LandCoastBobKind;
	const MapGenBobKind * m_LandInnerBobKind;
	const MapGenBobKind * m_LandUpperBobKind;
	const MapGenBobKind * m_WastelandInnerBobKind;
	const MapGenBobKind * m_WastelandOuterBobKind;
};

/** struct MapGenInfo
  *
  * This class holds world specific information for the map generator.
  * This info is usually read from the file "mapgeninfo" of a world.
  */
struct MapGenInfo {

	void parseProfile(World * world, Profile & profile);

	size_t getNumAreas(MapGenAreaInfo::MapGenAreaType areaType) const;
	const MapGenAreaInfo & getArea
		(MapGenAreaInfo::MapGenAreaType const areaType, uint32_t const index)
		const;
	const MapGenBobKind * getBobKind(const std::string & bobKindName) const;

	uint8_t getWaterOceanHeight  () const {return m_ocean_height;}
	uint8_t getWaterShelfHeight  () const {return m_shelf_height;}
	uint8_t getWaterShallowHeight() const {return m_shallow_height;}
	uint8_t getLandCoastHeight   () const {return m_coast_height;}
	uint8_t getLandUpperHeight   () const {return m_upperland_height;}
	uint8_t getMountainFootHeight() const {return m_mountainfoot_height;}
	uint8_t getMountainHeight    () const {return m_mountain_height;}
	uint8_t getSnowHeight        () const {return m_snow_height;}
	uint8_t getSummitHeight      () const {return m_summit_height;}

	uint32_t getSumLandWeight() const;

	size_t getNumBobAreas() const;
	const MapGenBobArea & getBobArea(size_t index) const;
	uint32_t getSumBobAreaWeight() const;

private:

	World * m_world;

	std::vector<MapGenAreaInfo> m_WaterAreas;
	std::vector<MapGenAreaInfo> m_LandAreas;
	std::vector<MapGenAreaInfo> m_WasteLandAreas;
	std::vector<MapGenAreaInfo> m_MountainAreas;

	std::vector<MapGenBobArea>           m_BobAreas;
	std::map<std::string, MapGenBobKind> m_BobKinds;

	uint8_t m_ocean_height;
	uint8_t m_shelf_height;
	uint8_t m_shallow_height;
	uint8_t m_coast_height;
	uint8_t m_upperland_height;
	uint8_t m_mountainfoot_height;
	uint8_t m_snow_height;
	uint8_t m_mountain_height;
	uint8_t m_summit_height;

	mutable int32_t  m_land_weight;
	mutable bool m_land_weight_valid;

	mutable int32_t m_sum_bob_area_weights;
	mutable bool m_sum_bob_area_weights_valid;

};

/** struct World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
struct World : boost::noncopyable {
	friend class Game;

	enum {
		OK = 0,
		ERR_WRONGVERSION
	};

	World(const std::string & name);

	static bool exists_world(std::string); ///  check if a world really exists
	static void get_all_worlds(std::vector<std::string> &);

	void load_graphics();

	const char * get_name  () const {return hd.name;}
	const char * get_author() const {return hd.author;}
	const char * get_descr () const {return hd.descr;}

	Terrain_Index index_of_terrain(char const * const name) const {
		return ters.get_index(name);
	}
	Terrain_Descr & terrain_descr(Terrain_Index const i) const {
		return *ters.get(i);
	}
	const Terrain_Descr & get_ter(Terrain_Index const i) const {
		assert(i < ters.get_nitems());
		return *ters.get(i);
	}
	Terrain_Descr const * get_ter(char const * const name) const {
		int32_t const i = ters.get_index(name);
		return i != -1 ? ters.get(i) : nullptr;
	}
	int32_t get_nr_terrains() const {return ters.get_nitems();}
	int32_t get_bob(char const * const l) const {return bobs.get_index(l);}
	BobDescr const * get_bob_descr(uint16_t const index) const {
		return bobs.get(index);
	}
	BobDescr const * get_bob_descr(const std::string & name) const {
		return bobs.exists(name.c_str());
	}
	int32_t get_nr_bobs() const {return bobs.get_nitems();}
	int32_t get_immovable_index(char const * const l)const {
		return immovables.get_index(l);
	}
	int32_t get_nr_immovables() const {return immovables.get_nitems();}
	Immovable_Descr const * get_immovable_descr(int32_t const index) const {
		return immovables.get(index);
	}

	int32_t get_resource(const char * const name) const {
		return m_resources.get_index(name);
	}
	Resource_Descr const * get_resource(Resource_Index const res) const {
		assert(res < m_resources.get_nitems());
		return m_resources.get(res);
	}
	int32_t get_nr_resources() const {return m_resources.get_nitems();}
	int32_t safe_resource_index(const char * const warename) const;
	const std::string & basedir() const {return m_basedir;}

	MapGenInfo & getMapGenInfo();

private:
	std::string m_basedir; //  base directory, where the main conf file resides
	World_Descr_Header                hd;

	DescriptionMaintainer<BobDescr>      bobs;
	DescriptionMaintainer<Immovable_Descr> immovables;
	DescriptionMaintainer<Terrain_Descr>   ters;
	DescriptionMaintainer<Resource_Descr>  m_resources;

	//  TODO: Should this be a description-maintainer?
	MapGenInfo m_mapGenInfo;

	void parse_root_conf(const std::string & name, Profile & root_conf);
	void parse_resources();
	void parse_terrains ();
	void parse_bobs     (std::string & directory, Profile & root_conf);
	void parse_mapgen   ();
};

}

#endif
