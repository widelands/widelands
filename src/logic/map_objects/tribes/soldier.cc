/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "logic/map_objects/tribes/soldier.h"

#include <cstdio>
#include <list>
#include <memory>

#include <boost/format.hpp>

#include "base/macros.h"
#include "base/math.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/findbob.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/attackable.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/mapviewpixelconstants.h"

namespace Widelands {

namespace {

constexpr int kRetreatWhenHealthDropsBelowThisPercentage = 50;
}  // namespace

SoldierDescr::SoldierDescr(const std::string& init_descname,
                           const LuaTable& table,
                           const EditorGameBase& egbase)
   : WorkerDescr(init_descname, MapObjectType::SOLDIER, table, egbase),
     health_(table.get_table("health")),
     attack_(table.get_table("attack")),
     defense_(table.get_table("defense")),
     evade_(table.get_table("evade")) {
	add_attribute(MapObject::Attribute::SOLDIER);

	// Battle animations
	// attack_success_*-> soldier is attacking and hit his opponent
	add_battle_animation(table.get_table("attack_success_w"), &attack_success_w_name_);
	add_battle_animation(table.get_table("attack_success_e"), &attack_success_e_name_);

	// attack_failure_*-> soldier is attacking and miss hit, defender evades
	add_battle_animation(table.get_table("attack_failure_w"), &attack_failure_w_name_);
	add_battle_animation(table.get_table("attack_failure_e"), &attack_failure_e_name_);

	// evade_success_* -> soldier is defending and opponent misses
	add_battle_animation(table.get_table("evade_success_w"), &evade_success_w_name_);
	add_battle_animation(table.get_table("evade_success_e"), &evade_success_e_name_);

	// evade_failure_* -> soldier is defending and opponent hits
	add_battle_animation(table.get_table("evade_failure_w"), &evade_failure_w_name_);
	add_battle_animation(table.get_table("evade_failure_e"), &evade_failure_e_name_);

	// die_*           -> soldier is dying
	add_battle_animation(table.get_table("die_w"), &die_w_name_);
	add_battle_animation(table.get_table("die_e"), &die_e_name_);
}

SoldierDescr::BattleAttribute::BattleAttribute(std::unique_ptr<LuaTable> table) {
	base = table->get_int("base");

	if (table->has_key("maximum")) {
		maximum = table->get_int("maximum");
		if (base > maximum) {
			throw GameDataError(
			   "Base %d is greater than maximum %d for a soldier's battle attribute.", base, maximum);
		}
	} else {
		maximum = base;
	}
	increase = table->get_int("increase_per_level");
	max_level = table->get_int("max_level");

	// Load Graphics
	std::vector<std::string> image_filenames =
	   table->get_table("pictures")->array_entries<std::string>();
	if (image_filenames.size() != max_level + 1) {
		throw GameDataError("Soldier needs to have %d pictures for battle attribute, but found %lu",
		                    max_level + 1, image_filenames.size());
	}
	for (const std::string& image_filename : image_filenames) {
		images.push_back(g_gr->images().get(image_filename));
	}
}

/**
 * Get random animation of specified type
 */
uint32_t SoldierDescr::get_rand_anim(Game& game, const char* const animation_name) const {
	std::string run = animation_name;

	if (strcmp(animation_name, "attack_success_w") == 0) {
		assert(!attack_success_w_name_.empty());
		uint32_t i = game.logic_rand() % attack_success_w_name_.size();
		run = attack_success_w_name_[i];
	}

	if (strcmp(animation_name, "attack_success_e") == 0) {
		assert(!attack_success_e_name_.empty());
		uint32_t i = game.logic_rand() % attack_success_e_name_.size();
		run = attack_success_e_name_[i];
	}

	if (strcmp(animation_name, "attack_failure_w") == 0) {
		assert(!attack_failure_w_name_.empty());
		uint32_t i = game.logic_rand() % attack_failure_w_name_.size();
		run = attack_failure_w_name_[i];
	}

	if (strcmp(animation_name, "attack_failure_e") == 0) {
		assert(!attack_failure_e_name_.empty());
		uint32_t i = game.logic_rand() % attack_failure_e_name_.size();
		run = attack_failure_e_name_[i];
	}

	if (strcmp(animation_name, "evade_success_w") == 0) {
		assert(!evade_success_w_name_.empty());
		uint32_t i = game.logic_rand() % evade_success_w_name_.size();
		run = evade_success_w_name_[i];
	}

	if (strcmp(animation_name, "evade_success_e") == 0) {
		assert(!evade_success_e_name_.empty());
		uint32_t i = game.logic_rand() % evade_success_e_name_.size();
		run = evade_success_e_name_[i];
	}

	if (strcmp(animation_name, "evade_failure_w") == 0) {
		assert(!evade_failure_w_name_.empty());
		uint32_t i = game.logic_rand() % evade_failure_w_name_.size();
		run = evade_failure_w_name_[i];
	}

	if (strcmp(animation_name, "evade_failure_e") == 0) {
		assert(!evade_failure_e_name_.empty());
		uint32_t i = game.logic_rand() % evade_failure_e_name_.size();
		run = evade_failure_e_name_[i];
	}
	if (strcmp(animation_name, "die_w") == 0) {
		assert(!die_w_name_.empty());
		uint32_t i = game.logic_rand() % die_w_name_.size();
		run = die_w_name_[i];
	}

	if (strcmp(animation_name, "die_e") == 0) {
		assert(!die_e_name_.empty());
		uint32_t i = game.logic_rand() % die_e_name_.size();
		run = die_e_name_[i];
	}
	if (!is_animation_known(run)) {
		log("Missing animation '%s' for soldier %s. Reverting to idle.\n", run.c_str(),
		    name().c_str());
		run = "idle";
	}
	return get_animation(run);
}

/**
 * Create a new soldier
 */
Bob& SoldierDescr::create_object() const {
	return *new Soldier(*this);
}

void SoldierDescr::add_battle_animation(std::unique_ptr<LuaTable> table,
                                        std::vector<std::string>* result) {
	for (const std::string& anim_name : table->array_entries<std::string>()) {
		if (!is_animation_known(anim_name)) {
			throw GameDataError("Trying to add unknown battle animation: %s", anim_name.c_str());
		}
		result->push_back(anim_name);
	}
}

/*
==============================

IMPLEMENTATION

==============================
*/

/// all done through init
Soldier::Soldier(const SoldierDescr& soldier_descr) : Worker(soldier_descr) {
	battle_ = nullptr;
	health_level_ = 0;
	attack_level_ = 0;
	defense_level_ = 0;
	evade_level_ = 0;

	current_health_ = get_max_health();
	retreat_health_ = 0;

	combat_walking_ = CD_NONE;
	combat_walkstart_ = 0;
	combat_walkend_ = 0;
}

void Soldier::init(EditorGameBase& egbase) {
	health_level_ = 0;
	attack_level_ = 0;
	defense_level_ = 0;
	evade_level_ = 0;
	retreat_health_ = 0;

	current_health_ = get_max_health();

	combat_walking_ = CD_NONE;
	combat_walkstart_ = 0;
	combat_walkend_ = 0;

	Worker::init(egbase);
}

void Soldier::cleanup(EditorGameBase& egbase) {
	Worker::cleanup(egbase);
}

bool Soldier::is_evict_allowed() {
	return !is_on_battlefield();
}

/*
 * Set this soldiers level. Automatically sets the new values
 */
void Soldier::set_level(uint32_t const health,
                        uint32_t const attack,
                        uint32_t const defense,
                        uint32_t const evade) {
	set_health_level(health);
	set_attack_level(attack);
	set_defense_level(defense);
	set_evade_level(evade);
}
void Soldier::set_health_level(const uint32_t health) {
	assert(health_level_ <= health);
	assert(health <= descr().get_max_health_level());

	uint32_t oldmax = get_max_health();

	health_level_ = health;

	uint32_t newmax = get_max_health();
	current_health_ = current_health_ * newmax / oldmax;
}
void Soldier::set_attack_level(const uint32_t attack) {
	assert(attack_level_ <= attack);
	assert(attack <= descr().get_max_attack_level());

	attack_level_ = attack;
}
void Soldier::set_defense_level(const uint32_t defense) {
	assert(defense_level_ <= defense);
	assert(defense <= descr().get_max_defense_level());

	defense_level_ = defense;
}
void Soldier::set_evade_level(const uint32_t evade) {
	assert(evade_level_ <= evade);
	assert(evade <= descr().get_max_evade_level());

	evade_level_ = evade;
}
void Soldier::set_retreat_health(const uint32_t retreat) {
	assert(retreat <= get_max_health());

	retreat_health_ = retreat;
}

uint32_t Soldier::get_level(TrainingAttribute const at) const {
	switch (at) {
	case TrainingAttribute::kHealth:
		return health_level_;
	case TrainingAttribute::kAttack:
		return attack_level_;
	case TrainingAttribute::kDefense:
		return defense_level_;
	case TrainingAttribute::kEvade:
		return evade_level_;
	case TrainingAttribute::kTotal:
		return health_level_ + attack_level_ + defense_level_ + evade_level_;
	default:
		NEVER_HERE();
	}
}

int32_t Soldier::get_training_attribute(TrainingAttribute const attr) const {
	switch (attr) {
	case TrainingAttribute::kHealth:
		return health_level_;
	case TrainingAttribute::kAttack:
		return attack_level_;
	case TrainingAttribute::kDefense:
		return defense_level_;
	case TrainingAttribute::kEvade:
		return evade_level_;
	case TrainingAttribute::kTotal:
		return health_level_ + attack_level_ + defense_level_ + evade_level_;
	default:
		return Worker::get_training_attribute(attr);
	}
}

uint32_t Soldier::get_max_health() const {
	return descr().get_base_health() + health_level_ * descr().get_health_incr_per_level();
}

uint32_t Soldier::get_min_attack() const {
	return descr().get_base_min_attack() + attack_level_ * descr().get_attack_incr_per_level();
}

uint32_t Soldier::get_max_attack() const {
	return descr().get_base_max_attack() + attack_level_ * descr().get_attack_incr_per_level();
}

uint32_t Soldier::get_defense() const {
	return descr().get_base_defense() + defense_level_ * descr().get_defense_incr_per_level();
}

uint32_t Soldier::get_evade() const {
	return descr().get_base_evade() + evade_level_ * descr().get_evade_incr_per_level();
}

//  Unsignedness ensures that we can only heal, not hurt through this method.
void Soldier::heal(const uint32_t health) {
	molog("[soldier] healing (%d+)%d/%d\n", health, current_health_, get_max_health());
	assert(health);
	assert(current_health_ < get_max_health());
	current_health_ += std::min(health, get_max_health() - current_health_);
	assert(current_health_ <= get_max_health());
}

/**
 * This only subs the specified number of health points, don't do anything more.
 */
void Soldier::damage(const uint32_t value) {
	assert(current_health_ > 0);

	molog("[soldier] damage %d(-%d)/%d\n", current_health_, value, get_max_health());
	if (current_health_ < value)
		current_health_ = 0;
	else
		current_health_ -= value;
}

/// Calculates the actual position to draw on from the base node position.
/// This function takes battling into account.
///
/// pos is the location, in pixels, of the node position_ (height is already
/// taken into account).
Vector2f Soldier::calc_drawpos(const EditorGameBase& game,
                               const Vector2f& field_on_dst,
                               const float scale) const {
	if (combat_walking_ == CD_NONE) {
		return Bob::calc_drawpos(game, field_on_dst, scale);
	}

	bool moving = false;
	Vector2f spos = field_on_dst, epos = field_on_dst;

	const float triangle_width = kTriangleWidth * scale;
	switch (combat_walking_) {
	case CD_WALK_W:
		moving = true;
		epos.x -= triangle_width / 4;
		break;
	case CD_WALK_E:
		moving = true;
		epos.x += triangle_width / 4;
		break;
	case CD_RETURN_W:
		moving = true;
		spos.x -= triangle_width / 4;
		break;
	case CD_RETURN_E:
		moving = true;
		spos.x += triangle_width / 4;
		break;
	case CD_COMBAT_W:
		moving = false;
		epos.x -= triangle_width / 4;
		break;
	case CD_COMBAT_E:
		moving = false;
		epos.x += triangle_width / 4;
		break;
	case CD_NONE:
		break;
	default:
		NEVER_HERE();
	}

	if (moving) {
		const float f = math::clamp(static_cast<float>(game.get_gametime() - combat_walkstart_) /
		                               (combat_walkend_ - combat_walkstart_),
		                            0.f, 1.f);
		assert(combat_walkstart_ <= game.get_gametime());
		assert(combat_walkstart_ < combat_walkend_);
		epos.x = f * epos.x + (1 - f) * spos.x;
	}
	return epos;
}

/*
 * Draw this soldier. This basically draws him as a worker, but add health points
 */
void Soldier::draw(const EditorGameBase& game,
                   const TextToDraw&,
                   const Vector2f& field_on_dst,
                   const float scale,
                   RenderTarget* dst) const {
	const uint32_t anim = get_current_anim();
	if (!anim) {
		return;
	}

	const Vector2f point_on_dst = calc_drawpos(game, field_on_dst, scale);
	draw_info_icon(
	   point_on_dst -
	      Vector2f(0.f, (g_gr->animations().get_animation(get_current_anim()).height() - 7) * scale),
	   scale, true, dst);
	draw_inner(game, point_on_dst, scale, dst);
}

/**
 * Draw the info icon (level indicators + health bar) for this soldier.
 */
void Soldier::draw_info_icon(Vector2f draw_position,
                             float scale,
                             const bool anchor_below,
                             RenderTarget* dst) const {
	// Since the graphics below are all pixel perfect and scaling them as floats
	// looks weird, we round to the nearest fullest integer.
	scale = std::round(scale);
	if (scale == 0.f) {
		return;
	}

	const Image* healthpic = get_health_level_pic();
	const Image* attackpic = get_attack_level_pic();
	const Image* defensepic = get_defense_level_pic();
	const Image* evadepic = get_evade_level_pic();

#ifndef NDEBUG
	// This function assumes stuff about our data files: level icons are all the
	// same size and this is smaller than the width of the healthbar. This
	// simplifies the drawing code below a lot. Before it had a lot of if () that
	// were never tested - since our data files never changed.
	const int dimension = attackpic->width();
	assert(attackpic->height() == dimension);
	assert(healthpic->width() == dimension);
	assert(healthpic->height() == dimension);
	assert(defensepic->width() == dimension);
	assert(defensepic->height() == dimension);
	assert(evadepic->width() == dimension);
	assert(evadepic->height() == dimension);
	assert(kSoldierHealthBarWidth > dimension);
#endif

	const float icon_size = healthpic->width();
	const float half_width = kSoldierHealthBarWidth;

	if (!anchor_below) {
		float totalwidth = 2 * half_width;
		float totalheight = 5.f + 2 * icon_size;
		draw_position.x += (totalwidth / 2.f) * scale;
		draw_position.y += (totalheight - 5.f) * scale;
	} else {
		draw_position.y -= 5.f * scale;
	}

	// Draw energy bar
	assert(get_max_health());
	const Rectf energy_outer(
	   draw_position - Vector2f(half_width, 0.f) * scale, half_width * 2.f * scale, 5.f * scale);
	dst->fill_rect(energy_outer, RGBColor(255, 255, 255));

	float health_width = 2.f * (half_width - 1.f) * current_health_ / get_max_health();
	Rectf energy_inner(
	   draw_position + Vector2f(-half_width + 1.f, 1.f) * scale, health_width * scale, 3 * scale);
	Rectf energy_complement(energy_inner.origin() + Vector2f(health_width, 0.f) * scale,
	                        (2 * (half_width - 1) - health_width) * scale, 3 * scale);

	const RGBColor& color = owner().get_playercolor();
	RGBColor complement_color;
	if (static_cast<uint32_t>(color.r) + color.g + color.b > 128 * 3) {
		complement_color = RGBColor(32, 32, 32);
	} else {
		complement_color = RGBColor(224, 224, 224);
	}

	dst->fill_rect(energy_inner, color);
	dst->fill_rect(energy_complement, complement_color);

	const auto draw_level_image = [icon_size, scale, &draw_position, dst](
	   const Vector2f& offset, const Image* image) {
		dst->blitrect_scale(
		   Rectf(draw_position + offset * icon_size * scale, icon_size * scale, icon_size * scale),
		   image, Recti(0, 0, icon_size, icon_size), 1.f, BlendMode::UseAlpha);
	};
	draw_level_image(Vector2f(-1.f, -2.f), attackpic);
	draw_level_image(Vector2f(0.f, -2.f), defensepic);
	draw_level_image(Vector2f(-1.f, -1.f), healthpic);
	draw_level_image(Vector2f(0.f, -1.f), evadepic);
}

/**
 * Compute the size of the info icon (level indicators + health bar) for soldiers of
 * the given tribe.
 */
void Soldier::calc_info_icon_size(const TribeDescr& tribe, uint32_t& w, uint32_t& h) {
	const SoldierDescr* soldierdesc =
	   static_cast<const SoldierDescr*>(tribe.get_worker_descr(tribe.soldier()));
	const Image* healthpic = soldierdesc->get_health_level_pic(0);
	const Image* attackpic = soldierdesc->get_attack_level_pic(0);
	const Image* defensepic = soldierdesc->get_defense_level_pic(0);
	const Image* evadepic = soldierdesc->get_evade_level_pic(0);
	uint16_t hpw = healthpic->width();
	uint16_t hph = healthpic->height();
	uint16_t atw = attackpic->width();
	uint16_t ath = attackpic->height();
	uint16_t dew = defensepic->width();
	uint16_t deh = defensepic->height();
	uint16_t evw = evadepic->width();
	uint16_t evh = evadepic->height();

	uint16_t animw;
	animw = kSoldierHealthBarWidth;

	w = std::max(std::max(atw + dew, hpw + evw), 2 * animw);
	h = 5 + std::max(hph + ath, evh + deh);
}

void Soldier::pop_task_or_fight(Game& game) {
	if (battle_)
		start_task_battle(game);
	else
		pop_task(game);
}

/**
 *
 *
 */
void Soldier::start_animation(EditorGameBase& egbase,
                              char const* const animname,
                              uint32_t const time) {
	molog("[soldier] starting animation %s", animname);
	Game& game = dynamic_cast<Game&>(egbase);
	return start_task_idle(game, descr().get_rand_anim(game, animname), time);
}

/**
 * \return \c true if this soldier is considered to be on the battlefield
 */
bool Soldier::is_on_battlefield() {
	return get_state(taskAttack) || get_state(taskDefense);
}

/**
 * \return \c true if this soldier is considered to be attacking the player
 */
bool Soldier::is_attacking_player(Game& game, Player& player) {
	State* state = get_state(taskAttack);
	if (state) {
		if (upcast(PlayerImmovable, imm, state->objvar1.get(game))) {
			return (imm->get_owner() == &player);
		}
	}
	return false;
}

Battle* Soldier::get_battle() {
	return battle_;
}

/**
 * Determine whether this soldier can be challenged by an opponent.
 *
 * Such a challenge might override a battle that the soldier is currently
 * walking towards, to avoid lockups when the combatants cannot reach
 * each other.
 */
bool Soldier::can_be_challenged() {
	if (current_health_ < 1) {  // Soldier is dead!
		return false;
	}
	if (!is_on_battlefield()) {
		return false;
	}
	if (!battle_) {
		return true;
	}
	return !battle_->locked(dynamic_cast<Game&>(owner().egbase()));
}

/**
 * Assign the soldier to a battle (may be zero).
 *
 * \note must only be called by the \ref Battle object
 */
void Soldier::set_battle(Game& game, Battle* const battle) {
	if (battle_ != battle) {
		battle_ = battle;
		send_signal(game, "battle");
	}
}

/**
 * Set a fallback task.
 */
void Soldier::init_auto_task(Game& game) {
	if (get_current_health() < 1) {
		molog("[soldier] init_auto_task: die\n");
		return start_task_die(game);
	}

	return Worker::init_auto_task(game);
}

struct FindNodeOwned {
	FindNodeOwned(PlayerNumber owner) : owner_(owner) {
	}
	bool accept(const Map&, const FCoords& coords) const {
		return (coords.field->get_owned_by() == owner_);
	}

private:
	PlayerNumber owner_;
};

/**
 * Leave our home building and single-mindedly try to attack
 * and conquer the given building.
 *
 * The following variables are used:
 * \li objvar1 the \ref Building we're attacking.
 */
Bob::Task const Soldier::taskAttack = {"attack", static_cast<Bob::Ptr>(&Soldier::attack_update),
                                       nullptr, static_cast<Bob::Ptr>(&Soldier::attack_pop), true};

void Soldier::start_task_attack(Game& game, Building& building) {
	push_task(game, taskAttack);
	State& state = top_state();
	state.objvar1 = &building;
	state.coords = building.get_position();
	state.ivar2 = 0;  // The return state 1=go home 2=go back in known land
	state.ivar3 = 0;  // Counts how often the soldier is blocked in a row

	state.ivar1 |= CF_RETREAT_WHEN_INJURED;
	set_retreat_health(kRetreatWhenHealthDropsBelowThisPercentage * get_max_health() / 100);

	// Injured soldiers are not allowed to attack
	if (get_retreat_health() > get_current_health()) {
		set_retreat_health(get_current_health());
	}
	molog("[attack] starting, retreat health: %d\n", get_retreat_health());
}

void Soldier::attack_update(Game& game, State& state) {
	std::string signal = get_signal();
	uint32_t defenders = 0;

	if (signal.size()) {
		if (signal == "battle" || signal == "wakeup" || signal == "sleep") {
			state.ivar3 = 0;
			signal_handled();
		} else if (signal == "blocked") {
			state.ivar3++;
			signal_handled();
		} else if (signal == "fail") {
			state.ivar3 = 0;
			signal_handled();
			if (state.objvar1.get(game)) {
				molog("[attack] failed to reach enemy\n");
				state.objvar1 = nullptr;
			} else {
				molog("[attack] unexpected fail\n");
				return pop_task(game);
			}
		} else if (signal == "location") {
			molog("[attack] Location destroyed\n");
			state.ivar3 = 0;
			signal_handled();
			if (state.ivar2 == 0) {
				state.ivar2 = 1;
			}
		} else {
			molog("[attack] cancelled by unexpected signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	} else {
		// no signals means no consecutive block -> we're not stuck anymore
		state.ivar3 = 0;
	}

	//  We are at enemy building flag, and a defender is coming, sleep until he
	// "wake up"s me
	if (signal == "sleep") {
		return start_task_idle(game, descr().get_animation("idle"), -1);
	}

	upcast(Building, location, get_location(game));
	upcast(Building, enemy, state.objvar1.get(game));

	// Handle returns
	if (state.ivar2 > 0) {
		if (state.ivar2 == 1) {
			// Return home
			if (!location || !is_a(MilitarySite, location)) {
				molog("[attack] No more site to go back to\n");
				state.ivar2 = 2;
				return schedule_act(game, 10);
			}
			Flag& baseflag = location->base_flag();
			if (get_position() == baseflag.get_position()) {
				// At flag, enter building
				return start_task_move(
				   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware()), true);
			}
			if (get_position() == location->get_position()) {
				// At building, check if attack is required
				if (!enemy) {
					molog("[attack] returned home\n");
					return pop_task_or_fight(game);
				}
				state.ivar2 = 0;
				return start_task_leavebuilding(game, false);
			}
			// Head to home
			if (state.ivar3 > kBockCountIsStuck)
				molog("[attack] soldier is stuck, blocked nodes will be ignored\n");

			if (start_task_movepath(game, baseflag.get_position(),
			                        4,  // use larger persist when returning home
			                        descr().get_right_walk_anims(does_carry_ware()), false, -1,
			                        state.ivar3 > kBockCountIsStuck))
				return;
			else {
				molog("[attack] failed to return home\n");
				return pop_task(game);
			}
		}
		if (state.ivar2 == 2) {
			// No more home, so return to homeland
			upcast(Flag, flag, game.map().get_immovable(get_position()));
			if (flag && flag->get_owner() == get_owner()) {
				// At a flag
				molog("[attack] Returned to own flag\n");
				return pop_task(game);
			}
			Coords target;
			if (get_location(game)) {
				// We still have a location, head for the flag
				target = get_location(game)->base_flag().get_position();
				molog("[attack] Going back to our flag\n");
			} else {
				// No location
				if (get_position().field->get_owned_by() == get_owner()->player_number()) {
					// We are in our land, become fugitive
					molog("[attack] Back to our land\n");
					return pop_task(game);
				}
				// Try to find our land
				Map* map = game.get_map();
				std::vector<Coords> coords;
				uint32_t maxdist = descr().vision_range() * 2;
				Area<FCoords> area(map->get_fcoords(get_position()), maxdist);
				if (map->find_reachable_fields(area, &coords, CheckStepDefault(descr().movecaps()),
				                               FindNodeOwned(get_owner()->player_number()))) {
					// Found home land
					target = coords.front();
					molog("[attack] Going back to our land\n");
				} else {
					// Become fugitive
					molog("[attack] No land in sight\n");
					return pop_task(game);
				}
			}
			if (start_task_movepath(game, target,
			                        4,  // use larger persist when returning home
			                        descr().get_right_walk_anims(does_carry_ware())))
				return;
			else {
				molog("[attack] failed to return to own land\n");
				return pop_task(game);
			}
		}
	}

	if (battle_)
		return start_task_battle(game);

	if (signal == "blocked") {
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		// Note that we *should* be woken via send_space_signals,
		// so the timeout is just an additional safety net.
		return start_task_idle(game, descr().get_animation("idle"), 5000);
	}

	// Count remaining defenders
	if (enemy) {
		if (upcast(MilitarySite, ms, enemy)) {
			defenders = ms->present_soldiers().size();
		}
		if (upcast(Warehouse, wh, enemy)) {
			Requirements noreq;
			defenders = wh->count_workers(game, wh->owner().tribe().soldier(), noreq);
		}
		//  Any enemy soldier at baseflag count as defender.
		std::vector<Bob*> soldiers;
		game.map().find_bobs(
		   Area<FCoords>(game.map().get_fcoords(enemy->base_flag().get_position()), 0), &soldiers,
		   FindBobEnemySoldier(get_owner()));
		defenders += soldiers.size();
	}

	if (!enemy || (get_retreat_health() > get_current_health() && defenders > 0)) {
		// Injured soldiers will try to return to safe site at home.
		if (get_retreat_health() > get_current_health()) {
			assert(state.ivar1 & CF_RETREAT_WHEN_INJURED);
			if (defenders) {
				molog(" [attack] badly injured (%d), retreating...\n", get_current_health());
				state.coords = Coords::null();
				state.objvar1 = nullptr;
			}
		}
		// The old militarysite gets replaced by a new one, so if "enemy" is not
		// valid anymore, we either "conquered" the new building, or it was
		// destroyed.
		if (state.coords) {
			BaseImmovable* const newimm = game.map()[state.coords].get_immovable();
			upcast(MilitarySite, newsite, newimm);
			if (newsite && (&newsite->owner() == &owner())) {
				if (upcast(SoldierControl, ctrl, newsite)) {
					state.objvar1 = nullptr;
					// We may also have our location destroyed in between
					if (ctrl->stationed_soldiers().size() < ctrl->soldier_capacity() &&
					    (!location ||
					     location->base_flag().get_position() != newsite->base_flag().get_position())) {
						molog("[attack] enemy belongs to us now, move in\n");
						pop_task(game);
						set_location(newsite);
						newsite->update_soldier_request();
						return schedule_act(game, 10);
					}
				}
			}
		}
		// Return home
		state.ivar2 = 1;
		return schedule_act(game, 10);
	}

	// At this point, we know that the enemy building still stands,
	// and that we're outside in the plains.
	if (get_position() != enemy->base_flag().get_position()) {
		if (start_task_movepath(game, enemy->base_flag().get_position(), 3,
		                        descr().get_right_walk_anims(does_carry_ware())))
			return;
		else {
			molog("[attack] failed to move towards building flag, cancel attack "
			      "and return home!\n");
			state.coords = Coords::null();
			state.objvar1 = nullptr;
			state.ivar2 = 1;
			return schedule_act(game, 10);
		}
	}

	upcast(Attackable, attackable, enemy);
	assert(attackable);

	molog("[attack] attacking target building\n");
	//  give the enemy soldier some time to act
	schedule_act(game, attackable->attack(*this) ? 1000 : 10);
}

void Soldier::attack_pop(Game& game, State&) {
	if (battle_)
		battle_->cancel(game, *this);
}

/**
 * Accept Bob when is a Soldier alive that is attacking the Player.
 */
struct FindBobSoldierAttackingPlayer : public FindBob {
	FindBobSoldierAttackingPlayer(Game& g, Player& p) : player(p), game(g) {
	}

	bool accept(Bob* const bob) const override {
		if (upcast(Soldier, soldier, bob)) {
			return soldier->get_current_health() && soldier->is_attacking_player(game, player) &&
			       soldier->owner().is_hostile(player);
		}
		return false;
	}

	Player& player;
	Game& game;
};

/**
 * Soldiers with this task go out of his buildings. They will
 * try to find an enemy in his lands and go to hunt them down (signaling
 * "battle"). If no enemy was found inside our lands, but an enemy is found
 * outside our lands, then wait until the enemy goes inside or dissapear.
 * If no enemy is found, then return home.
 *
 * Variables used:
 * \li ivar1 used to store \c CombatFlags
 * \li ivar2 when CF_DEFEND_STAYHOME, 1 if it has reached the flag
//           when CF_RETREAT_WHEN_INJURED, the lesser health before fleeing
 */
Bob::Task const Soldier::taskDefense = {"defense", static_cast<Bob::Ptr>(&Soldier::defense_update),
                                        nullptr, static_cast<Bob::Ptr>(&Soldier::defense_pop),
                                        true};

void Soldier::start_task_defense(Game& game, bool stayhome) {
	molog("[defense] starting\n");
	push_task(game, taskDefense);
	State& state = top_state();

	state.ivar1 = 0;
	state.ivar2 = 0;

	// Here goes 'configuration'
	if (stayhome) {
		state.ivar1 |= CF_DEFEND_STAYHOME;
		set_retreat_health(0);
	} else {
		/* Flag defenders are not allowed to flee, to avoid abuses */
		state.ivar1 |= CF_RETREAT_WHEN_INJURED;
		set_retreat_health(get_max_health() * kRetreatWhenHealthDropsBelowThisPercentage / 100);

		// Soldier must defend even if he starts injured
		// (current health is below retreat treshold)
		if (get_retreat_health() > get_current_health()) {
			set_retreat_health(get_current_health());
		}
	}
	molog("[defense] retreat health set: %d\n", get_retreat_health());
}

struct SoldierDistance {
	Soldier* s;
	int dist;

	SoldierDistance(Soldier* a, int d) : dist(d) {
		s = a;
	}

	struct Greater {
		bool operator()(const SoldierDistance& a, const SoldierDistance& b) {
			return (a.dist > b.dist);
		}
	};
};

void Soldier::defense_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (signal.size()) {
		if (signal == "blocked" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog("[defense] cancelled by signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	PlayerImmovable* const location = get_location(game);
	BaseImmovable* const position = game.map()[get_position()].get_immovable();

	/**
	 * Attempt to fix a crash when player bulldozes a building being defended
	 * by soldiers.
	 */
	if (!location)
		return pop_task(game);

	Flag& baseflag = location->base_flag();

	if (battle_)
		return start_task_battle(game);

	if (signal == "blocked")
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		// Note that we *should* be woken via send_space_signals,
		// so the timeout is just an additional safety net.
		return start_task_idle(game, descr().get_animation("idle"), 5000);

	// If we only are defending our home ...
	if (state.ivar1 & CF_DEFEND_STAYHOME) {
		if (position == location && state.ivar2 == 1) {
			molog("[defense] stayhome: returned home\n");
			return pop_task_or_fight(game);
		}

		if (position == &baseflag) {
			state.ivar2 = 1;
			assert(state.ivar2 == 1);

			if (battle_)
				return start_task_battle(game);

			// Check if any attacker is waiting us to fight
			std::vector<Bob*> soldiers;
			game.map().find_bobs(
			   Area<FCoords>(get_position(), 0), &soldiers, FindBobEnemySoldier(get_owner()));

			for (Bob* temp_bob : soldiers) {
				if (upcast(Soldier, temp_soldier, temp_bob)) {
					if (temp_soldier->can_be_challenged()) {
						new Battle(game, *this, *temp_soldier);
						return start_task_battle(game);
					}
				}
			}

			if (state.ivar2 == 1) {
				molog("[defense] stayhome: return home\n");
				return start_task_return(game, false);
			}
		}

		molog("[defense] stayhome: leavebuilding\n");
		return start_task_leavebuilding(game, false);
	}

	// We are outside our building, get list of enemy soldiers attacking us
	std::vector<Bob*> soldiers;
	game.map().find_bobs(Area<FCoords>(get_position(), 10), &soldiers,
	                     FindBobSoldierAttackingPlayer(game, *get_owner()));

	if (soldiers.empty() || (get_current_health() < get_retreat_health())) {
		if (get_retreat_health() > get_current_health()) {
			assert(state.ivar1 & CF_RETREAT_WHEN_INJURED);
		}

		if (get_current_health() < get_retreat_health()) {
			molog("[defense] I am heavily injured (%d)!\n", get_current_health());
		} else
			molog("[defense] no enemy soldiers found, ending task\n");

		// If no enemy was found, return home
		if (!location) {
			molog("[defense] location disappeared during battle\n");
			return pop_task(game);
		}

		// Soldier is inside of building
		if (position == location) {
			molog("[defense] returned home\n");
			return pop_task_or_fight(game);
		}

		// Soldier is on base flag
		if (position == &baseflag) {
			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware()), true);
		}
		molog("[defense] return home\n");
		if (start_task_movepath(game, baseflag.get_position(),
		                        4,  // use larger persist when returning home
		                        descr().get_right_walk_anims(does_carry_ware())))
			return;

		molog("[defense] could not find way home\n");
		return pop_task(game);
	}

	// Go through soldiers
	std::vector<SoldierDistance> targets;
	for (Bob* temp_bob : soldiers) {

		// If enemy is in our land, then go after it!
		if (upcast(Soldier, soldier, temp_bob)) {
			assert(soldier != this);
			Field const f = game.map().operator[](soldier->get_position());

			//  Check soldier, be sure that we can fight against soldier.
			// Soldiers can not go over enemy land when defending.
			if ((soldier->can_be_challenged()) && (f.get_owned_by() == get_owner()->player_number())) {
				uint32_t thisDist = game.map().calc_distance(get_position(), soldier->get_position());
				targets.push_back(SoldierDistance(soldier, thisDist));
			}
		}
	}

	std::stable_sort(targets.begin(), targets.end(), SoldierDistance::Greater());

	while (!targets.empty()) {
		const SoldierDistance& target = targets.back();

		if (position == location) {
			return start_task_leavebuilding(game, false);
		}

		if (target.dist <= 1) {
			molog("[defense] starting battle with %u!\n", target.s->serial());
			new Battle(game, *this, *(target.s));
			return start_task_battle(game);
		}

		// Move towards soldier
		if (start_task_movepath(game, target.s->get_position(), 3,
		                        descr().get_right_walk_anims(does_carry_ware()), false, 1)) {
			molog("[defense] move towards soldier %u\n", target.s->serial());
			return;
		} else {
			molog("[defense] failed to move towards attacking soldier %u\n", target.s->serial());
			targets.pop_back();
		}
	}
	// If the enemy is not in our land, wait
	return start_task_idle(game, descr().get_animation("idle"), 250);
}

void Soldier::defense_pop(Game& game, State&) {
	if (battle_)
		battle_->cancel(game, *this);
}

Bob::Task const Soldier::taskMoveInBattle = {
   "moveInBattle", static_cast<Bob::Ptr>(&Soldier::move_in_battle_update), nullptr, nullptr, true};

void Soldier::start_task_move_in_battle(Game& game, CombatWalkingDir dir) {
	int32_t mapdir = IDLE;

	switch (dir) {
	case CD_WALK_W:
	case CD_RETURN_E:
		mapdir = WALK_W;
		break;
	case CD_WALK_E:
	case CD_RETURN_W:
		mapdir = WALK_E;
		break;
	case CD_NONE:
	case CD_COMBAT_E:
	case CD_COMBAT_W:
		throw GameDataError("bad direction '%d'", dir);
	}

	Map& map = game.map();
	int32_t const tdelta = (map.calc_cost(get_position(), mapdir)) / 2;
	molog("[move_in_battle] dir: (%d) tdelta: (%d)\n", dir, tdelta);
	combat_walking_ = dir;
	combat_walkstart_ = game.get_gametime();
	combat_walkend_ = combat_walkstart_ + tdelta;

	push_task(game, taskMoveInBattle);
	State& state = top_state();
	state.ivar1 = dir;
	set_animation(game, descr().get_animation(mapdir == WALK_E ? "walk_e" : "walk_w"));
}

void Soldier::move_in_battle_update(Game& game, State&) {
	if (game.get_gametime() >= combat_walkend_) {
		switch (combat_walking_) {
		case CD_NONE:
			break;
		case CD_WALK_W:
			combat_walking_ = CD_COMBAT_W;
			break;
		case CD_WALK_E:
			combat_walking_ = CD_COMBAT_E;
			break;
		case CD_RETURN_W:
		case CD_RETURN_E:
		case CD_COMBAT_W:
		case CD_COMBAT_E:
			combat_walking_ = CD_NONE;
			break;
		default:
			NEVER_HERE();
		}
		return pop_task(game);
	} else
		//  Only end the task once we've actually completed the step
		// Ignore signals until then
		return schedule_act(game, combat_walkend_ - game.get_gametime());
}

/**
 * \return \c true if the defending soldier should not stray from
 * his home flag.
 */
bool Soldier::stay_home() {
	if (State const* const state = get_state(taskDefense))
		return state->ivar1 & CF_DEFEND_STAYHOME;
	return false;
}

/**
 * We are out in the open and involved in a challenge/battle.
 * Meet with the other soldier and fight.
 */
Bob::Task const Soldier::taskBattle = {"battle", static_cast<Bob::Ptr>(&Soldier::battle_update),
                                       nullptr, static_cast<Bob::Ptr>(&Soldier::battle_pop), true};

void Soldier::start_task_battle(Game& game) {
	assert(battle_);
	combat_walking_ = CD_NONE;

	push_task(game, taskBattle);
}

void Soldier::battle_update(Game& game, State&) {
	std::string signal = get_signal();
	molog("[battle] update for player %u's soldier: signal = \"%s\"\n", owner().player_number(),
	      signal.c_str());

	if (signal.size()) {
		if (signal == "blocked") {
			signal_handled();
			return start_task_idle(game, descr().get_animation("idle"), 5000);
		} else if (signal == "location" || signal == "battle" || signal == "wakeup")
			signal_handled();
		else {
			molog("[battle] interrupted by unexpected signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	if (!battle_) {
		if (combat_walking_ == CD_COMBAT_W) {
			return start_task_move_in_battle(game, CD_RETURN_W);
		}
		if (combat_walking_ == CD_COMBAT_E) {
			return start_task_move_in_battle(game, CD_RETURN_E);
		}
		assert(combat_walking_ == CD_NONE);
		molog("[battle] is over\n");
		send_space_signals(game);
		return pop_task(game);
	}

	Map& map = game.map();
	Soldier& opponent = *battle_->opponent(*this);
	if (opponent.get_position() != get_position()) {
		if (is_a(Building, map[get_position()].get_immovable())) {
			// Note that this does not use the "leavebuilding" task,
			// because that task is geared towards orderly workers leaving
			// their location, whereas this case can also happen when
			// a player starts a construction site over a waiting soldier.
			molog("[battle] we are in a building, leave it\n");
			return start_task_move(
			   game, WALK_SE, descr().get_right_walk_anims(does_carry_ware()), true);
		}
	}

	if (stay_home()) {
		if (this == battle_->first()) {
			molog("[battle] stay_home, so reverse roles\n");
			new Battle(game, *battle_->second(), *battle_->first());
			return skip_act();  //  we will get a signal via set_battle()
		} else {
			if (combat_walking_ != CD_COMBAT_E) {
				opponent.send_signal(game, "wakeup");
				return start_task_move_in_battle(game, CD_WALK_E);
			}
		}
	} else {
		if (opponent.stay_home() && (this == battle_->second())) {
			// Wait until correct roles are assigned
			new Battle(game, *battle_->second(), *battle_->first());
			return schedule_act(game, 10);
		}

		if (opponent.get_position() != get_position()) {
			Coords dest = opponent.get_position();

			if (upcast(Building, building, map[dest].get_immovable()))
				dest = building->base_flag().get_position();

			uint32_t const dist = map.calc_distance(get_position(), dest);

			if (dist >= 2 || this == battle_->first()) {
				// Only make small steps at a time, so we can adjust to the
				// opponent's change of position.
				if (start_task_movepath(game, dest, 0, descr().get_right_walk_anims(does_carry_ware()),
				                        false, (dist + 3) / 4)) {
					molog("[battle] player %u's soldier started task_movepath to (%i,%i)\n",
					      owner().player_number(), dest.x, dest.y);
					return;
				} else {
					BaseImmovable const* const immovable_position =
					   get_position().field->get_immovable();
					BaseImmovable const* const immovable_dest = map[dest].get_immovable();

					const std::string messagetext =
					   (boost::format("The game engine has encountered a logic error. The %s "
					                  "#%u of player %u could not find a way from (%i, %i) "
					                  "(with %s immovable) to the opponent (%s #%u of player "
					                  "%u) at (%i, %i) (with %s immovable). The %s will now "
					                  "desert (but will not be executed). Strange things may "
					                  "happen. No solution for this problem has been "
					                  "implemented yet. (bug #536066) (The game has been "
					                  "paused.)") %
					    descr().descname().c_str() % serial() %
					    static_cast<unsigned int>(owner().player_number()) % get_position().x %
					    get_position().y %
					    (immovable_position ? immovable_position->descr().descname().c_str() : ("no")) %
					    opponent.descr().descname().c_str() % opponent.serial() %
					    static_cast<unsigned int>(opponent.owner().player_number()) % dest.x % dest.y %
					    (immovable_dest ? immovable_dest->descr().descname().c_str() : ("no")) %
					    descr().descname().c_str())
					      .str();
					owner().add_message(
					   game, *new Message(
					            Message::Type::kGameLogic, game.get_gametime(), descr().descname(),
					            "images/ui_basic/menu_help.png", _("Logic error"),
					            (boost::format("<rt><p font-size=12>%s</p></rt>") % messagetext).str(),
					            get_position(), serial_));
					opponent.owner().add_message(
					   game, *new Message(
					            Message::Type::kGameLogic, game.get_gametime(), descr().descname(),
					            "images/ui_basic/menu_help.png", _("Logic error"),
					            (boost::format("<rt><p font-size=12>%s</p></rt>") % messagetext).str(),
					            opponent.get_position(), serial_));
					game.game_controller()->set_desired_speed(0);
					return pop_task(game);
				}
			}
		} else {
			assert(opponent.get_position() == get_position());
			assert(battle_ == opponent.get_battle());

			if (opponent.is_walking()) {
				molog("[battle]: Opponent '%d' is walking, sleeping\n", opponent.serial());
				// We should be woken up by our opponent, but add a timeout anyway for robustness
				return start_task_idle(game, descr().get_animation("idle"), 5000);
			}

			if (battle_->first()->serial() == serial()) {
				if (combat_walking_ != CD_COMBAT_W) {
					molog("[battle]: Moving west\n");
					opponent.send_signal(game, "wakeup");
					return start_task_move_in_battle(game, CD_WALK_W);
				}
			} else {
				if (combat_walking_ != CD_COMBAT_E) {
					molog("[battle]: Moving east\n");
					opponent.send_signal(game, "wakeup");
					return start_task_move_in_battle(game, CD_WALK_E);
				}
			}
		}
	}

	battle_->get_battle_work(game, *this);
}

void Soldier::battle_pop(Game& game, State&) {
	if (battle_)
		battle_->cancel(game, *this);
}

Bob::Task const Soldier::taskDie = {"die", static_cast<Bob::Ptr>(&Soldier::die_update), nullptr,
                                    static_cast<Bob::Ptr>(&Soldier::die_pop), true};

void Soldier::start_task_die(Game& game) {
	push_task(game, taskDie);
	top_state().ivar1 = game.get_gametime() + 1000;

	// Dead soldier is not owned by a location
	set_location(nullptr);

	start_task_idle(
	   game, descr().get_animation(combat_walking_ == CD_COMBAT_W ? "die_w" : "die_e"), 1000);
}

void Soldier::die_update(Game& game, State& state) {
	std::string signal = get_signal();
	molog("[die] update for player %u's soldier: signal = \"%s\"\n", owner().player_number(),
	      signal.c_str());

	if (signal.size()) {
		signal_handled();
	}

	if ((state.ivar1 >= 0) && (static_cast<uint32_t>(state.ivar1) > game.get_gametime()))
		return schedule_act(game, state.ivar1 - game.get_gametime());

	// When task updated, dead is near!
	return pop_task(game);
}

void Soldier::die_pop(Game& game, State&) {
	// Destroy the soldier!
	molog("[die] soldier %u has died\n", serial());
	schedule_destroy(game);
}

/**
 * Accept a Bob if it is a Soldier, is not dead and is running taskAttack or
 * taskDefense.
 */
struct FindBobSoldierOnBattlefield : public FindBob {
	bool accept(Bob* const bob) const override {
		if (upcast(Soldier, soldier, bob))
			return soldier->is_on_battlefield() && soldier->get_current_health();
		return false;
	}
};

/**
 * Override \ref Bob::check_node_blocked.
 *
 * As long as we're on the battlefield, check for other soldiers.
 */
bool Soldier::check_node_blocked(Game& game, const FCoords& field, bool const commit) {
	State* attackdefense = get_state(taskAttack);

	if (!attackdefense)
		attackdefense = get_state(taskDefense);

	if (!attackdefense || ((attackdefense->ivar1 & CF_RETREAT_WHEN_INJURED) &&
	                       get_retreat_health() > get_current_health())) {
		// Retreating or non-combatant soldiers act like normal bobs
		return Bob::check_node_blocked(game, field, commit);
	}

	if (field.field->get_immovable() && field.field->get_immovable() == get_location(game)) {
		if (commit)
			send_space_signals(game);
		return false;  // we can always walk home
	}

	Soldier* foundsoldier = nullptr;
	bool foundbattle = false;
	bool foundopponent = false;
	bool multiplesoldiers = false;

	for (Bob* bob = field.field->get_first_bob(); bob; bob = bob->get_next_on_field()) {
		if (upcast(Soldier, soldier, bob)) {
			if (!soldier->is_on_battlefield() || !soldier->get_current_health())
				continue;

			if (!foundsoldier) {
				foundsoldier = soldier;
			} else {
				multiplesoldiers = true;
			}

			if (soldier->get_battle()) {
				foundbattle = true;

				if (battle_ && battle_->opponent(*this) == soldier)
					foundopponent = true;
			}
		}
	}

	if (!foundopponent && (foundbattle || foundsoldier)) {
		if (commit && !foundbattle && !multiplesoldiers) {
			if (foundsoldier->owner().is_hostile(*get_owner()) && foundsoldier->can_be_challenged()) {
				molog("[check_node_blocked] attacking a soldier (%u)\n", foundsoldier->serial());
				new Battle(game, *this, *foundsoldier);
			}
		}

		return true;
	} else {
		if (commit)
			send_space_signals(game);
		return false;
	}
}

/**
 * Send a "wakeup" signal to all surrounding soldiers that are out in the open,
 * so that they may repeat pathfinding.
 */
void Soldier::send_space_signals(Game& game) {
	std::vector<Bob*> soldiers;

	game.map().find_bobs(Area<FCoords>(get_position(), 1), &soldiers, FindBobSoldierOnBattlefield());

	for (Bob* temp_soldier : soldiers) {
		if (upcast(Soldier, soldier, temp_soldier)) {
			if (soldier != this) {
				soldier->send_signal(game, "wakeup");
			}
		}
	}

	PlayerNumber const land_owner = get_position().field->get_owned_by();
	if (land_owner != owner().player_number()) {
		std::vector<BaseImmovable*> attackables;
		game.map().find_reachable_immovables_unique(
		   Area<FCoords>(get_position(), MaxProtectionRadius), attackables,
		   CheckStepWalkOn(descr().movecaps(), false), FindImmovableAttackable());

		for (BaseImmovable* temp_attackable : attackables) {
			if (dynamic_cast<const PlayerImmovable&>(*temp_attackable).get_owner()->player_number() ==
			    land_owner) {
				dynamic_cast<Attackable&>(*temp_attackable).aggressor(*this);
			}
		}
	}
}

void Soldier::log_general_info(const EditorGameBase& egbase) {
	Worker::log_general_info(egbase);
	molog("[Soldier]\n");
	molog("Levels: %d/%d/%d/%d\n", health_level_, attack_level_, defense_level_, evade_level_);
	molog("Health:   %d/%d\n", current_health_, get_max_health());
	molog("Retreat:  %d\n", retreat_health_);
	molog("Attack:   %d-%d\n", get_min_attack(), get_max_attack());
	molog("Defense:  %d%%\n", get_defense());
	molog("Evade:    %d%%\n", get_evade());
	molog("CombatWalkingDir:   %i\n", combat_walking_);
	molog("CombatWalkingStart: %i\n", combat_walkstart_);
	molog("CombatWalkEnd:      %i\n", combat_walkend_);
	molog("HasBattle:   %s\n", battle_ ? "yes" : "no");
	if (battle_) {
		molog("BattleSerial: %u\n", battle_->serial());
		molog("Opponent: %u\n", battle_->opponent(*this)->serial());
	}
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 3;
// TODO(TiborB): This is only for map compatibility in regression tests, we should get rid of this
// ASAP
constexpr uint8_t kOldPacketVersion = 2;

Soldier::Loader::Loader() : battle_(0) {
}

void Soldier::Loader::load(FileRead& fr) {
	Worker::Loader::load(fr);

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion || packet_version == kOldPacketVersion) {

			Soldier& soldier = get<Soldier>();
			soldier.current_health_ = fr.unsigned_32();
			if (packet_version == kCurrentPacketVersion) {
				soldier.retreat_health_ = fr.unsigned_32();
			} else {
				// not ideal but will be used only for regression tests
				soldier.retreat_health_ = 0;
			}

			soldier.health_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_health_level());
			soldier.attack_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_attack_level());
			soldier.defense_level_ =
			   std::min(fr.unsigned_32(), soldier.descr().get_max_defense_level());
			soldier.evade_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_evade_level());

			if (soldier.current_health_ > soldier.get_max_health())
				soldier.current_health_ = soldier.get_max_health();

			if (soldier.retreat_health_ > soldier.get_max_health())
				soldier.retreat_health_ = soldier.get_max_health();

			soldier.combat_walking_ = static_cast<CombatWalkingDir>(fr.unsigned_8());
			if (soldier.combat_walking_ != CD_NONE) {
				soldier.combat_walkstart_ = fr.unsigned_32();
				soldier.combat_walkend_ = fr.unsigned_32();
			}

			battle_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("Soldier", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading soldier: %s", e.what());
	}
}

void Soldier::Loader::load_pointers() {
	Worker::Loader::load_pointers();

	Soldier& soldier = get<Soldier>();

	if (battle_)
		soldier.battle_ = &mol().get<Battle>(battle_);
}

const Bob::Task* Soldier::Loader::get_task(const std::string& name) {
	if (name == "attack")
		return &taskAttack;
	if (name == "defense")
		return &taskDefense;
	if (name == "battle")
		return &taskBattle;
	if (name == "moveInBattle")
		return &taskMoveInBattle;
	if (name == "die")
		return &taskDie;
	return Worker::Loader::get_task(name);
}

Soldier::Loader* Soldier::create_loader() {
	return new Loader;
}

void Soldier::do_save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	Worker::do_save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);
	fw.unsigned_32(current_health_);
	fw.unsigned_32(retreat_health_);
	fw.unsigned_32(health_level_);
	fw.unsigned_32(attack_level_);
	fw.unsigned_32(defense_level_);
	fw.unsigned_32(evade_level_);

	fw.unsigned_8(combat_walking_);
	if (combat_walking_ != CD_NONE) {
		fw.unsigned_32(combat_walkstart_);
		fw.unsigned_32(combat_walkend_);
	}

	fw.unsigned_32(mos.get_object_file_index_or_zero(battle_));
}
}
