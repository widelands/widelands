/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#ifndef WORLD_H
#define WORLD_H

#include "logic/bob.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "worlddata.h"

#include "widelands.h"

class Section;

namespace Widelands {

struct Editor_Game_Base;

#define WORLD_NAME_LEN 128
#define WORLD_AUTHOR_LEN 128
#define WORLD_DESCR_LEN 1024

struct World_Descr_Header {
	char name  [WORLD_NAME_LEN];
	char author[WORLD_AUTHOR_LEN];
	char descr [WORLD_DESCR_LEN];
};

struct Resource_Descr {
	typedef Resource_Index Index;
	Resource_Descr() {}
	~Resource_Descr() {}

	void parse(Section &, std::string const &);

	const std::string & name     () const throw () {return m_name;}
	std::string const & descname() const throw () {return m_descname;}

	bool is_detectable() const throw () {return m_is_detectable;}
	int32_t get_max_amount() const throw () {return m_max_amount;}

	std::string const & get_editor_pic(uint32_t amount) const;

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

	Resource_Descr & operator= (Resource_Descr const &);
	explicit Resource_Descr    (Resource_Descr const &);
};

struct Terrain_Descr {
	friend struct World;

	typedef Terrain_Index Index;
	Terrain_Descr
		(char const * directory, Section *, Descr_Maintainer<Resource_Descr> *);
	~Terrain_Descr();

	void load_graphics();

	uint32_t         get_texture() const throw () {return m_texture;}
	uint8_t        get_is     () const throw () {return m_is;}
	const std::string & name() const throw () {return m_name;}
	int32_t resource_value(const Resource_Index resource) const throw () {
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

	bool is_resource_valid(const int32_t res) const throw () {
		for (int32_t i = 0; i < m_nr_valid_resources; ++i)
			if (m_valid_resources[i] == res)
				return true;
		return false;
	}
	int8_t get_default_resources() const {return m_default_resources;}
	int32_t get_default_resources_amount() const throw () {
		return m_default_amount;
	}

private:
	const std::string m_name;
	char  * m_picnametempl;
	uint32_t    m_frametime;
	uint8_t   m_is;

	uint8_t         * m_valid_resources;
	uint8_t           m_nr_valid_resources;
	int8_t            m_default_resources;
	int32_t           m_default_amount;
	uint32_t          m_texture; //  renderer's texture

	Terrain_Descr & operator= (Terrain_Descr const &);
	explicit Terrain_Descr    (Terrain_Descr const &);
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

	std::string const & getImmovableBob(size_t index) const {
		return m_ImmovableBobs[index];
	};
	std::string const & getMoveableBob(size_t index) const {
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

/** class MapGenInfo
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
	MapGenBobArea const & getBobArea(size_t index) const;
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

/** class World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
struct World {
	friend struct Game;

	enum {
		OK = 0,
		ERR_WRONGVERSION
	};

	World(std::string const & name);

	static bool exists_world(std::string); ///  check if a world really exists
	static void get_all_worlds(std::vector<std::string> &);

	void load_graphics();

	const char * get_name  () const throw () {return hd.name;}
	const char * get_author() const throw () {return hd.author;}
	const char * get_descr () const throw () {return hd.descr;}

	Terrain_Index index_of_terrain(char const * const name) const {
		return ters.get_index(name);
	}
	Terrain_Descr & terrain_descr(Terrain_Index const i) const {
		return *ters.get(i);
	}
	Terrain_Descr const & get_ter(Terrain_Index const i) const {
		assert(i < ters.get_nitems());
		return *ters.get(i);
	}
	Terrain_Descr const * get_ter(char const * const name) const {
		int32_t const i = ters.get_index(name);
		return i != -1 ? ters.get(i) : 0;
	}
	int32_t get_nr_terrains() const {return ters.get_nitems();}
	int32_t get_bob(char const * const l) const {return bobs.get_index(l);}
	Bob::Descr const * get_bob_descr(uint16_t const index) const {
		return bobs.get(index);
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
	std::string const & basedir() const {return m_basedir;}

	MapGenInfo & getMapGenInfo();

private:
	std::string m_basedir; //  base directory, where the main conf file resides
	World_Descr_Header                hd;

	Descr_Maintainer<Bob::Descr>      bobs;
	Descr_Maintainer<Immovable_Descr> immovables;
	Descr_Maintainer<Terrain_Descr>   ters;
	Descr_Maintainer<Resource_Descr>  m_resources;

	//  TODO: Should this be a description-maintainer?
	MapGenInfo m_mapGenInfo;

	void parse_root_conf(std::string const & name, Profile & root_conf);
	void parse_resources();
	void parse_terrains ();
	void parse_bobs     (std::string & directory, Profile & root_conf);
	void parse_mapgen   ();

	World & operator= (World const &);
	explicit World    (World const &);
};

}

#endif
