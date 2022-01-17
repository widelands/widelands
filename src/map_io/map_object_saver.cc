/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "map_io/map_object_saver.h"

#include "base/wexception.h"
#include "economy/ferry_fleet.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/ship_fleet.h"
#include "economy/ware_instance.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/battle.h"

namespace Widelands {

MapObjectSaver::MapObjectSaver()
   : nr_roads_(0),
     nr_waterways_(0),
     nr_flags_(0),
     nr_buildings_(0),
     nr_bobs_(0),
     nr_wares_(0),
     nr_immovables_(0),
     nr_battles_(0),
     nr_ship_fleets_(0),
     nr_ferry_fleets_(0),
     nr_portdocks_(0),
     lastserial_(0) {
}

/**
 * Return a pointer to the record for the given object.
 * Create a record if that hasn't been done yet.
 */
MapObjectSaver::MapObjectRec& MapObjectSaver::get_object_record(const MapObject& obj) {
	MapObjectRecordMap::iterator it = objects_.find(&obj);

	if (it != objects_.end()) {
		return it->second;
	}

	MapObjectRec rec;
#ifndef NDEBUG
	rec.description = to_string(obj.descr().type());
	rec.description += " (";
	rec.description += std::to_string(obj.serial());
	rec.description += ')';
#endif
	rec.fileserial = ++lastserial_;
	rec.registered = false;
	rec.saved = false;
	return objects_.insert(std::make_pair(&obj, rec)).first->second;
}

/**
 * Returns true if this object has already been registered.
 * \deprecated since get_object_file_index supports unregistered objects now
 */
bool MapObjectSaver::is_object_known(const MapObject& obj) const {
	MapObjectRecordMap::const_iterator it = objects_.find(&obj);

	if (it == objects_.end()) {
		return false;
	}

	return it->second.registered;
}

bool MapObjectSaver::is_object_saved(const MapObject& obj) {
	return get_object_record(obj).saved;
}

/*
 * Registers this object as a new one
 */
Serial MapObjectSaver::register_object(const MapObject& obj) {
	MapObjectRec& rec = get_object_record(obj);

	assert(!rec.registered);

	switch (obj.descr().type()) {
	case MapObjectType::FLAG:
		++nr_flags_;
		break;
	case MapObjectType::ROAD:
		++nr_roads_;
		break;
	case MapObjectType::WATERWAY:
		++nr_waterways_;
		break;
	case MapObjectType::BATTLE:
		++nr_battles_;
		break;
	case MapObjectType::SHIP_FLEET:
		++nr_ship_fleets_;
		break;
	case MapObjectType::FERRY_FLEET:
		++nr_ferry_fleets_;
		break;
	case MapObjectType::PORTDOCK:
		++nr_portdocks_;
		break;
	case MapObjectType::WARE:
		++nr_wares_;
		break;
	default:
		if (obj.descr().type() >= MapObjectType::BUILDING) {
			++nr_buildings_;
		} else if (obj.descr().type() >= MapObjectType::IMMOVABLE) {
			++nr_immovables_;
		} else if (obj.descr().type() >= MapObjectType::BOB) {
			++nr_bobs_;
		} else {
			throw wexception(
			   "MapObjectSaver: Unknown MapObjectType %u", static_cast<unsigned>(obj.descr().type()));
		}
		break;
	}

	rec.registered = true;
	return rec.fileserial;
}

/**
 * Returns the file index for this map object. This is used on load
 * to regenerate the dependencies between the objects.
 */
uint32_t MapObjectSaver::get_object_file_index(const MapObject& obj) {
	return get_object_record(obj).fileserial;
}

/**
 * Returns the file index of the given object, or zero for null pointers.
 */
uint32_t MapObjectSaver::get_object_file_index_or_zero(const MapObject* obj) {
	if (obj) {
		return get_object_file_index(*obj);
	}
	return 0;
}

/*
 * mark this object as saved
 */
void MapObjectSaver::mark_object_as_saved(const MapObject& obj) {
	MapObjectRec& rec = get_object_record(obj);
	assert(rec.registered);
	rec.saved = true;
}

#ifndef NDEBUG
/*
 * Return the number of unsaved objects
 */
void MapObjectSaver::detect_unsaved_objects() const {
	for (const auto& temp_map : objects_) {
		if (!temp_map.second.saved) {
			throw wexception("%s has not been saved", temp_map.second.description.c_str());
		}
	}
}
#endif
}  // namespace Widelands
