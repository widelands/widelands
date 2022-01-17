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

#include "logic/map_objects/immovable.h"

#include <memory>

#include "base/log.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/immovable_program.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"

namespace Widelands {

BaseImmovable::BaseImmovable(const MapObjectDescr& mo_descr) : MapObject(&mo_descr) {
}

int32_t BaseImmovable::string_to_size(const std::string& size) {
	if (size == "none") {
		return BaseImmovable::NONE;
	}
	if (size == "small") {
		return BaseImmovable::SMALL;
	}
	if (size == "medium") {
		return BaseImmovable::MEDIUM;
	}
	if (size == "big") {
		return BaseImmovable::BIG;
	}
	throw GameDataError("Unknown size %s.", size.c_str());
}

std::string BaseImmovable::size_to_string(int32_t size) {
	switch (size) {
	case BaseImmovable::NONE:
		return "none";
	case BaseImmovable::SMALL:
		return "small";
	case BaseImmovable::MEDIUM:
		return "medium";
	case BaseImmovable::BIG:
		return "big";
	default:
		NEVER_HERE();
	}
}

static std::string const base_immovable_name = "unknown";

/**
 * Associate the given field with this immovable. Recalculate if necessary.
 *
 * Only call this during init.
 *
 * \note this function will remove the immovable (if existing) currently connected to this position.
 */
void BaseImmovable::set_position(EditorGameBase& egbase, const Coords& c) {
	assert(c);

	Map* map = egbase.mutable_map();
	FCoords f = map->get_fcoords(c);
	if (f.field->immovable && f.field->immovable != this) {
		f.field->immovable->remove(egbase);
	}

	f.field->immovable = this;

	if (get_size() >= SMALL) {
		map->recalc_for_field_area(egbase, Area<FCoords>(f, 2));
	}

	// Needed so players can update partially visible buildings' vision.
	if (get_size() >= BIG) {
		Notifications::publish(NoteFieldTerrainChanged{f, map->get_index(f)});
	}
}

/**
 * Remove the link to the given field.
 *
 * Only call this during cleanup.
 */
void BaseImmovable::unset_position(EditorGameBase& egbase, const Coords& c) {
	Map* map = egbase.mutable_map();
	FCoords const f = map->get_fcoords(c);

	// this is to help to debug failing assertion below (see bug 1542238)
	if (f.field->immovable != this) {
		log_err_time(
		   egbase.get_gametime(),
		   "Internal error: Immovable at %3dx%3d does not match: is %s but %s was expected.\n", c.x,
		   c.y, (f.field->immovable) ? f.field->immovable->descr().name().c_str() : "None",
		   descr().name().c_str());
	}

	assert(f.field->immovable == this);

	// Needed so players can update partially visible buildings' vision.
	if (get_size() >= BIG) {
		Notifications::publish(NoteFieldTerrainChanged{f, map->get_index(f)});
	}

	f.field->immovable = nullptr;
	egbase.inform_players_about_immovable(f.field - &(*map)[0], nullptr);

	if (get_size() >= SMALL) {
		map->recalc_for_field_area(egbase, Area<FCoords>(f, 2));
	}
}

/*
==============================================================================

ImmovableDescr IMPLEMENTATION

==============================================================================
*/

/**
 * Parse a common immovable functions from init file.
 */
ImmovableDescr::ImmovableDescr(const std::string& init_descname,
                               const LuaTable& table,
                               const std::vector<std::string>& attribs,
                               Descriptions& descriptions)
   : MapObjectDescr(MapObjectType::IMMOVABLE, table.get_string("name"), init_descname, table),
     descriptions_(descriptions),
     size_(BaseImmovable::NONE) {
	if (!is_animation_known("idle")) {
		throw GameDataError("Immovable %s has no idle animation", name().c_str());
	}

	if (table.has_key("size")) {
		size_ = BaseImmovable::string_to_size(table.get_string("size"));
	}

	if (table.has_key("buildcost")) {
		buildcost_ = Buildcost(table.get_table("buildcost"), descriptions);
	}

	if (table.has_key("terrain_affinity")) {
		terrain_affinity_.reset(new TerrainAffinity(*table.get_table("terrain_affinity"), name()));
	}

	if (!attribs.empty()) {
		add_attributes(attribs);

		for (const std::string& attribute : attribs) {
			if (attribute == "resi") {
				// All resource indicators must have a menu icon
				if (icon_filename().empty()) {
					throw GameDataError("Resource indicator %s has no menu icon", name().c_str());
				}
				break;
			}
		}

		// Old trees get an extra species name so we can use it in help lists.
		bool is_tree = false;
		for (const std::string& attribute : attribs) {
			if (attribute == "tree") {
				is_tree = true;
				break;
			}
		}
		if (is_tree) {
			if (!table.has_key("species")) {
				throw wexception(
				   "Immovable '%s' with type 'tree' must define a species", name().c_str());
			}
			species_ = table.get_string("species");
		}
	}

	std::unique_ptr<LuaTable> programs = table.get_table("programs");
	for (std::string program_name : programs->keys<std::string>()) {
		program_name = to_lower(program_name);
		if (programs_.count(program_name)) {
			throw GameDataError("Program '%s' has already been declared for immovable '%s'",
			                    program_name.c_str(), name().c_str());
		}
		try {
			// TODO(GunChleoc): Compatibility, remove after v1.0
			if (program_name == "program") {
				log_warn("The main program for the immovable %s should be renamed from 'program' "
				         "to 'main'\n",
				         name().c_str());
				if (programs->keys<std::string>().count(MapObjectProgram::kMainProgram)) {
					log_err("         This also clashes with an already existing 'main' program\n");
				}
				programs_[MapObjectProgram::kMainProgram] = new ImmovableProgram(
				   MapObjectProgram::kMainProgram,
				   programs->get_table(program_name)->array_entries<std::string>(), *this);
			} else {
				programs_[program_name] = new ImmovableProgram(
				   program_name, programs->get_table(program_name)->array_entries<std::string>(),
				   *this);
			}
		} catch (const std::exception& e) {
			throw GameDataError("%s: Error in immovable program %s: %s", name().c_str(),
			                    program_name.c_str(), e.what());
		}
	}

	make_sure_default_program_is_there();
}

bool ImmovableDescr::has_terrain_affinity() const {
	return terrain_affinity_ != nullptr;
}

const TerrainAffinity& ImmovableDescr::terrain_affinity() const {
	return *terrain_affinity_;
}

void ImmovableDescr::make_sure_default_program_is_there() {
	if (!programs_.count(MapObjectProgram::kMainProgram)) {  //  default program
		assert(is_animation_known("idle"));
		std::vector<std::string> arguments{"idle"};
		programs_[MapObjectProgram::kMainProgram] = new ImmovableProgram(
		   MapObjectProgram::kMainProgram, std::unique_ptr<ImmovableProgram::Action>(
		                                      new ImmovableProgram::ActAnimate(arguments, *this)));
	}
}

void ImmovableDescr::add_collected_by(const Descriptions& descriptions,
                                      const std::string& prodsite) {
	if (collected_by_.count(prodsite)) {
		return;  // recursion break
	}
	collected_by_.insert(prodsite);
	for (const std::string& immo : became_from_) {
		descriptions.get_mutable_immovable_descr(descriptions.safe_immovable_index(immo))
		   ->add_collected_by(descriptions, prodsite);
	}
}

void ImmovableDescr::register_immovable_relation(const std::string& a, const std::string& b) {
	descriptions_.add_immovable_relation(a, b);
}

/**
 * Cleanup
 */
ImmovableDescr::~ImmovableDescr() {
	while (!programs_.empty()) {
		delete programs_.begin()->second;
		programs_.erase(programs_.begin());
	}
}

/**
 * Find the program of the given name.
 */
ImmovableProgram const* ImmovableDescr::get_program(const std::string& program_name) const {
	{
		Programs::const_iterator const it = programs_.find(program_name);
		if (it != programs_.end()) {
			return it->second;
		}
	}

	// Program not found - fall back to MapObjectProgram::kMainProgram for permanent map
	// compatibility
	Programs::const_iterator const it = programs_.find(MapObjectProgram::kMainProgram);
	if (it != programs_.end()) {
		return it->second;
	}

	throw GameDataError("immovable %s has no program \"%s\"", name().c_str(), program_name.c_str());
}

/**
 * Create an immovable of this type
 * If this immovable was created by a building, 'former_building' can be set
 * in order to display information about it.
 */
Immovable& ImmovableDescr::create(EditorGameBase& egbase,
                                  const Coords& coords,
                                  const BuildingDescr* former_building_descr) const {
	Immovable& result = *new Immovable(*this, former_building_descr);
	result.position_ = coords;
	result.init(egbase);
	return result;
}

/*
==============================

IMPLEMENTATION

==============================
*/

Immovable::Immovable(const ImmovableDescr& imm_descr,
                     const Widelands::BuildingDescr* former_building_descr)
   : BaseImmovable(imm_descr),
     former_building_descr_(former_building_descr),
     anim_(0),
     animstart_(0),
     program_(nullptr),
     program_ptr_(0),
     anim_construction_total_(0),
     anim_construction_done_(0),
     program_step_(0),
     growth_delay_(0) {
}

BaseImmovable::PositionList Immovable::get_positions(const EditorGameBase&) const {
	PositionList rv;

	rv.push_back(position_);
	return rv;
}

void BaseImmovable::set_owner(Player* player) {
	assert(owner_ == nullptr);
	owner_ = player;
}

int32_t Immovable::get_size() const {
	return descr().get_size();
}

bool Immovable::get_passable() const {
	return descr().get_size() < BIG;
}

void Immovable::start_animation(const EditorGameBase& egbase, uint32_t const anim) {
	anim_ = anim;
	animstart_ = egbase.get_gametime();
	anim_construction_done_ = anim_construction_total_ = 0;
}

void Immovable::increment_program_pointer() {
	program_ptr_ = (program_ptr_ + 1) % program_->size();
	action_data_.reset(nullptr);
}

bool Immovable::is_marked_for_removal(PlayerNumber p) const {
	return marked_for_removal_.count(p) > 0;
}

void Immovable::set_marked_for_removal(PlayerNumber p, bool mark) {
	if (mark) {
		marked_for_removal_.insert(p);
	} else {
		marked_for_removal_.erase(p);
	}
}

/**
 * Actually initialize the immovable.
 */
bool Immovable::init(EditorGameBase& egbase) {
	BaseImmovable::init(egbase);

	set_position(egbase, position_);

	//  Set animation data according to current program state.
	ImmovableProgram const* prog = program_;
	if (!prog) {
		prog = descr().get_program(MapObjectProgram::kMainProgram);
	}
	assert(prog != nullptr);

	if (upcast(ImmovableProgram::ActAnimate const, act_animate, &(*prog)[program_ptr_])) {
		start_animation(egbase, act_animate->animation());
	}

	if (upcast(Game, game, &egbase)) {
		switch_program(*game, MapObjectProgram::kMainProgram);
	}
	return true;
}

/**
 * Cleanup before destruction
 */
void Immovable::cleanup(EditorGameBase& egbase) {
	unset_position(egbase, position_);

	BaseImmovable::cleanup(egbase);
}

/**
 * Switch the currently running program.
 */
void Immovable::switch_program(Game& game, const std::string& program_name) {
	program_ = descr().get_program(program_name);
	assert(program_ != nullptr);
	program_ptr_ = 0;
	program_step_ = Time(0);
	action_data_.reset(nullptr);
	schedule_act(game, Duration(1));
}

/**
 * Run program timer.
 */
void Immovable::act(Game& game, uint32_t const data) {
	BaseImmovable::act(game, data);

	if (program_step_ <= game.get_gametime()) {
		//  Might delete itself!
		(*program_)[program_ptr_].execute(game, *this);
	}
}

void Immovable::draw(const Time& gametime,
                     const InfoToDraw info_to_draw,
                     const Vector2f& point_on_dst,
                     const Widelands::Coords& coords,
                     float scale,
                     RenderTarget* dst) {
	if (!anim_) {
		return;
	}
	if (!anim_construction_total_) {
		dst->blit_animation(
		   point_on_dst, coords, scale, anim_, Time(gametime.get() - animstart_.get()));
		if (former_building_descr_) {
			do_draw_info(
			   info_to_draw, former_building_descr_->descname(), "", point_on_dst, scale, dst);
		}
	} else {
		draw_construction(gametime, info_to_draw, point_on_dst, coords, scale, dst);
	}
}

void Immovable::draw_construction(const Time& gametime,
                                  const InfoToDraw info_to_draw,
                                  const Vector2f& point_on_dst,
                                  const Widelands::Coords& coords,
                                  const float scale,
                                  RenderTarget* dst) {
	const ImmovableProgram::ActConstruct* constructionact = nullptr;
	if (program_ptr_ < program_->size()) {
		constructionact =
		   dynamic_cast<const ImmovableProgram::ActConstruct*>(&(*program_)[program_ptr_]);
	}

	const Duration steptime = constructionact ? constructionact->buildtime() : Duration(5000);

	Duration done(0);
	if (anim_construction_done_ > 0) {
		done = steptime * (anim_construction_done_ - 1);
		done += std::min(steptime, gametime - animstart_);
	}

	Duration total = steptime * anim_construction_total_;
	if (done > total) {
		done = total;
	}

	const Animation& anim = g_animation_manager->get_animation(anim_);
	const size_t nr_frames = anim.nr_frames();
	Duration frametime(g_animation_manager->get_animation(anim_).frametime());
	Duration units_per_frame = (total + Duration(nr_frames - 1)) / nr_frames;
	const size_t current_frame = done.get() / units_per_frame.get();

	assert(get_owner() != nullptr);  // Who would build something they do not own?
	const RGBColor& player_color = get_owner()->get_playercolor();
	if (current_frame > 0) {
		// Not the first pic, so draw the previous one in the back
		dst->blit_animation(point_on_dst, Widelands::Coords::null(), scale, anim_,
		                    Time(frametime.get() * (current_frame - 1)), &player_color);
	}

	const int percent = ((done.get() % units_per_frame.get()) * 100) / units_per_frame.get();

	dst->blit_animation(point_on_dst, coords, scale, anim_, Time((frametime * current_frame).get()),
	                    &player_color, percent);

	// Additionally, if statistics are enabled, draw a progression string
	do_draw_info(
	   info_to_draw, descr().descname(),
	   StyleManager::color_tag(format(_("%i%% built"), (done.get() * 100 / total.get())),
	                           g_style_manager->building_statistics_style().construction_color()),
	   point_on_dst, scale, dst);
}

/**
 * Set the current action's data to \p data.
 *
 * \warning \p data must not be equal to the currently set data, but it may be 0.
 */
void Immovable::set_action_data(ImmovableActionData* data) {
	action_data_.reset(data);
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersionImmovable = 11;

// Supporting older versions for map loading
void Immovable::Loader::load(FileRead& fr, uint8_t const packet_version) {
	BaseImmovable::Loader::load(fr);

	Immovable& imm = dynamic_cast<Immovable&>(*get_object());

	// Supporting older versions for map loading
	if (packet_version >= 5) {
		PlayerNumber pn = fr.unsigned_8();
		if (pn && pn <= kMaxPlayers) {
			Player* plr = egbase().get_player(pn);
			if (!plr) {
				throw GameDataError("Immovable::load: player %u does not exist", pn);
			}
			imm.set_owner(plr);
		}
	}

	// Position
	imm.position_ = read_coords_32(&fr, egbase().map().extent());
	imm.set_position(egbase(), imm.position_);

	if (packet_version > 7 && (packet_version < 11 || fr.unsigned_8())) {
		if (Player* owner = imm.get_owner()) {
			DescriptionIndex idx = owner->tribe().safe_building_index(fr.string());
			if (owner->tribe().has_building(idx)) {
				imm.set_former_building(*owner->tribe().get_building_descr(idx));
			}
		}
	}

	// Animation. If the animation is no longer known, pick the main animation instead.
	char const* const animname = fr.c_string();
	if (imm.descr().is_animation_known(animname)) {
		imm.anim_ = imm.descr().get_animation(animname, &imm);
	} else {
		log_warn("Unknown animation '%s' for immovable '%s', using main animation instead.\n",
		         animname, imm.descr().name().c_str());
		imm.anim_ = imm.descr().main_animation();
	}

	imm.animstart_ = Time(fr);
	if (packet_version >= 4) {
		imm.anim_construction_total_ = fr.unsigned_32();
		if (imm.anim_construction_total_) {
			imm.anim_construction_done_ = fr.unsigned_32();
		}
	}

	{  //  program
		std::string program_name;
		if (1 == packet_version) {
			program_name = fr.unsigned_8() ? fr.c_string() : MapObjectProgram::kMainProgram;
			program_name = to_lower(program_name);
		} else {
			program_name = fr.c_string();
			if (program_name.empty()) {
				program_name = MapObjectProgram::kMainProgram;
			}
		}

		imm.program_ = imm.descr().get_program(program_name);
	}
	imm.program_ptr_ = fr.unsigned_32();

	if (!imm.program_) {
		imm.program_ptr_ = 0;
	} else {
		if (imm.program_ptr_ >= imm.program_->size()) {
			// Try to not fail if the program of some immovable has changed
			// significantly.
			// Note that in some cases, the immovable may end up broken despite
			// the fixup, but there isn't really anything we can do against that.
			log_warn("Immovable '%s', size of program '%s' seems to have changed.\n",
			         imm.descr().name().c_str(), imm.program_->name().c_str());
			imm.program_ptr_ = 0;
		}
	}

	if (packet_version > 6) {
		imm.program_step_ = Time(fr);
	} else {
		imm.program_step_ = Time(fr.signed_32());
	}
	imm.growth_delay_ = packet_version >= 8 ? Duration(fr) : Duration(0);

	if (packet_version >= 3 && packet_version <= 5) {
		imm.reserved_by_worker_ = fr.unsigned_8();
	}
	if (packet_version >= 4) {
		std::string dataname = fr.c_string();
		if (!dataname.empty()) {
			imm.set_action_data(ImmovableActionData::load(fr, imm, dataname));
		}
	}
	if (packet_version >= 10) {
		for (uint8_t i = fr.unsigned_8(); i; --i) {
			imm.marked_for_removal_.insert(fr.unsigned_8());
		}
	}
}

void Immovable::Loader::load_pointers() {
	BaseImmovable::Loader::load_pointers();
}

void Immovable::Loader::load_finish() {
	BaseImmovable::Loader::load_finish();

	Immovable& imm = dynamic_cast<Immovable&>(*get_object());
	if (upcast(Game, game, &egbase())) {
		imm.schedule_act(*game, Duration(1));
	}

	egbase().inform_players_about_immovable(
	   Map::get_index(imm.position_, egbase().map().get_width()), &imm.descr());
}

void Immovable::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	// This is in front because it is required to obtain the description
	// necessary to create the Immovable
	fw.unsigned_8(HeaderImmovable);
	fw.unsigned_8(kCurrentPacketVersionImmovable);
	fw.string(descr().name());

	// The main loading data follows
	BaseImmovable::save(egbase, mos, fw);

	fw.unsigned_8(get_owner() ? get_owner()->player_number() : 0);
	write_coords_32(&fw, position_);

	// Former building
	const bool has_former_building = get_owner() && former_building_descr_;
	fw.unsigned_8(has_former_building ? 1 : 0);
	if (has_former_building) {
		fw.string(former_building_descr_->name());
	}

	// Animations
	fw.string(descr().get_animation_name(anim_));
	animstart_.save(fw);
	fw.unsigned_32(anim_construction_total_);
	if (anim_construction_total_) {
		fw.unsigned_32(anim_construction_done_);
	}

	// Program Stuff
	fw.string(program_ ? program_->name() : "");

	fw.unsigned_32(program_ptr_);
	program_step_.save(fw);
	growth_delay_.save(fw);

	if (action_data_) {
		fw.c_string(action_data_->name());
		action_data_->save(fw, *this);
	} else {
		fw.c_string("");
	}

	fw.unsigned_8(marked_for_removal_.size());
	for (const PlayerNumber& p : marked_for_removal_) {
		fw.unsigned_8(p);
	}
}

MapObject::Loader* Immovable::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		// Supporting older versions for map loading
		if (1 <= packet_version && packet_version <= kCurrentPacketVersionImmovable) {
			if (packet_version < 11) {
				fr.c_string();  // Consume obsolete owner type (world/tribes)
			}
			Immovable* imm = new Immovable(*egbase.descriptions().get_immovable_descr(
			   egbase.mutable_descriptions()->load_immovable(fr.c_string())));

			loader->init(egbase, mol, *imm);
			loader->load(fr, packet_version);
		} else {
			throw UnhandledVersionError("Immovable", packet_version, kCurrentPacketVersionImmovable);
		}
	} catch (const std::exception& e) {
		throw wexception("immovable type: %s", e.what());
	}

	return loader.release();
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * compute the remaining buildcost.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_remaining_buildcost(Game& /* game */, Buildcost* buildcost) {
	ActConstructData* d = get_action_data<ActConstructData>();
	if (!d) {
		return false;
	}

	const Buildcost& total = descr().buildcost();
	for (const auto& item : total) {
		uint32_t delivered = d->delivered[item.first];
		if (delivered < item.second) {
			(*buildcost)[item.first] = item.second - delivered;
		}
	}

	return true;
}

bool Immovable::apply_growth_delay(Game& game) {
	if (growth_delay_.get() == 0) {
		return false;
	}
	schedule_act(game, growth_delay_);
	growth_delay_ = Duration(0);
	return true;
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * consume the given ware type as delivered.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_ware(Game& game, DescriptionIndex index) {
	ActConstructData* d = get_action_data<ActConstructData>();
	if (!d) {
		return false;
	}

	molog(game.get_gametime(), "construct_ware: index %u", index);

	Buildcost::iterator it = d->delivered.find(index);
	if (it != d->delivered.end()) {
		it->second++;
	} else {
		d->delivered[index] = 1;
	}

	anim_construction_done_ = d->delivered.total();
	animstart_ = game.get_gametime();

	molog(game.get_gametime(), "construct_ware: total %u delivered: %u", index, d->delivered[index]);

	Buildcost remaining;
	construct_remaining_buildcost(game, &remaining);

	const ImmovableProgram::ActConstruct* action =
	   dynamic_cast<const ImmovableProgram::ActConstruct*>(&(*program_)[program_ptr_]);
	assert(action != nullptr);

	if (remaining.empty()) {
		// Wait for the last building animation to finish.
		program_step_ = schedule_act(game, action->buildtime());
	} else {
		program_step_ = schedule_act(game, action->decaytime());
	}

	return true;
}

/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize
 */
PlayerImmovable::PlayerImmovable(const MapObjectDescr& mo_descr)
   : BaseImmovable(mo_descr), ware_economy_(nullptr), worker_economy_(nullptr) {
}

/**
 * Cleanup
 */
PlayerImmovable::~PlayerImmovable() {
	if (!workers_.empty()) {
		log_warn("PlayerImmovable::~PlayerImmovable: %" PRIuS " workers left!\n", workers_.size());
	}
}

/**
 * Change the economy, transfer the workers
 */
void PlayerImmovable::set_economy(Economy* const e, WareWorker type) {
	if (get_economy(type) == e) {
		return;
	}

	(type == wwWARE ? ware_economy_ : worker_economy_) = e;

	for (Worker* worker : workers_) {
		worker->set_economy(e, type);
	}
}

/**
 * Associate the given worker with this immovable.
 * The worker will be transferred along to another economy, and it will be
 * released when the immovable is destroyed.
 *
 * This should only be called from Worker::set_location.
 */
void PlayerImmovable::add_worker(Worker& w) {
	workers_.push_back(&w);
}

/**
 * Disassociate the given worker with this building.
 *
 * This should only be called from Worker::set_location.
 */
void PlayerImmovable::remove_worker(Worker& w) {
	for (Workers::iterator worker_iter = workers_.begin(); worker_iter != workers_.end();
	     ++worker_iter) {
		if (*worker_iter == &w) {
			*worker_iter = *(workers_.end() - 1);
			workers_.pop_back();
			return;
		}
	}

	throw wexception("PlayerImmovable::remove_worker: not in list");
}

void Immovable::set_former_building(const BuildingDescr& building) {
	former_building_descr_ = &building;
}

/**
 * Set the immovable's owner. Currently, it can only be set once.
 */
void PlayerImmovable::set_owner(Player* new_owner) {
	assert(owner_ == nullptr);
	owner_ = new_owner;
	Notifications::publish(NoteImmovable(this, NoteImmovable::Ownership::GAINED));
}

/**
 * Initialize the immovable.
 */
bool PlayerImmovable::init(EditorGameBase& egbase) {
	return BaseImmovable::init(egbase);
}

/**
 * Release workers
 */
void PlayerImmovable::cleanup(EditorGameBase& egbase) {
	while (!workers_.empty()) {
		workers_[0]->set_location(nullptr);
	}

	Notifications::publish(NoteImmovable(this, NoteImmovable::Ownership::LOST));

	BaseImmovable::cleanup(egbase);
}

/**
 * We are the destination of the given ware's transfer, which is not associated
 * with any request.
 */
void PlayerImmovable::receive_ware(Game&, DescriptionIndex ware) {
	throw wexception("MO(%u): Received a ware(%u), do not know what to do with it", serial(), ware);
}

/**
 * We are the destination of the given worker's transfer, which is not
 * associated with any request.
 */
void PlayerImmovable::receive_worker(Game&, Worker& worker) {
	throw wexception(
	   "MO(%u): Received a worker(%u), do not know what to do with it", serial(), worker.serial());
}

/**
 * Dump general information
 */
void PlayerImmovable::log_general_info(const EditorGameBase& egbase) const {
	BaseImmovable::log_general_info(egbase);

	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "this: %p\n", this);
	molog(egbase.get_gametime(), "owner_: %p\n", owner_.load());
	FORMAT_WARNINGS_ON
	molog(egbase.get_gametime(), "player_number: %i\n", owner_.load()->player_number());
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "ware_economy_: %p\n", ware_economy_);
	molog(egbase.get_gametime(), "worker_economy_: %p\n", worker_economy_);
	FORMAT_WARNINGS_ON
}

constexpr uint8_t kCurrentPacketVersionPlayerImmovable = 1;

void PlayerImmovable::Loader::load(FileRead& fr) {
	BaseImmovable::Loader::load(fr);

	PlayerImmovable& imm = get<PlayerImmovable>();

	try {
		uint8_t packet_version = fr.unsigned_8();

		if (packet_version == kCurrentPacketVersionPlayerImmovable) {
			PlayerNumber owner_number = fr.unsigned_8();

			if (!owner_number || owner_number > egbase().map().get_nrplayers()) {
				throw GameDataError("owner number is %u but there are only %u players", owner_number,
				                    egbase().map().get_nrplayers());
			}

			Player* owner = egbase().get_player(owner_number);
			if (!owner) {
				throw GameDataError("owning player %u does not exist", owner_number);
			}

			imm.owner_ = owner;
		} else {
			throw UnhandledVersionError(
			   "PlayerImmovable", packet_version, kCurrentPacketVersionPlayerImmovable);
		}
	} catch (const std::exception& e) {
		throw wexception("loading player immovable: %s", e.what());
	}
}

void PlayerImmovable::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	BaseImmovable::save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersionPlayerImmovable);
	fw.unsigned_8(owner().player_number());
}
}  // namespace Widelands
