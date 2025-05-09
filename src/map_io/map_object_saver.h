/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_OBJECT_SAVER_H
#define WL_MAP_IO_MAP_OBJECT_SAVER_H

#include "graphic/playercolor.h"
#include "logic/widelands.h"
#include "map_io/map_message_saver.h"

namespace Widelands {

class MapObject;

/*
 * This class helps to
 *   - keep track of map objects on the map
 *   - translate MapObject* Pointer into the index used in the saved file
 */
struct MapObjectSaver {
	MapObjectSaver() = default;

	[[nodiscard]] bool is_object_known(const MapObject&) const;
	Serial register_object(const MapObject&);

	uint32_t get_object_file_index(const MapObject&);
	uint32_t get_object_file_index_or_zero(MapObject const*);

	void mark_object_as_saved(const MapObject&);

// Information functions
#ifndef NDEBUG
	void detect_unsaved_objects() const;
#endif
	[[nodiscard]] uint32_t get_nr_roads() const {
		return nr_roads_;
	}
	[[nodiscard]] uint32_t get_nr_waterways() const {
		return nr_waterways_;
	}
	[[nodiscard]] uint32_t get_nr_flags() const {
		return nr_flags_;
	}
	[[nodiscard]] uint32_t get_nr_buildings() const {
		return nr_buildings_;
	}
	[[nodiscard]] uint32_t get_nr_wares() const {
		return nr_wares_;
	}
	[[nodiscard]] uint32_t get_nr_bobs() const {
		return nr_bobs_;
	}
	[[nodiscard]] uint32_t get_nr_immovables() const {
		return nr_immovables_;
	}
	[[nodiscard]] uint32_t get_nr_battles() const {
		return nr_battles_;
	}

	bool is_object_saved(const MapObject&);

	/// \note Indexed by player number - 1.
	MapMessageSaver message_savers[kMaxPlayers];

private:
	struct MapObjectRec {
#ifndef NDEBUG
		std::string description;
#endif
		uint32_t fileserial;
		bool registered;
		bool saved;
	};
	using MapObjectRecordMap = std::map<const MapObject*, MapObjectRec>;

	MapObjectRec& get_object_record(const MapObject&);

	MapObjectRecordMap objects_;
	uint32_t nr_roads_{0U};
	uint32_t nr_waterways_{0U};
	uint32_t nr_flags_{0U};
	uint32_t nr_buildings_{0U};
	uint32_t nr_bobs_{0U};
	uint32_t nr_wares_{0U};
	uint32_t nr_immovables_{0U};
	uint32_t nr_battles_{0U};
	uint32_t nr_ship_fleets_{0U};
	uint32_t nr_ferry_fleets_{0U};
	uint32_t nr_portdocks_{0U};
	uint32_t lastserial_{0U};
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_OBJECT_SAVER_H
