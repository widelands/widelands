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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/map_object.h"

#include <algorithm>
#include <cstdarg>
#include <memory>

#include "base/log.h"
#include "base/multithreading.h"
#include "base/wexception.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace {
char const* const animation_direction_names[6] = {"_ne", "_e", "_se", "_sw", "_w", "_nw"};
}  // namespace

namespace Widelands {

CmdDestroyMapObject::CmdDestroyMapObject(const Time& t, MapObject& o)
   : GameLogicCommand(t), obj_serial(o.serial()) {
}

void CmdDestroyMapObject::execute(Game& game) {
	game.syncstream().unsigned_8(SyncEntry::kDestroyObject);
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* obj = game.objects().get_object(obj_serial)) {
		obj->destroy(game);
	}
}

constexpr uint16_t kCurrentPacketVersionDestroyMapObject = 1;

void CmdDestroyMapObject::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDestroyMapObject) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const serial = fr.unsigned_32()) {
				try {
					obj_serial = mol.get<MapObject>(serial).serial();
				} catch (const WException& e) {
					throw GameDataError("%u: %s", serial, e.what());
				}
			} else {
				obj_serial = 0;
			}
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

CmdAct::CmdAct(const Time& t, MapObject& o, int32_t const a)
   : GameLogicCommand(t), obj_serial(o.serial()), arg(a) {
}

void CmdAct::execute(Game& game) {
	game.syncstream().unsigned_8(SyncEntry::kCmdAct);
	game.syncstream().unsigned_32(obj_serial);

	if (MapObject* const obj = game.objects().get_object(obj_serial)) {
		game.syncstream().unsigned_8(static_cast<uint8_t>(obj->descr().type()));
		obj->act(game, arg);
	} else {
		game.syncstream().unsigned_8(static_cast<uint8_t>(MapObjectType::MAPOBJECT));
	}
	// the object must queue the next CMD_ACT itself if necessary
}

constexpr uint16_t kCurrentPacketVersionCmdAct = 1;

void CmdAct::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdAct) {
			GameLogicCommand::read(fr, egbase, mol);
			if (Serial const object_serial = fr.unsigned_32()) {
				try {
					obj_serial = mol.get<MapObject>(object_serial).serial();
				} catch (const WException& e) {
					throw GameDataError("object %u: %s", object_serial, e.what());
				}
			} else {
				obj_serial = 0;
			}
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
	if (!objects_.empty()) {
		log_warn("ObjectManager: ouch! remaining objects\n");
	}

	verb_log_info("lastserial: %i\n", lastserial_);
}

/**
 * Clear all objects
 */
void ObjectManager::cleanup(EditorGameBase& egbase) {
	is_cleaning_up_ = true;

	// If all wares (read: flags) of an economy are gone, but some workers remain,
	// the economy is destroyed before workers detach. This can cause segfault.
	// Destruction happens in correct order after this dirty quickie.
	// Run at the end of game, algorithmic efficiency may be what it is.
	const static std::vector<MapObjectType> killusfirst{
	   MapObjectType::WARE,        MapObjectType::WATERWAY, MapObjectType::FERRY,
	   MapObjectType::FERRY_FLEET, MapObjectType::SHIP,     MapObjectType::SHIP_FLEET,
	   MapObjectType::PORTDOCK,    MapObjectType::WORKER,   MapObjectType::CARRIER,
	   MapObjectType::SOLDIER,
	};
	for (auto moi : killusfirst) {
		while (!objects_.empty()) {
			MapObjectMap::iterator it = objects_.begin();
			while (it != objects_.end() && (moi) != it->second->descr_->type()) {
				++it;
			}
			if (it == objects_.end()) {
				break;
			}
			it->second->remove(egbase);
		}
	}
	while (!objects_.empty()) {
		MapObjectMap::iterator it = objects_.begin();
		it->second->remove(egbase);
	}

	lastserial_ = 0;
	is_cleaning_up_ = false;
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
	if (serial_ == 0u) {
		return nullptr;
	}
	MapObject* const obj = egbase.objects().get_object(serial_);
	if (obj == nullptr) {
		serial_ = 0;
	}
	return obj;
}

// This version also returns a pointer to a non-const object,
// because it is logically the pointer that is const, not the object
// that is pointed to.
// That is, a 'const ObjectPointer' behaves like a 'ObjectPointer * const'.
MapObject* ObjectPointer::get(const EditorGameBase& egbase) const {
	return serial_ != 0u ? egbase.objects().get_object(serial_) : nullptr;
}

/*
==============================================================================

MapObjectDescr IMPLEMENTATION

==============================================================================
*/
MapObjectDescr::MapObjectDescr(const MapObjectType init_type,
                               const std::string& init_name,
                               const std::string& init_descname)
   : type_(init_type), name_(init_name), descname_(init_descname) {
}
MapObjectDescr::MapObjectDescr(const MapObjectType init_type,
                               const std::string& init_name,
                               const std::string& init_descname,
                               const LuaTable& table)
   : MapObjectDescr(init_type, init_name, init_descname) {
	if (table.has_key("helptext_script")) {
		// TODO(GunChleoc): Compatibility - remove after v1.0
		log_warn("Helptexts script for %s is obsolete - please move strings to "
		         "tribes/initializations/<tribename>/units.lua",
		         name().c_str());
	}

	bool has_animations = false;
	// TODO(GunChleoc): When all animations have been converted, require that animation_directory is
	// not empty if the map object has animations.
	const std::string animation_directory(
	   table.has_key("animation_directory") ? table.get_string("animation_directory") : "");
	if (table.has_key("animations")) {
		has_animations = true;
		add_animations(*table.get_table("animations"), animation_directory, Animation::Type::kFiles);
	}
	if (table.has_key("spritesheets")) {
		has_animations = true;
		add_animations(
		   *table.get_table("spritesheets"), animation_directory, Animation::Type::kSpritesheet);
	}
	if (has_animations) {
		if (!is_animation_known("idle")) {
			throw GameDataError(
			   "Map object %s has animations but no idle animation", init_name.c_str());
		}
		assert(g_animation_manager->get_representative_image(name())->width() > 0);
	}
	if (table.has_key("icon")) {
		icon_filename_ = table.get_string("icon");
		if (icon_filename_.empty()) {
			throw GameDataError("Map object %s has a menu icon, but it is empty", init_name.c_str());
		}
	}
	check_representative_image();

	// TODO(GunChleoc): Compatibility, remove after v1.0
	if (table.has_key("attributes")) {
		throw GameDataError("Attributes need to be defined in 'register.lua' now");
	}
}
MapObjectDescr::~MapObjectDescr() {
	anims_.clear();
}

std::map<std::string, MapObjectDescr::AttributeIndex> MapObjectDescr::attribute_names_;

bool MapObjectDescr::is_animation_known(const std::string& animname) const {
	return (anims_.count(animname) == 1);
}

/**
 * Add all animations for this map object
 */
void MapObjectDescr::add_animations(const LuaTable& table,
                                    const std::string& animation_directory,
                                    Animation::Type anim_type) {
	for (const std::string& animname : table.keys<std::string>()) {
		try {
			std::unique_ptr<LuaTable> anim = table.get_table(animname);
			// TODO(GunChleoc): Maybe remove basename after conversion has been completed
			const std::string basename =
			   anim->has_key<std::string>("basename") ? anim->get_string("basename") : animname;
			const bool is_directional =
			   anim->has_key<std::string>("directional") ? anim->get_bool("directional") : false;
			if (is_directional) {
				std::set<float> available_scales;
				for (int dir = 0; dir < 6; ++dir) {
					const std::string directional_animname = animname + animation_direction_names[dir];
					if (is_animation_known(directional_animname)) {
						throw GameDataError("Tried to add already existing directional animation '%s\'",
						                    directional_animname.c_str());
					}
					const std::string directional_basename = basename + animation_direction_names[dir];
					uint32_t anim_id = 0;
					try {
						anim_id = g_animation_manager->load(
						   *anim, directional_basename, animation_directory, anim_type);
						anims_.insert(std::make_pair(directional_animname, anim_id));
					} catch (const std::exception& e) {
						throw GameDataError(
						   "Direction '%s': %s", animation_direction_names[dir], e.what());
					}
					// Validate directions' scales
					if (dir == 0) {
						available_scales = g_animation_manager->get_animation(anim_id).available_scales();
					}
					if (available_scales.size() !=
					    g_animation_manager->get_animation(anim_id).available_scales().size()) {
						throw GameDataError("Direction '%s': number of available scales does not match "
						                    "with direction '%s'",
						                    animation_direction_names[dir], animation_direction_names[0]);
					}
				}
			} else {
				if (is_animation_known(animname)) {
					throw GameDataError(
					   "Tried to add already existing animation '%s'", animname.c_str());
				}
				if (animname == "idle") {
					anims_.insert(std::make_pair(
					   animname, g_animation_manager->load(
					                name_, *anim, basename, animation_directory, anim_type)));
				} else {
					anims_.insert(std::make_pair(
					   animname,
					   g_animation_manager->load(*anim, basename, animation_directory, anim_type)));
				}
			}
		} catch (const std::exception& e) {
			throw GameDataError("Error loading animation '%s' for map object '%s': %s",
			                    animname.c_str(), name().c_str(), e.what());
		}
	}
}

void MapObjectDescr::assign_directional_animation(DirAnimations* anims,
                                                  const std::string& basename) const {
	for (int32_t dir = 1; dir <= 6; ++dir) {
		const std::string anim_name = basename + animation_direction_names[dir - 1];
		try {
			anims->set_animation(dir, get_animation(anim_name, nullptr));
		} catch (const GameDataError& e) {
			throw GameDataError("MO: Missing directional animation: %s", e.what());
		}
	}
}

uint32_t MapObjectDescr::get_animation(const std::string& animname,
                                       const MapObject* /* object */) const {
	std::map<std::string, uint32_t>::const_iterator it = anims_.find(animname);
	if (it == anims_.end()) {
		throw GameDataError("Unknown animation: %s for %s", animname.c_str(), name().c_str());
	}
	return it->second;
}

uint32_t MapObjectDescr::main_animation() const {
	if (is_animation_known("idle")) {
		return get_animation("idle", nullptr);
	}
	return !anims_.empty() ? anims_.begin()->second : 0;
}

std::string MapObjectDescr::get_animation_name(uint32_t const anim) const {
	for (const auto& temp_anim : anims_) {
		if (temp_anim.second == anim) {
			return temp_anim.first;
		}
	}
	NEVER_HERE();
}

void MapObjectDescr::load_graphics() const {
	for (const auto& temp_anim : anims_) {
		g_animation_manager->get_animation(temp_anim.second).load_default_scale_and_sounds();
	}
}

const Image* MapObjectDescr::representative_image(const RGBColor* player_color) const {
	if (is_animation_known("idle")) {
		return g_animation_manager->get_representative_image(
		   get_animation("idle", nullptr), player_color);
	}
	return nullptr;
}

void MapObjectDescr::check_representative_image() const {
	if (representative_image() == nullptr) {
		throw Widelands::GameDataError(
		   "The %s %s has no representative image. Does it have an \"idle\" animation?",
		   to_string(type()).c_str(), name().c_str());
	}
}

const Image* MapObjectDescr::icon() const {
	if (!icon_filename_.empty()) {
		return g_image_cache->get(icon_filename_);
	}
	return nullptr;
}
const std::string& MapObjectDescr::icon_filename() const {
	return icon_filename_;
}

/**
 * Search for the attribute in the attribute list
 */
bool MapObjectDescr::has_attribute(AttributeIndex attr) const {
	for (const uint32_t& attrib : attribute_ids_) {
		if (attrib == attr) {
			return true;
		}
	}
	return false;
}

/**
 * Add an attribute to the attribute list if it's not already there
 */
void MapObjectDescr::add_attribute(AttributeIndex attr) {
	if (!has_attribute(attr)) {
		attribute_ids_.push_back(attr);
	}
}

void MapObjectDescr::add_attributes(const std::vector<std::string>& attribs) {
	for (const std::string& attrib : attribs) {
		uint32_t const attrib_id = get_attribute_id(attrib, true);
		add_attribute(attrib_id);
	}
}

const MapObjectDescr::Attributes& MapObjectDescr::attributes() const {
	return attribute_ids_;
}

/**
 * Lookup an attribute by name. If the attribute name hasn't been encountered
 * before and add_if_not_exists = true, we add it to the map. Else, throws exception.
 */
MapObjectDescr::AttributeIndex MapObjectDescr::get_attribute_id(const std::string& name,
                                                                bool add_if_not_exists) {
	if (!add_if_not_exists) {
		// Load on demand for objects that no player tribe owns
		Notifications::publish(
		   NoteMapObjectDescription(name, NoteMapObjectDescription::LoadType::kAttribute));
	}

	auto it = attribute_names_.find(name);

	if (it != attribute_names_.end()) {
		return it->second;
	}

	if (!add_if_not_exists) {
		throw GameDataError("get_attribute_id: attribute '%s' not found!\n", name.c_str());
	}
	AttributeIndex attribute_id = attribute_names_.size();
	attribute_names_[name] = attribute_id;
	return attribute_id;
}

void MapObjectDescr::set_helptexts(const std::string& tribename,
                                   std::map<std::string, std::string> localized_helptext) {
	// Create or overwrite
	helptexts_[tribename] = std::move(localized_helptext);
}

const std::map<std::string, std::string>&
MapObjectDescr::get_helptexts(const std::string& tribename) const {
	assert(has_helptext(tribename));
	return helptexts_.at(tribename);
}
bool MapObjectDescr::has_helptext(const std::string& tribename) const {
	return helptexts_.count(tribename) == 1;
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
	MutexLock m(MutexLock::ID::kObjects);

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
bool MapObject::init(EditorGameBase& egbase) {
	MutexLock m(MutexLock::ID::kObjects);
	egbase.objects().insert(this);
	return true;
}

/**
 * \warning Make sure you call this from derived classes!
 */
void MapObject::cleanup(EditorGameBase& egbase) {
	egbase.objects().remove(*this);
}

void MapObject::do_draw_info(const InfoToDraw& info_to_draw,
                             const std::string& census,
                             const std::string& statictics,
                             const Vector2f& field_on_dst,
                             float scale,
                             RenderTarget* dst) const {
	if ((info_to_draw & (InfoToDraw::kCensus | InfoToDraw::kStatistics)) == 0) {
		return;
	}

	// Rendering text is expensive, so let's just do it for only a few sizes.
	const float granularity = 4.f;
	float text_scale = scale;
	// The formula is a bit fancy to avoid too much text overlap.
	text_scale = std::round(granularity *
	                        (text_scale > 1.f ? std::sqrt(text_scale) : std::pow(text_scale, 2.f))) /
	             granularity;

	// Skip tiny text for performance reasons
	if (text_scale < 0.5f) {
		return;
	}

	UI::FontStyleInfo census_font(g_style_manager->building_statistics_style().census_font());
	census_font.set_size(scale * census_font.size());

	// We always render this so we can have a stable position for the statistics string.
	std::shared_ptr<const UI::RenderedText> rendered_census =
	   UI::g_fh->render(as_richtext_paragraph(census, census_font, UI::Align::kCenter), 120 * scale);
	Vector2i position = field_on_dst.cast<int>() - Vector2i(0, 48) * scale;
	if ((info_to_draw & InfoToDraw::kCensus) != 0) {
		rendered_census->draw(*dst, position, UI::Align::kCenter);
	}

	// Draw statistics if we want them, they are available and they fill fit
	if (((info_to_draw & InfoToDraw::kStatistics) != 0) && !statictics.empty() && scale >= 0.5f) {
		UI::FontStyleInfo statistics_font(
		   g_style_manager->building_statistics_style().statistics_font());
		statistics_font.set_size(scale * statistics_font.size());

		std::shared_ptr<const UI::RenderedText> rendered_statistics =
		   UI::g_fh->render(as_richtext_paragraph(statictics, statistics_font, UI::Align::kCenter));
		position.y += rendered_census->height() + text_height(statistics_font) / 4;
		rendered_statistics->draw(*dst, position, UI::Align::kCenter);
	}
}

const Image* MapObject::representative_image() const {
	return descr().representative_image(get_owner() != nullptr ? &get_owner()->get_playercolor() :
                                                                nullptr);
}

/**
 * Default implementation
 */
int32_t MapObject::get_training_attribute(TrainingAttribute /* attr */) const {
	return -1;
}

/**
 * Queue a CMD_ACT tdelta milliseconds from now, using the given data.
 *
 * \return The absolute gametime at which the CMD_ACT will occur.
 */
Time MapObject::schedule_act(Game& game, const Duration& tdelta, uint32_t const data) {
	if (tdelta.is_valid()) {
		const Time time = game.get_gametime() + tdelta;

		game.cmdqueue().enqueue(new CmdAct(time, *this, data));

		return time;
	}
	return Time();
}

/**
 * Called when a CMD_ACT triggers.
 */
void MapObject::act(Game& /* game */, uint32_t /* data */) {
}

/**
 * Set the logsink. This should only be used by the debugging facilities.
 */
void MapObject::set_logsink(LogSink* const sink) {
	logsink_ = sink;
}

void MapObject::log_general_info(const EditorGameBase& /* egbase */) const {
}

const Player& MapObject::owner() const {
	if (owner_ == nullptr) {
		throw wexception("Attempted to get null owner reference for player");
	}
	return *owner_;
}

/**
 * Prints a log message prepended by the object's serial number.
 */
void MapObject::molog(const Time& gametime, char const* fmt, ...) const {
	if (!g_verbose && (logsink_ == nullptr)) {
		return;
	}

	va_list va;
	char buffer[2048];

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (logsink_ != nullptr) {
		logsink_->log(buffer);
	}

	log_dbg_time(gametime, "MO(%u,%s): %s", serial_, descr().name().c_str(), buffer);
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
		if (header != HeaderMapObject) {
			throw wexception("header is %u, expected %u", header, HeaderMapObject);
		}

		uint8_t const packet_version = fr.unsigned_8();
		// Supporting older versions for map loading
		if (packet_version < 1 || packet_version > kCurrentPacketVersionMapObject) {
			throw UnhandledVersionError("MapObject", packet_version, kCurrentPacketVersionMapObject);
		}

		Serial const serial = fr.unsigned_32();
		try {
			mol().register_object<MapObject>(serial, *get_object());
		} catch (const WException& e) {
			throw wexception("%u: %s", serial, e.what());
		}

		if (packet_version >= 2) {
			MapObject& obj = *get_object();
			obj.reserved_by_worker_ = (fr.unsigned_8() != 0u);
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
 *
 * We also preload some animation graphics here to prevent jitter at game start.
 */
void MapObject::Loader::load_finish() {
	const MapObject& mo = get<MapObject>();
	mo.descr().load_graphics();
}

/**
 * Save the MapObject to the given file.
 */
void MapObject::save(EditorGameBase& /* egbase */, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderMapObject);
	fw.unsigned_8(kCurrentPacketVersionMapObject);

	fw.unsigned_32(mos.get_object_file_index(*this));
	fw.unsigned_8(static_cast<uint8_t>(reserved_by_worker_));
}

}  // namespace Widelands
