/*
 * Copyright (C) 2002-2003, 2006-2011, 2013 by the Widelands Development Team
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

#include <cstdio>
#include <cstring>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "config.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/constants.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/immovable_program.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapfringeregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/tribes_legacy_lookup_table.h"
#include "map_io/world_legacy_lookup_table.h"
#include "notifications/notifications.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "wui/interactive_base.h"

namespace Widelands {

namespace  {

BaseImmovable::Size string_to_size(const std::string& size) {
	if (size == "none")
		return BaseImmovable::NONE;
	if (size == "small")
		return BaseImmovable::SMALL;
	if (size == "medium")
		return BaseImmovable::MEDIUM;
	if (size == "big")
		return BaseImmovable::BIG;
	throw GameDataError("Unknown size %s.", size.c_str());
}

}  // namespace


BaseImmovable::BaseImmovable(const MapObjectDescr & mo_descr) :
MapObject(&mo_descr)
{}


static std::string const base_immovable_name = "unknown";

/**
 * Associate the given field with this immovable. Recalculate if necessary.
 *
 * Only call this during init.
 *
 * \note this function will remove the immovable (if existing) currently connected to this position.
 */
void BaseImmovable::set_position(EditorGameBase & egbase, Coords const c)
{
	assert(c);

	Map & map = egbase.map();
	FCoords f = map.get_fcoords(c);
	if (f.field->immovable && f.field->immovable != this)
		f.field->immovable->remove(egbase);

	f.field->immovable = this;

	if (get_size() >= SMALL)
		map.recalc_for_field_area(egbase.world(), Area<FCoords>(f, 2));
}

/**
 * Remove the link to the given field.
 *
 * Only call this during cleanup.
*/
void BaseImmovable::unset_position(EditorGameBase & egbase, Coords const c)
{
	Map & map = egbase.map();
	FCoords const f = map.get_fcoords(c);

	// this is to help to debug failing assertion below (see bug 1542238)
	if (f.field->immovable != this){
		log(" Internal error: Immovable at %3dx%3d does not match: is %s but %s was expected.\n",
		c.x,
		c.y,
		(f.field->immovable) ? f.field->immovable->descr().name().c_str() : "None",
		descr().name().c_str());
	}

	assert(f.field->immovable == this);

	f.field->immovable = nullptr;
	egbase.inform_players_about_immovable(f.field - &map[0], nullptr);

	if (get_size() >= SMALL)
		map.recalc_for_field_area(egbase.world(), Area<FCoords>(f, 2));
}


/*
==============================================================================

ImmovableProgram IMPLEMENTATION

==============================================================================
*/

ImmovableProgram::ImmovableProgram(const std::string& init_name,
								   const std::vector<std::string>& lines,
											  ImmovableDescr* immovable)
   : m_name(init_name) {
	for (const std::string& line : lines) {
		std::vector<std::string> parts;
		boost::split(parts, line, boost::is_any_of("="));
		if (parts.size() != 2) {
			throw GameDataError("invalid line: %s.", line.c_str());
		}
		std::unique_ptr<char []> arguments(new char[parts[1].size() + 1]);
		strncpy(arguments.get(), parts[1].c_str(), parts[1].size() + 1);

		Action* action;
		if (parts[0] == "animate") {
			action = new ActAnimate(arguments.get(), *immovable);
		} else if (parts[0] == "transform") {
			action = new ActTransform(arguments.get(), *immovable);
		} else if (parts[0] == "grow") {
			action = new ActGrow(arguments.get(), *immovable);
		} else if (parts[0] == "remove") {
			action = new ActRemove(arguments.get(), *immovable);
		} else if (parts[0] == "seed") {
			action = new ActSeed(arguments.get(), *immovable);
		} else if (parts[0] == "playFX") {
			action = new ActPlayFX(arguments.get(), *immovable);
		} else if (parts[0] == "construction") {
			action = new ActConstruction(arguments.get(), *immovable);
		} else {
			throw GameDataError(
				"unknown command type \"%s\" in immovable \"%s\"", parts[0].c_str(), immovable->name().c_str());
		}
		m_actions.push_back(action);
	}
	if (m_actions.empty())
		throw GameDataError("no actions");
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
										 MapObjectDescr::OwnerType input_type) :
	MapObjectDescr(
	MapObjectType::IMMOVABLE, table.get_string("name"), init_descname, table),
	m_size(BaseImmovable::NONE),
	owner_type_(input_type) {
	if (!is_animation_known("idle")) {
		throw GameDataError("Immovable %s has no idle animation", table.get_string("name").c_str());
	}

	if (table.has_key("size")) {
		m_size = string_to_size(table.get_string("size"));
	}

	if (table.has_key("terrain_affinity")) {
		terrain_affinity_.reset(new TerrainAffinity(*table.get_table("terrain_affinity"), name()));
	}

	if (table.has_key("attributes")) {
		add_attributes(table.get_table("attributes")->
							array_entries<std::string>(), {MapObject::Attribute::RESI});
	}

	std::unique_ptr<LuaTable> programs = table.get_table("programs");
	for (const std::string& program_name : programs->keys<std::string>()) {
		try {
			m_programs[program_name] = new ImmovableProgram(
			   program_name, programs->get_table(program_name)->array_entries<std::string>(), this);
		} catch (const std::exception& e) {
			throw wexception("Error in program %s: %s", program_name.c_str(), e.what());
		}
	}

	make_sure_default_program_is_there();
}

/**
 * Parse a world immovable from its init file.
 */
ImmovableDescr::ImmovableDescr(const std::string& init_descname, const LuaTable& table, const World& world) :
	ImmovableDescr(init_descname, table, MapObjectDescr::OwnerType::kWorld) {

	int editor_category_index =
			world.editor_immovable_categories().get_index(table.get_string("editor_category"));
	if (editor_category_index == Widelands::INVALID_INDEX) {
		throw GameDataError("Unknown editor_category: %s\n",
							  table.get_string("editor_category").c_str());
	}
	editor_category_ = world.editor_immovable_categories().get_mutable(editor_category_index);
}

/**
 * Parse a tribes immovable from its init file.
 */
ImmovableDescr::ImmovableDescr(const std::string& init_descname,
										 const LuaTable& table,
										 const Tribes& tribes) :
	ImmovableDescr(init_descname, table, MapObjectDescr::OwnerType::kTribe) {
	if (table.has_key("buildcost")) {
		m_buildcost = Buildcost(table.get_table("buildcost"), tribes);
	}
}

const EditorCategory& ImmovableDescr::editor_category() const {
	return *editor_category_;
}

bool ImmovableDescr::has_terrain_affinity() const {
	return terrain_affinity_.get() != nullptr;
}

const TerrainAffinity& ImmovableDescr::terrain_affinity() const {
	return *terrain_affinity_;
}

void ImmovableDescr::make_sure_default_program_is_there() {
	if (!m_programs.count("program")) {  //  default program
		assert(is_animation_known("idle"));
		char parameters[] = "idle";
		m_programs["program"] = new ImmovableProgram(
		   "program", new ImmovableProgram::ActAnimate(parameters, *this));
	}
}

/**
 * Cleanup
*/
ImmovableDescr::~ImmovableDescr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Find the program of the given name.
*/
ImmovableProgram const * ImmovableDescr::get_program
	(const std::string & program_name) const
{
	Programs::const_iterator const it = m_programs.find(program_name);

	if (it == m_programs.end())
		throw GameDataError
			("immovable %s has no program \"%s\"",
			 name().c_str(), program_name.c_str());

	return it->second;
}


/**
 * Create an immovable of this type
*/
Immovable & ImmovableDescr::create
	(EditorGameBase & egbase, Coords const coords) const
{
	assert(this);
	Immovable & result = *new Immovable(*this);
	result.m_position = coords;
	result.init(egbase);
	return result;
}


/*
==============================

IMPLEMENTATION

==============================
*/

Immovable::Immovable(const ImmovableDescr & imm_descr) :
BaseImmovable (imm_descr),
m_owner(nullptr),
m_anim        (0),
m_animstart   (0),
m_program     (nullptr),
m_program_ptr (0),
m_anim_construction_total(0),
m_anim_construction_done(0),
m_program_step(0)
{}

Immovable::~Immovable()
{
}

BaseImmovable::PositionList Immovable::get_positions
	(const EditorGameBase &) const
{
	PositionList rv;

	rv.push_back(m_position);
	return rv;
}

int32_t Immovable::get_size() const
{
	return descr().get_size();
}

bool Immovable::get_passable() const
{
	return descr().get_size() < BIG;
}

void Immovable::set_owner(Player * player)
{
	m_owner = player;
}

void Immovable::start_animation
	(const EditorGameBase & egbase, uint32_t const anim)
{
	m_anim      = anim;
	m_animstart = egbase.get_gametime();
	m_anim_construction_done = m_anim_construction_total = 0;
}


void Immovable::increment_program_pointer()
{
	m_program_ptr = (m_program_ptr + 1) % m_program->size();
	m_action_data.reset(nullptr);
}


/**
 * Actually initialize the immovable.
*/
void Immovable::init(EditorGameBase & egbase)
{
	BaseImmovable::init(egbase);

	set_position(egbase, m_position);

	//  Set animation data according to current program state.
	ImmovableProgram const * prog = m_program;
	if (!prog) {
		prog = descr().get_program("program");
		assert(prog != nullptr);
	}
	if (upcast(ImmovableProgram::ActAnimate const, act_animate, &(*prog)[m_program_ptr]))
		start_animation(egbase, act_animate->animation());

	if (upcast(Game, game, &egbase)) {
		switch_program(*game, "program");
	}
}


/**
 * Cleanup before destruction
*/
void Immovable::cleanup(EditorGameBase & egbase)
{
	unset_position(egbase, m_position);

	BaseImmovable::cleanup(egbase);
}


/**
 * Switch the currently running program.
*/
void Immovable::switch_program(Game& game, const std::string& program_name) {
	m_program = descr().get_program(program_name);
	assert(m_program != nullptr);
	m_program_ptr = 0;
	m_program_step = 0;
	m_action_data.reset(nullptr);
	schedule_act(game, 1);
}

/**
 * Run program timer.
*/
void Immovable::act(Game & game, uint32_t const data)
{
	BaseImmovable::act(game, data);

	if (m_program_step <= game.get_gametime()) {
		//  Might delete itself!
		(*m_program)[m_program_ptr].execute(game, *this);
	}
}


void Immovable::draw
	(const EditorGameBase& game, RenderTarget& dst, const FCoords&, const Point& pos)
{
	if (m_anim) {
		if (!m_anim_construction_total)
			dst.blit_animation(pos, m_anim, game.get_gametime() - m_animstart);
		else
			draw_construction(game, dst, pos);
	}
}

void Immovable::draw_construction
	(const EditorGameBase & game, RenderTarget & dst, const Point pos)
{
	const ImmovableProgram::ActConstruction * constructionact = nullptr;
	if (m_program_ptr < m_program->size())
		constructionact = dynamic_cast<const ImmovableProgram::ActConstruction *>
			(&(*m_program)[m_program_ptr]);

	const uint32_t steptime = constructionact ? constructionact->buildtime() : 5000;

	uint32_t done = 0;
	if (m_anim_construction_done > 0) {
		done = steptime * (m_anim_construction_done - 1);
		done += std::min(steptime, game.get_gametime() - m_animstart);
	}

	uint32_t total = m_anim_construction_total * steptime;
	if (done > total)
		done = total;

	const Animation& anim = g_gr->animations().get_animation(m_anim);
	const size_t nr_frames = anim.nr_frames();
	uint32_t frametime = g_gr->animations().get_animation(m_anim).frametime();
	uint32_t units_per_frame = (total + nr_frames - 1) / nr_frames;
	const size_t current_frame = done / units_per_frame;
	const uint16_t curw = anim.width();
	const uint16_t curh = anim.height();

	uint32_t lines = ((done % units_per_frame) * curh) / units_per_frame;

	assert(get_owner() != nullptr);  // Who would build something they do not own?
	const RGBColor& player_color = get_owner()->get_playercolor();
	if (current_frame > 0) {
		// Not the first pic, so draw the previous one in the back
		dst.blit_animation(pos, m_anim, (current_frame - 1) * frametime, player_color);
	}

	assert(lines <= curh);
	dst.blit_animation(pos, m_anim, current_frame * frametime, player_color,
	                   Rect(Point(0, curh - lines), curw, lines));

	// Additionally, if statistics are enabled, draw a progression string
	if (game.get_ibase()->get_display_flags() & InteractiveBase::dfShowStatistics) {
		unsigned int percent = (100 * done / total);
		m_construct_string =
			(boost::format("<font color=%s>%s</font>")
			 % UI_FONT_CLR_DARK.hex_value() % (boost::format(_("%i%% built")) % percent).str())
			 .str();
		m_construct_string = as_uifont(m_construct_string);
		dst.blit(pos - Point(0, 48),
				 UI::g_fh1->render(m_construct_string),
				 BlendMode::UseAlpha,
				 UI::Align::kCenter);
	}
}




/**
 * Set the current action's data to \p data.
 *
 * \warning \p data must not be equal to the currently set data, but it may be 0.
 */
void Immovable::set_action_data(ImmovableActionData * data)
{
	m_action_data.reset(data);
}


/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersionImmovable = 7;

// Supporting older versions for map loading
void Immovable::Loader::load(FileRead & fr, uint8_t const packet_version)
{
	BaseImmovable::Loader::load(fr);

	Immovable & imm = dynamic_cast<Immovable&>(*get_object());

	if (packet_version >= 5) {
		PlayerNumber pn = fr.unsigned_8();
		if (pn && pn <= MAX_PLAYERS) {
			Player * plr = egbase().get_player(pn);
			if (!plr)
				throw GameDataError("Immovable::load: player %u does not exist", pn);
			imm.set_owner(plr);
		}
	}

	// Position
	imm.m_position = read_coords_32(&fr, egbase().map().extent());
	imm.set_position(egbase(), imm.m_position);

	// Animation
	char const * const animname = fr.c_string();
	try {
		imm.m_anim = imm.descr().get_animation(animname);
	} catch (const MapObjectDescr::AnimationNonexistent &) {
		imm.m_anim = imm.descr().main_animation();
		log
			("Warning: (%s) Animation \"%s\" not found, using animation %s).\n",
			 imm.descr().name().c_str(), animname, imm.descr().get_animation_name(imm.m_anim).c_str());
	}
	imm.m_animstart = fr.signed_32();
	if (packet_version >= 4) {
		imm.m_anim_construction_total = fr.unsigned_32();
		if (imm.m_anim_construction_total)
			imm.m_anim_construction_done = fr.unsigned_32();
	}

	{ //  program
		std::string program_name;
		if (1 == packet_version) {
			program_name = fr.unsigned_8() ? fr.c_string() : "program";
			std::transform
				(program_name.begin(), program_name.end(), program_name.begin(),
				 tolower);
		} else {
			program_name = fr.c_string();
			if (program_name.empty())
				program_name = "program";
		}
		imm.m_program = imm.descr().get_program(program_name);
	}
	imm.m_program_ptr = fr.unsigned_32();

	if (!imm.m_program) {
		imm.m_program_ptr = 0;
	} else {
		if (imm.m_program_ptr >= imm.m_program->size()) {
			// Try to not fail if the program of some immovable has changed
			// significantly.
			// Note that in some cases, the immovable may end up broken despite
			// the fixup, but there isn't really anything we can do against that.
			log
				("Warning: Immovable '%s', size of program '%s' seems to have "
				 "changed.\n",
				 imm.descr().name().c_str(), imm.m_program->name().c_str());
			imm.m_program_ptr = 0;
		}
	}

	if (packet_version > 6) {
		imm.m_program_step = fr.unsigned_32();
	} else {
		imm.m_program_step = fr.signed_32();
	}

	if (packet_version >= 3 && packet_version <= 5) {
	        imm.m_reserved_by_worker = fr.unsigned_8();
	}
	if (packet_version >= 4) {
		std::string dataname = fr.c_string();
		if (!dataname.empty()) {
			imm.set_action_data(ImmovableActionData::load(fr, imm, dataname));
		}
	}
}

void Immovable::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();
}

void Immovable::Loader::load_finish()
{
	BaseImmovable::Loader::load_finish();

	Immovable & imm = dynamic_cast<Immovable &>(*get_object());
	if (upcast(Game, game, &egbase()))
		imm.schedule_act(*game, 1);

	egbase().inform_players_about_immovable
		(Map::get_index(imm.m_position, egbase().map().get_width()),
		 &imm.descr());
}

void Immovable::save
	(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw)
{
	// This is in front because it is required to obtain the description
	// necessary to create the Immovable
	fw.unsigned_8(HeaderImmovable);
	fw.unsigned_8(kCurrentPacketVersionImmovable);

	if (descr().owner_type() == MapObjectDescr::OwnerType::kTribe) {
		if (get_owner() == nullptr) log(" Tribe immovable has no owner!! ");
		fw.c_string("tribes");
	} else {
		fw.c_string("world");
	}

	fw.string(descr().name());

	// The main loading data follows
	BaseImmovable::save(egbase, mos, fw);

	fw.unsigned_8(get_owner() ? get_owner()->player_number() : 0);
	write_coords_32(&fw, m_position);

	// Animations
	fw.string(descr().get_animation_name(m_anim));
	fw.signed_32(m_animstart);
	fw.unsigned_32(m_anim_construction_total);
	if (m_anim_construction_total)
		fw.unsigned_32(m_anim_construction_done);

	// Program Stuff
	fw.string(m_program ? m_program->name() : "");

	fw.unsigned_32(m_program_ptr);
	fw.unsigned_32(m_program_step);

	if (m_action_data) {
	  fw.c_string(m_action_data->name());
	  m_action_data->save(fw, *this);
	} else {
		fw.c_string("");
	}
}

MapObject::Loader * Immovable::load
	(EditorGameBase & egbase, MapObjectLoader & mol, FileRead & fr,
	 const WorldLegacyLookupTable& world_lookup_table,
	 const TribesLegacyLookupTable& tribes_lookup_table)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		// Supporting older versions for map loading
		if (1 <= packet_version && packet_version <= kCurrentPacketVersionImmovable) {

			const std::string owner_type = fr.c_string();
			std::string name = fr.c_string();
			Immovable * imm = nullptr;

			if (owner_type != "world") { //  It is a tribe immovable.
				// Needed for map compatibility
				if (packet_version < 7) {
					name = tribes_lookup_table.lookup_immovable(owner_type, name);
				}
				const DescriptionIndex idx = egbase.tribes().immovable_index(name);
				if (idx != Widelands::INVALID_INDEX) {
					imm = new Immovable(*egbase.tribes().get_immovable_descr(idx));
				} else {
					throw GameDataError
						("tribes do not define immovable type \"%s\"", name.c_str());
				}
			} else { //  world immovable
				const World & world = egbase.world();
				name = world_lookup_table.lookup_immovable(name);
				const DescriptionIndex idx = world.get_immovable_index(name.c_str());
				if (idx == Widelands::INVALID_INDEX) {
					throw GameDataError
						("world does not define immovable type \"%s\"", name.c_str());
				}
				imm = new Immovable(*world.get_immovable_descr(idx));
			}

			loader->init(egbase, mol, *imm);
			loader->load(fr, packet_version);
		} else {
			throw UnhandledVersionError("Immovable", packet_version, kCurrentPacketVersionImmovable);
		}
	} catch (const std::exception & e) {
		throw wexception("immovable type: %s", e.what());
	}

	return loader.release();
}


ImmovableProgram::Action::~Action() {}

ImmovableProgram::ActAnimate::ActAnimate(char* parameters, ImmovableDescr& descr) {
	try {
		bool reached_end;
		char * const animation_name = next_word(parameters, reached_end);
		if (!descr.is_animation_known(animation_name)) {
			throw GameDataError("Unknown animation: %s.", animation_name);
		}
		m_id = descr.get_animation(animation_name);

		if (!reached_end) { //  The next parameter is the duration.
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp || value <= 0)
				throw GameDataError("expected %s but found \"%s\"", "duration in ms", parameters);
			m_duration = value;
		} else {
			m_duration = 0; //  forever
		}
	} catch (const WException & e) {
		throw GameDataError("animate: %s", e.what());
	}
}


/// Use convolutuion to make the animation time a random variable with binomial
/// distribution and the configured time as the expected value.
void ImmovableProgram::ActAnimate::execute
	(Game & game, Immovable & immovable) const
{
	immovable.start_animation(game, m_id);
	immovable.program_step(
	   game, m_duration ? 1 + game.logic_rand() % m_duration + game.logic_rand() % m_duration : 0);
}


ImmovableProgram::ActPlayFX::ActPlayFX(char* parameters, const ImmovableDescr&) {
	try {
		bool reached_end;
		name = next_word(parameters, reached_end);

		if (!reached_end) {
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			priority = value;
			if (*endp || priority != value)
				throw GameDataError
					("expected %s but found \"%s\"", "priority", parameters);
		} else
			priority = 127;

		g_sound_handler.load_fx_if_needed(FileSystem::fs_dirname(name),
													 FileSystem::fs_filename(name.c_str()),
													 name);
	} catch (const WException & e) {
		throw GameDataError("playFX: %s", e.what());
	}
}

/** Demand from the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played
 * is decided only by the sound server*/
void ImmovableProgram::ActPlayFX::execute
	(Game & game, Immovable & immovable) const
{
	g_sound_handler.play_fx(name, immovable.get_position(), priority);
	immovable.program_step(game);
}


ImmovableProgram::ActTransform::ActTransform
	(char * parameters, ImmovableDescr & descr)
{
	try {
		tribe = true;
		bob = false;
		probability = 0;

		std::vector<std::string> params = split_string(parameters, " ");
		for (uint32_t i = 0; i < params.size(); ++i) {
			if (params[i] == "bob")
				bob = true;
			else if (params[i] == "immovable")
				bob = false;
			else if (params[i][0] >= '0' && params[i][0] <= '9') {
				long int const value = atoi(params[i].c_str());
				if (value < 1 || 254 < value)
					throw GameDataError
						("expected %s but found \"%s\"", "probability in range [1, 254]",
						 params[i].c_str());
				probability = value;
			} else {
				std::vector<std::string> segments = split_string(params[i], ":");

				if (segments.size() > 2)
					throw GameDataError("object type has more than 2 segments");
				if (segments.size() == 2) {
					if (segments[0] == "world")
						tribe = false;
					else if (segments[0] == "tribe") {
						if (descr.owner_type() != MapObjectDescr::OwnerType::kTribe)
							throw GameDataError("scope \"tribe\" does not match the immovable type");
						tribe = true;
					} else
						throw GameDataError
							(
							 "unknown scope \"%s\" given for target type (must be "
							 "\"world\" or \"tribe\")",
							 parameters);

					type_name = segments[1];
				} else {
					type_name = segments[0];
				}
			}
		}
		if (type_name == descr.name())
			throw GameDataError("illegal transformation to the same type");
	} catch (const WException & e) {
		throw GameDataError("transform: %s", e.what());
	}
}

void ImmovableProgram::ActTransform::execute
	(Game & game, Immovable & immovable) const
{
	if (probability == 0 || game.logic_rand() % 256 < probability) {
		Player * player = immovable.get_owner();
		Coords const c = immovable.get_position();
		MapObjectDescr::OwnerType owner_type = immovable.descr().owner_type();
		immovable.remove(game); //  Now immovable is a dangling reference!

		if (bob) {
			game.create_ship(c, type_name, player);
		} else {
			Immovable & imm = game.create_immovable(c, type_name, owner_type);
			if (player)
				imm.set_owner(player);
		}
	} else
		immovable.program_step(game);
}


ImmovableProgram::ActGrow::ActGrow
	(char * parameters, ImmovableDescr & descr)
{
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'grow', but has no terrain_affinity entry.", descr.name().c_str());
	}

	try {
		tribe = true;
		for (char * p = parameters;;)
			switch (*p) {
			case ':': {
				*p = '\0';
				++p;
				if (descr.owner_type() != MapObjectDescr::OwnerType::kTribe)
					throw GameDataError
						(
						 "immovable type not in tribes but target type has scope "
						 "(\"%s\")",
						 parameters);
				else if (strcmp(parameters, "world"))
					throw GameDataError
						(
						 "scope \"%s\" given for target type (must be "
						 "\"world\")",
						 parameters);
				tribe = false;
				parameters = p;
				break;
			}
			case '\0':
				goto end;
			default:
				++p;
				break;
			}
	end:
		type_name = parameters;
	} catch (const WException & e) {
		throw GameDataError("grow: %s", e.what());
	}
}

void ImmovableProgram::ActGrow::execute(Game& game, Immovable& immovable) const {
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if (logic_rand_as_double(&game) <
		 probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains())) {
		MapObjectDescr::OwnerType owner_type = descr.owner_type();
		immovable.remove(game);  //  Now immovable is a dangling reference!
		game.create_immovable(f, type_name, owner_type);
	} else {
		immovable.program_step(game);
	}
}

/**
 * remove
*/
ImmovableProgram::ActRemove::ActRemove(char * parameters, ImmovableDescr &)
{
	try {
		if (*parameters) {
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp || value < 1 || 254 < value)
				throw GameDataError
					("expected %s but found \"%s\"",
					 "probability in range [1, 254]", parameters);
			probability = value;
		} else
			probability = 0;
	} catch (const WException & e) {
		throw GameDataError("remove: %s", e.what());
	}
}

void ImmovableProgram::ActRemove::execute
	(Game & game, Immovable & immovable) const
{
	if (probability == 0 || game.logic_rand() % 256 < probability)
		immovable.remove(game); //  Now immovable is a dangling reference!
	else
		immovable.program_step(game);
}

ImmovableProgram::ActSeed::ActSeed(char * parameters, ImmovableDescr & descr)
{
	try {
		probability = 0;
		for (char * p = parameters;;)
			switch (*p) {
			case ':': {
				*p = '\0';
				++p;
				if (descr.owner_type() != MapObjectDescr::OwnerType::kTribe)
					throw GameDataError
						(
						 "immovable type not in tribes but target type has scope "
						 "(\"%s\")",
						 parameters);
				else if (strcmp(parameters, "world"))
					throw GameDataError
						(
						 "scope \"%s\" given for target type (must be "
						 "\"world\")",
						 parameters);
				parameters = p;
				break;
			}
			case ' ': {
				*p = '\0';
				++p;
				char * endp;
				long int const value = strtol(p, &endp, 0);
				if (*endp || value < 1 || 254 < value)
					throw GameDataError
						("expected %s but found \"%s\"", "probability in range [1, 254]",
						 p);
				probability = value;
			//  fallthrough
			}
			/* no break */
			case '\0':
				goto end;
			default:
				++p;
				break;
			}
	end:
		type_name = parameters;
	} catch (const WException & e) {
		throw GameDataError("seed: %s", e.what());
	}
}

void ImmovableProgram::ActSeed::execute
	(Game & game, Immovable & immovable) const
{
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if (logic_rand_as_double(&game) <
		probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains())) {
		// Seed a new tree.
		MapFringeRegion<> mr(map, Area<>(f, 0));
		uint32_t fringe_size = 0;
		do {
			mr.extend(map);
			fringe_size += 6;
		} while (game.logic_rand() % std::numeric_limits<uint8_t>::max() < probability);

		for (uint32_t n = game.logic_rand() % fringe_size; n; --n) {
			mr.advance(map);
		}

		const FCoords new_location = map.get_fcoords(mr.location());
		if (!new_location.field->get_immovable() &&
		    (new_location.field->nodecaps() & MOVECAPS_WALK) &&
		    logic_rand_as_double(&game) <
		       probability_to_grow(
		          descr.terrain_affinity(), new_location, map, game.world().terrains())) {
			game.create_immovable(mr.location(), type_name, descr.owner_type());
		}
	}

	immovable.program_step(game);
}

ImmovableProgram::ActConstruction::ActConstruction(char* parameters, ImmovableDescr& descr) {
	try {
		if (descr.owner_type() != MapObjectDescr::OwnerType::kTribe)
			throw GameDataError("only usable for tribe immovable");

		std::vector<std::string> params = split_string(parameters, " ");

		if (params.size() != 3)
			throw GameDataError("usage: animation-name buildtime decaytime");

		m_buildtime = atoi(params[1].c_str());
		m_decaytime = atoi(params[2].c_str());

		std::string animation_name = params[0];
		if (!descr.is_animation_known(animation_name)) {
			throw GameDataError("unknown animation \"%s\" in immovable program for immovable \"%s\"",
									  animation_name.c_str(), descr.name().c_str());
		}
		m_animid = descr.get_animation(animation_name);

	} catch (const WException & e) {
		throw GameDataError("construction: %s", e.what());
	}
}

constexpr uint8_t kCurrentPacketVersionConstructionData = 1;

struct ActConstructionData : ImmovableActionData {
	const char * name() const override {return "construction";}
	void save(FileWrite & fw, Immovable & imm) override {
		fw.unsigned_8(kCurrentPacketVersionConstructionData);
		delivered.save(fw, imm.get_owner()->tribe());
	}

	static ActConstructionData * load(FileRead & fr, Immovable & imm) {
		ActConstructionData * d = new ActConstructionData;

		try {
			uint8_t packet_version = fr.unsigned_8();
			if (packet_version == kCurrentPacketVersionConstructionData) {
				d->delivered.load(fr, imm.get_owner()->tribe());
			} else {
				throw UnhandledVersionError("ActConstructionData",
													 packet_version, kCurrentPacketVersionConstructionData);
			}
		} catch (const WException & e) {
			delete d;
			d = nullptr;
			throw GameDataError("ActConstructionData: %s", e.what());
		}

		return d;
	}

	Buildcost delivered;
};

void ImmovableProgram::ActConstruction::execute(Game & g, Immovable & imm) const
{
	ActConstructionData * d = imm.get_action_data<ActConstructionData>();
	if (!d) {
		// First execution
		d = new ActConstructionData;
		imm.set_action_data(d);

		imm.start_animation(g, m_animid);
		imm.m_anim_construction_total = imm.descr().buildcost().total();
	} else {
		// Perhaps we are called due to the construction timeout of the last construction step
		Buildcost remaining;
		imm.construct_remaining_buildcost(g, &remaining);
		if (remaining.empty()) {
			imm.program_step(g);
			return;
		}

		// Otherwise, this is a decay timeout
		uint32_t totaldelivered = 0;
		for (Buildcost::const_iterator it = d->delivered.begin(); it != d->delivered.end(); ++it)
			totaldelivered += it->second;

		if (!totaldelivered) {
			imm.remove(g);
			return;
		}

		uint32_t randdecay = g.logic_rand() % totaldelivered;
		for (Buildcost::iterator it = d->delivered.begin(); it != d->delivered.end(); ++it) {
			if (randdecay < it->second) {
				it->second--;
				break;
			}

			randdecay -= it->second;
		}

		imm.m_anim_construction_done = d->delivered.total();
	}

	imm.m_program_step = imm.schedule_act(g, m_decaytime);
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * compute the remaining buildcost.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_remaining_buildcost(Game & /* game */, Buildcost * buildcost)
{
	ActConstructionData * d = get_action_data<ActConstructionData>();
	if (!d)
		return false;

	const Buildcost & total = descr().buildcost();
	for (Buildcost::const_iterator it = total.begin(); it != total.end(); ++it) {
		uint32_t delivered = d->delivered[it->first];
		if (delivered < it->second)
			(*buildcost)[it->first] = it->second - delivered;
	}

	return true;
}

/**
 * For an immovable that is currently in construction mode, return \c true and
 * consume the given ware type as delivered.
 *
 * If the immovable is not currently in construction mode, return \c false.
 */
bool Immovable::construct_ware(Game & game, DescriptionIndex index)
{
	ActConstructionData * d = get_action_data<ActConstructionData>();
	if (!d)
		return false;

	molog("construct_ware: index %u", index);

	Buildcost::iterator it = d->delivered.find(index);
	if (it != d->delivered.end())
		it->second++;
	else
		d->delivered[index] = 1;

	m_anim_construction_done = d->delivered.total();
	m_animstart = game.get_gametime();

	molog("construct_ware: total %u delivered: %u", index, d->delivered[index]);

	Buildcost remaining;
	construct_remaining_buildcost(game, &remaining);

	const ImmovableProgram::ActConstruction * action =
		dynamic_cast<const ImmovableProgram::ActConstruction *>(&(*m_program)[m_program_ptr]);
	assert(action != nullptr);

	if (remaining.empty()) {
		// Wait for the last building animation to finish.
		m_program_step = schedule_act(game, action->buildtime());
	} else {
		m_program_step = schedule_act(game, action->decaytime());
	}

	return true;
}


ImmovableActionData * ImmovableActionData::load(FileRead & fr, Immovable & imm, const std::string & name)
{
	if (name == "construction")
		return ActConstructionData::load(fr, imm);
	else {
		log("ImmovableActionData::load: type %s not known", name.c_str());
		return nullptr;
	}
}


/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize
*/
PlayerImmovable::PlayerImmovable(const MapObjectDescr & mo_descr) :
	BaseImmovable(mo_descr), m_owner(nullptr), m_economy(nullptr)
{}

/**
 * Cleanup
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log
			("PlayerImmovable::~PlayerImmovable: %lu workers left!\n",
			 static_cast<long unsigned int>(m_workers.size()));
}

/**
 * Change the economy, transfer the workers
*/
void PlayerImmovable::set_economy(Economy * const e)
{
	if (m_economy == e)
		return;

	for (uint32_t i = 0; i < m_workers.size(); ++i)
		m_workers[i]->set_economy(e);

	m_economy = e;
}

/**
 * Associate the given worker with this immovable.
 * The worker will be transferred along to another economy, and it will be
 * released when the immovable is destroyed.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::add_worker(Worker & w)
{
	m_workers.push_back(&w);
}

/**
 * Disassociate the given worker with this building.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::remove_worker(Worker & w)
{
	for (Workers::iterator worker_iter = m_workers.begin(); worker_iter != m_workers.end(); ++worker_iter)
		if (*worker_iter == &w) {
				*worker_iter = *(m_workers.end() - 1);
			return m_workers.pop_back();
		}

	throw wexception("PlayerImmovable::remove_worker: not in list");
}


/**
 * Set the immovable's owner. Currently, it can only be set once.
*/
void PlayerImmovable::set_owner(Player * const new_owner) {
	assert(m_owner == nullptr);

	m_owner = new_owner;

	Notifications::publish(NoteImmovable(this, NoteImmovable::Ownership::GAINED));
}

/**
 * Initialize the immovable.
*/
void PlayerImmovable::init(EditorGameBase & egbase)
{
	BaseImmovable::init(egbase);
}

/**
 * Release workers
*/
void PlayerImmovable::cleanup(EditorGameBase & egbase)
{
	while (!m_workers.empty())
		m_workers[0]->set_location(nullptr);

	Notifications::publish(NoteImmovable(this, NoteImmovable::Ownership::LOST));

	BaseImmovable::cleanup(egbase);
}

/**
 * We are the destination of the given ware's transfer, which is not associated
 * with any request.
 */
void PlayerImmovable::receive_ware(Game &, DescriptionIndex ware)
{
	throw wexception
		("MO(%u): Received a ware(%u), do not know what to do with it",
		 serial(), ware);
}

/**
 * We are the destination of the given worker's transfer, which is not
 * associated with any request.
 */
void PlayerImmovable::receive_worker(Game &, Worker & worker)
{
	throw wexception
		("MO(%u): Received a worker(%u), do not know what to do with it",
		 serial(), worker.serial());
}


/**
 * Dump general information
 */
void PlayerImmovable::log_general_info(const EditorGameBase & egbase)
{
	BaseImmovable::log_general_info(egbase);

	molog("this: %p\n", this);
	molog("m_owner: %p\n", m_owner);
	molog("* player nr: %i\n", m_owner->player_number());
	molog("m_economy: %p\n", m_economy);
}

constexpr uint8_t kCurrentPacketVersionPlayerImmovable = 1;

PlayerImmovable::Loader::Loader()
{
}

void PlayerImmovable::Loader::load(FileRead & fr)
{
	BaseImmovable::Loader::load(fr);

	PlayerImmovable & imm = get<PlayerImmovable>();

	try {
		uint8_t packet_version = fr.unsigned_8();

		if (packet_version == kCurrentPacketVersionPlayerImmovable) {
			PlayerNumber owner_number = fr.unsigned_8();

			if (!owner_number || owner_number > egbase().map().get_nrplayers())
				throw GameDataError
					("owner number is %u but there are only %u players",
					 owner_number, egbase().map().get_nrplayers());

			Player * owner = egbase().get_player(owner_number);
			if (!owner)
				throw GameDataError("owning player %u does not exist", owner_number);

			imm.m_owner = owner;
		} else {
			throw UnhandledVersionError("PlayerImmovable", packet_version, kCurrentPacketVersionPlayerImmovable);
		}
	} catch (const std::exception & e) {
		throw wexception("loading player immovable: %s", e.what());
	}
}

void PlayerImmovable::save(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw)
{
	BaseImmovable::save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersionPlayerImmovable);
	fw.unsigned_8(owner().player_number());
}

}
