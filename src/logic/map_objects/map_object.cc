/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/map_objects/map_object.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/queue_cmd_ids.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

CmdDestroyMapObject::CmdDestroyMapObject(uint32_t const t, MapObject& o)
   : GameLogicCommand(t), obj_serial(o.serial()) {
}

void CmdDestroyMapObject::execute(Game& game) {
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* obj = game.objects().get_object(obj_serial))
		obj->destroy(game);
}

constexpr uint16_t kCurrentPacketVersionDestroyMapObject = 1;

void CmdDestroyMapObject::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDestroyMapObject) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const serial = fr.unsigned_32())
				try {
					obj_serial = mol.get<MapObject>(serial).serial();
				} catch (const WException& e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			else
				obj_serial = 0;
		} else {
			throw UnhandledVersionError(
			   "CmdDestroyMapObject", packet_version, kCurrentPacketVersionDestroyMapObject);
		}
	} catch (const WException& e) {
		throw GameDataError("destroy map object: %s", e.what());
	}
}
void CmdDestroyMapObject::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDestroyMapObject);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));
}

CmdAct::CmdAct(uint32_t const t, MapObject& o, int32_t const a)
   : GameLogicCommand(t), obj_serial(o.serial()), arg(a) {
}

void CmdAct::execute(Game& game) {
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* const obj = game.objects().get_object(obj_serial))
		obj->act(game, arg);
	// the object must queue the next CMD_ACT itself if necessary
}

constexpr uint16_t kCurrentPacketVersionCmdAct = 1;

void CmdAct::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdAct) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const object_serial = fr.unsigned_32())
				try {
					obj_serial = mol.get<MapObject>(object_serial).serial();
				} catch (const WException& e) {
					throw GameDataError("object %u: %s", object_serial, e.what());
				}
			else
				obj_serial = 0;
			arg = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdAct", packet_version, kCurrentPacketVersionCmdAct);
		}
	} catch (const WException& e) {
		throw wexception("act: %s", e.what());
	}
}
void CmdAct::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdAct);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(obj_serial)));

	// And arg
	fw.unsigned_32(arg);
}

ObjectManager::~ObjectManager() {
	// better not throw an exception in a destructor...
	if (!objects_.empty())
		log("ObjectManager: ouch! remaining objects\n");

	log("lastserial: %i\n", lastserial_);
}

/**
 * Clear all objects
 */
void ObjectManager::cleanup(EditorGameBase& egbase) {
	while (!objects_.empty()) {
		MapObjectMap::iterator it = objects_.begin();
		it->second->remove(egbase);
	}
	lastserial_ = 0;
}

/**
 * Insert the given MapObject into the object manager
 */
void ObjectManager::insert(MapObject* obj) {
	++lastserial_;
	assert(lastserial_);
	obj->serial_ = lastserial_;
	objects_[lastserial_] = obj;
}

/**
 * Remove the MapObject from the manager
 */
void ObjectManager::remove(MapObject& obj) {
	objects_.erase(obj.serial_);
}

/*
 * Return the list of all serials currently in use
 */
std::vector<Serial> ObjectManager::all_object_serials_ordered() const {
	std::vector<Serial> rv;

	for (const auto& o : objects_) {
		rv.push_back(o.first);
	}

	std::sort(rv.begin(), rv.end());

	return rv;
}

MapObject* ObjectPointer::get(const EditorGameBase& egbase) {
	if (!serial_)
		return nullptr;
	MapObject* const obj = egbase.objects().get_object(serial_);
	if (!obj)
		serial_ = 0;
	return obj;
}

// This version also returns a pointer to a non-const object,
// because it is logically the pointer that is const, not the object
// that is pointed to.
// That is, a 'const ObjectPointer' behaves like a 'ObjectPointer * const'.
MapObject* ObjectPointer::get(const EditorGameBase& egbase) const {
	return serial_ ? egbase.objects().get_object(serial_) : nullptr;
}

/*
==============================================================================

MapObjectDescr IMPLEMENTATION

==============================================================================
*/
MapObjectDescr::MapObjectDescr(const MapObjectType init_type,
                               const std::string& init_name,
                               const std::string& init_descname)
   : type_(init_type),
     name_(init_name),
     descname_(init_descname),
     representative_image_filename_(""),
     icon_filename_("") {
}
MapObjectDescr::MapObjectDescr(const MapObjectType init_type,
                               const std::string& init_name,
                               const std::string& init_descname,
                               const LuaTable& table)
   : MapObjectDescr(init_type, init_name, init_descname) {
	if (table.has_key("animations")) {
		std::unique_ptr<LuaTable> anims(table.get_table("animations"));
		for (const std::string& animation : anims->keys<std::string>()) {
			add_animation(animation, g_gr->animations().load(*anims->get_table(animation)));
		}
		if (!is_animation_known("idle")) {
			throw GameDataError(
			   "Map object %s has animations but no idle animation", init_name.c_str());
		}
		representative_image_filename_ =
		   g_gr->animations().get_animation(get_animation("idle")).representative_image_filename();
	}
	if (table.has_key("icon")) {
		icon_filename_ = table.get_string("icon");
		if (icon_filename_.empty()) {
			throw GameDataError("Map object %s has a menu icon, but it is empty", init_name.c_str());
		}
	}
	// TODO(GunChleoc): We can't scale down images in the font renderer yet, so we need an extra
	// representative image if the animation has high resolution.
	if (table.has_key("representative_image")) {
		representative_image_filename_ = table.get_string("representative_image");
	}
}
MapObjectDescr::~MapObjectDescr() {
	anims_.clear();
}

uint32_t MapObjectDescr::dyn_attribhigh_ = MapObject::HIGHEST_FIXED_ATTRIBUTE;
MapObjectDescr::AttribMap MapObjectDescr::dyn_attribs_;

bool MapObjectDescr::is_animation_known(const std::string& animname) const {
	return (anims_.count(animname) == 1);
}

/**
 * Add this animation for this map object under this name
 */
void MapObjectDescr::add_animation(const std::string& animname, uint32_t const anim) {
	if (is_animation_known(animname)) {
		throw GameDataError("Tried to add already existing animation \"%s\"", animname.c_str());
	} else {
		anims_.insert(std::pair<std::string, uint32_t>(animname, anim));
	}
}

void MapObjectDescr::add_directional_animation(DirAnimations* anims, const std::string& prefix) {
	static char const* const dirstrings[6] = {"ne", "e", "se", "sw", "w", "nw"};
	for (int32_t dir = 1; dir <= 6; ++dir) {
		const std::string anim_name = prefix + std::string("_") + dirstrings[dir - 1];
		try {
			anims->set_animation(dir, get_animation(anim_name));
		} catch (const MapObjectDescr::AnimationNonexistent&) {
			throw GameDataError("MO: no directional animation '%s'", anim_name.c_str());
		}
	}
}

std::string MapObjectDescr::get_animation_name(uint32_t const anim) const {

	for (const auto& temp_anim : anims_) {
		if (temp_anim.second == anim) {
			return temp_anim.first;
		}
	}
	NEVER_HERE();
}

const Image* MapObjectDescr::representative_image(const RGBColor* player_color) const {
	if (is_animation_known("idle")) {
		return g_gr->animations().get_representative_image(get_animation("idle"), player_color);
	}
	return nullptr;
}
const std::string& MapObjectDescr::representative_image_filename() const {
	return representative_image_filename_;
}

const Image* MapObjectDescr::icon() const {
	if (!icon_filename_.empty()) {
		return g_gr->images().get(icon_filename_);
	}
	return nullptr;
}
const std::string& MapObjectDescr::icon_filename() const {
	return icon_filename_;
}

/**
 * Search for the attribute in the attribute list
 */
bool MapObjectDescr::has_attribute(uint32_t const attr) const {
	for (const uint32_t& attrib : attributes_) {
		if (attrib == attr) {
			return true;
		}
	}
	return false;
}

/**
 * Add an attribute to the attribute list if it's not already there
 */
void MapObjectDescr::add_attribute(uint32_t const attr) {
	if (!has_attribute(attr))
		attributes_.push_back(attr);
}

void MapObjectDescr::add_attributes(const std::vector<std::string>& attributes,
                                    const std::set<uint32_t>& allowed_special) {
	for (const std::string& attribute : attributes) {
		uint32_t const attrib = get_attribute_id(attribute, true);
		if (attrib < MapObject::HIGHEST_FIXED_ATTRIBUTE) {
			if (!allowed_special.count(attrib)) {
				throw GameDataError("bad attribute \"%s\"", attribute.c_str());
			}
		}
		add_attribute(attrib);
	}
}

/**
 * Lookup an attribute by name. If the attribute name hasn't been encountered
 * before and add_if_not_exists = true, we add it to the map. Else, throws exception.
 */
uint32_t MapObjectDescr::get_attribute_id(const std::string& name, bool add_if_not_exists) {
	AttribMap::iterator it = dyn_attribs_.find(name);

	if (it != dyn_attribs_.end()) {
		return it->second;
	}

	if (name == "worker") {
		return MapObject::WORKER;
	} else if (name == "resi") {
		return MapObject::RESI;
	}

	if (!add_if_not_exists) {
		throw GameDataError("get_attribute_id: attribute '%s' not found!\n", name.c_str());
	} else {
		++dyn_attribhigh_;
		dyn_attribs_[name] = dyn_attribhigh_;
	}
	assert(dyn_attribhigh_ != 0);  // wrap around seems *highly* unlikely ;)

	return dyn_attribhigh_;
}


/*
==============================================================================

MapObject IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize a map object
 */
MapObject::MapObject(const MapObjectDescr* const the_descr)
   : descr_(the_descr), serial_(0), logsink_(nullptr), owner_(nullptr), reserved_by_worker_(false) {
}

/**
 * Call this function if you want to remove the object immediately, without
 * any effects.
 */
void MapObject::remove(EditorGameBase& egbase) {
	removed(serial_);  // Signal call
	cleanup(egbase);
	delete this;
}

/**
 * Destroy the object immediately. Unlike remove(), special actions may be
 * performed:
 * \li Create a decaying skeleton (humans)
 * \li Create a burning fire (buildings)
 * \li ...
 *
 * \warning This function will immediately delete the memory allocated for
 * the object. Therefore, it may be safer to call schedule_destroy()
 * instead.
 */
void MapObject::destroy(EditorGameBase& egbase) {
	remove(egbase);
}

/**
 * Schedule the object for immediate destruction.
 * This can be used to safely destroy the object from within an act function.
 */
void MapObject::schedule_destroy(Game& game) {
	game.cmdqueue().enqueue(new CmdDestroyMapObject(game.get_gametime(), *this));
}

/**
 * Initialize the object by adding it to the object manager.
 *
 * \warning Make sure you call this from derived classes!
 */
void MapObject::init(EditorGameBase& egbase) {
	egbase.objects().insert(this);
}

/**
 * \warning Make sure you call this from derived classes!
 */
void MapObject::cleanup(EditorGameBase& egbase) {
	egbase.objects().remove(*this);
}

void MapObject::do_draw_info(const TextToDraw& draw_text,
                             const std::string& census,
                             const std::string& statictics,
                             const Vector2f& field_on_dst,
                             float scale,
                             RenderTarget* dst) const {
	if (draw_text == TextToDraw::kNone) {
		return;
	}

	// Rendering text is expensive, so let's just do it for only a few sizes.
	scale = std::round(scale);
	if (scale == 0.f) {
		return;
	}
	const int font_size = scale * UI_FONT_SIZE_SMALL;

	// We always render this so we can have a stable position for the statistics string.
	const Image* rendered_census_info =
	   UI::g_fh1->render(as_condensed(census, UI::Align::kCenter, font_size), 120);

	// Rounding guarantees that text aligns with pixels to avoid subsampling.
	const Vector2f census_pos = round(field_on_dst - Vector2f(0, 48) * scale).cast<float>();
	if (draw_text & TextToDraw::kCensus) {
		dst->blit(census_pos, rendered_census_info, BlendMode::UseAlpha, UI::Align::kCenter);
	}

	if (draw_text & TextToDraw::kStatistics && !statictics.empty()) {
		const Vector2f statistics_pos =
		   round(census_pos + Vector2f(0, rendered_census_info->height() / 2.f + 10 * scale))
		      .cast<float>();
		dst->blit(statistics_pos,
		          UI::g_fh1->render(as_condensed(statictics, UI::Align::kLeft, font_size)),
		          BlendMode::UseAlpha, UI::Align::kCenter);
	}
}

const Image* MapObject::representative_image() const {
	return descr().representative_image();
}

/**
 * Default implementation
 */
int32_t MapObject::get_training_attribute(TrainingAttribute) const {
	return -1;
}

/**
 * Queue a CMD_ACT tdelta milliseconds from now, using the given data.
 *
 * \return The absolute gametime at which the CMD_ACT will occur.
 */
uint32_t MapObject::schedule_act(Game& game, uint32_t const tdelta, uint32_t const data) {
	if (tdelta < endless()) {
		uint32_t const time = game.get_gametime() + tdelta;

		game.cmdqueue().enqueue(new CmdAct(time, *this, data));

		return time;
	} else
		return never();
}

/**
 * Called when a CMD_ACT triggers.
 */
void MapObject::act(Game&, uint32_t) {
}

/**
 * Set the logsink. This should only be used by the debugging facilities.
 */
void MapObject::set_logsink(LogSink* const sink) {
	logsink_ = sink;
}

void MapObject::log_general_info(const EditorGameBase&) {
}

/**
 * Prints a log message prepended by the object's serial number.
 */
void MapObject::molog(char const* fmt, ...) const {
	if (!g_verbose && !logsink_)
		return;

	va_list va;
	char buffer[2048];

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (logsink_)
		logsink_->log(buffer);

	log("MO(%u,%s): %s", serial_, descr().name().c_str(), buffer);
}

bool MapObject::is_reserved_by_worker() const {
	return reserved_by_worker_;
}

void MapObject::set_reserved_by_worker(bool reserve) {
	reserved_by_worker_ = reserve;
}

constexpr uint8_t kCurrentPacketVersionMapObject = 2;

/**
 * Load the entire data package from the given file.
 * This will be called from the MapObject's derived class static load function.
 *
 * Derived functions must read all data into member variables, even if
 * it is used only later in \ref load_pointers or \ref load_finish .
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load(FileRead& fr) {
	try {
		uint8_t const header = fr.unsigned_8();
		if (header != HeaderMapObject)
			throw wexception("header is %u, expected %u", header, HeaderMapObject);

		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version <= 0 || packet_version > kCurrentPacketVersionMapObject) {
			throw UnhandledVersionError("MapObject", packet_version, kCurrentPacketVersionMapObject);
		}

		Serial const serial = fr.unsigned_32();
		try {
			mol().register_object<MapObject>(serial, *get_object());
		} catch (const WException& e) {
			throw wexception("%u: %s", serial, e.what());
		}

		if (packet_version == kCurrentPacketVersionMapObject) {
			get_object()->reserved_by_worker_ = fr.unsigned_8();
		}
	} catch (const WException& e) {
		throw wexception("map object: %s", e.what());
	}

	egbase().objects().insert(get_object());
}

/**
 * This will be called after all instances have been loaded.
 *
 * This is where pointers to other instances should be established, possibly
 * using data that was previously stored in a member variable by \ref load .
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load_pointers() {
}

/**
 * This will be called after all instances have been load_pointer'ed.
 *
 * This is where dependent data (e.g. ware requests) should be checked and
 * configured.
 *
 * Derived functions must call ancestor's function in the appropriate place.
 */
void MapObject::Loader::load_finish() {
}

/**
 * Save the MapObject to the given file.
 */
void MapObject::save(EditorGameBase&, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderMapObject);
	fw.unsigned_8(kCurrentPacketVersionMapObject);

	fw.unsigned_32(mos.get_object_file_index(*this));
	fw.unsigned_8(reserved_by_worker_);
}

std::string to_string(const MapObjectType type) {
	switch (type) {
	case MapObjectType::BOB:
		return "bob";
	case MapObjectType::CRITTER:
		return "critter";
	case MapObjectType::SHIP:
		return "ship";
	case MapObjectType::WORKER:
		return "worker";
	case MapObjectType::CARRIER:
		return "carrier";
	case MapObjectType::SOLDIER:
		return "soldier";
	case MapObjectType::WARE:
		return "ware";
	case MapObjectType::BATTLE:
		return "battle";
	case MapObjectType::FLEET:
		return "fleet";
	case MapObjectType::IMMOVABLE:
		return "immovable";
	case MapObjectType::FLAG:
		return "flag";
	case MapObjectType::ROAD:
		return "road";
	case MapObjectType::PORTDOCK:
		return "portdock";
	case MapObjectType::BUILDING:
		return "building";
	case MapObjectType::CONSTRUCTIONSITE:
		return "constructionsite";
	case MapObjectType::DISMANTLESITE:
		return "dismantlesite";
	case MapObjectType::WAREHOUSE:
		return "warehouse";
	case MapObjectType::PRODUCTIONSITE:
		return "productionsite";
	case MapObjectType::MILITARYSITE:
		return "militarysite";
	case MapObjectType::TRAININGSITE:
		return "trainingsite";
	case MapObjectType::MAPOBJECT:
		throw wexception("Unknown MapObjectType %d.", static_cast<int>(type));
	}
	NEVER_HERE();
}
}
