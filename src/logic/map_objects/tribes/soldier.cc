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

#include "logic/map_objects/tribes/soldier.h"

#include <algorithm>
#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/rendertarget.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/mapregion.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/mapviewpixelconstants.h"

namespace Widelands {

constexpr int kSoldierHealthBarWidth = 13;

SoldierLevelRange::SoldierLevelRange(const LuaTable& t) {
	min_health = t.get_int("min_health");
	min_attack = t.get_int("min_attack");
	min_defense = t.get_int("min_defense");
	min_evade = t.get_int("min_evade");
	max_health = t.get_int("max_health");
	max_attack = t.get_int("max_attack");
	max_defense = t.get_int("max_defense");
	max_evade = t.get_int("max_evade");
}

bool SoldierLevelRange::matches(int32_t health,
                                int32_t attack,
                                int32_t defense,
                                int32_t evade) const {
	return (health >= min_health && health <= max_health && attack >= min_attack &&
	        attack <= max_attack && defense >= min_defense && defense <= max_defense &&
	        evade >= min_evade && evade <= max_evade);
}

bool SoldierLevelRange::matches(const Soldier* soldier) const {
	return matches(soldier->get_health_level(), soldier->get_attack_level(),
	               soldier->get_defense_level(), soldier->get_evade_level());
}

SoldierDescr::SoldierDescr(const std::string& init_descname,
                           const LuaTable& table,
                           Descriptions& descriptions)
   : WorkerDescr(init_descname, MapObjectType::SOLDIER, table, descriptions),
     health_(table.get_table("health")),
     attack_(table.get_table("attack")),
     defense_(table.get_table("defense")),
     evade_(table.get_table("evade")) {

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

	// per-level walking and idle animations
	add_battle_animation(table.get_table("idle"), &idle_name_);
	{
		std::unique_ptr<LuaTable> walk_table = table.get_table("walk");
		for (const auto& entry : walk_table->keys<int>()) {
			std::unique_ptr<LuaTable> range_table = walk_table->get_table(entry);
			// I would prefer to use the SoldierLevelRange as key in the table,
			// but LuaTable can handle only string keys :(
			std::unique_ptr<SoldierLevelRange> range(nullptr);
			std::map<uint8_t, std::string> map;
			for (const std::string& dir_name : range_table->keys<std::string>()) {
				uint8_t dir;
				if (dir_name == "range") {
					range.reset(new SoldierLevelRange(*range_table->get_table(dir_name)));
					continue;
				}
				if (dir_name == "sw") {
					dir = WALK_SW;
				} else if (dir_name == "se") {
					dir = WALK_SE;
				} else if (dir_name == "nw") {
					dir = WALK_NW;
				} else if (dir_name == "ne") {
					dir = WALK_NE;
				} else if (dir_name == "e") {
					dir = WALK_E;
				} else if (dir_name == "w") {
					dir = WALK_W;
				} else {
					throw GameDataError("Invalid walking direction: %s", dir_name.c_str());
				}
				const std::string anim_name = range_table->get_string(dir_name);
				if (!is_animation_known(anim_name)) {
					throw GameDataError(
					   "Trying to add unknown soldier walking animation: %s", anim_name.c_str());
				}
				map.emplace(dir, anim_name);
			}
			walk_name_.emplace(std::move(range), map);
		}
	}
}

SoldierDescr::BattleAttribute::BattleAttribute(std::unique_ptr<LuaTable> table) {
	base = table->get_int("base");

	if (table->has_key("maximum")) {
		maximum = table->get_int("maximum");
		if (base > maximum) {
			throw GameDataError(
			   "Base %u is greater than maximum %u for a soldier's battle attribute.", base, maximum);
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
		throw GameDataError(
		   "Soldier needs to have %u pictures for battle attribute, but found %" PRIuS, max_level + 1,
		   image_filenames.size());
	}
	for (const std::string& image_filename : image_filenames) {
		images.push_back(g_image_cache->get(image_filename));
	}
}

/**
 * Get random animation of specified type
 */
uint32_t SoldierDescr::get_rand_anim(Game& game,
                                     const std::string& animation_name,
                                     const Soldier* soldier) const {
	std::string run = animation_name;

	const SoldierAnimationsList* animations = nullptr;
	if (animation_name == "attack_success_w") {
		animations = &attack_success_w_name_;
	} else if (animation_name == "attack_success_e") {
		animations = &attack_success_e_name_;
	} else if (animation_name == "attack_failure_w") {
		animations = &attack_failure_w_name_;
	} else if (animation_name == "attack_failure_e") {
		animations = &attack_failure_e_name_;
	} else if (animation_name == "evade_success_w") {
		animations = &evade_success_w_name_;
	} else if (animation_name == "evade_success_e") {
		animations = &evade_success_e_name_;
	} else if (animation_name == "evade_failure_w") {
		animations = &evade_failure_w_name_;
	} else if (animation_name == "evade_failure_e") {
		animations = &evade_failure_e_name_;
	} else if (animation_name == "die_w") {
		animations = &die_w_name_;
	} else if (animation_name == "die_e") {
		animations = &die_e_name_;
	} else {
		throw GameDataError("Unknown soldier battle animation: %s", animation_name.c_str());
	}

	assert(!animations->empty());
	uint32_t nr_animations = 0;
	for (const auto& pair : *animations) {
		if (pair.second.matches(soldier)) {
			nr_animations++;
		}
	}
	if (nr_animations < 1) {
		throw GameDataError("No battle animations for %s found!", animation_name.c_str());
	}
	uint32_t i = game.logic_rand() % nr_animations;
	for (const auto& pair : *animations) {
		if (pair.second.matches(soldier)) {
			if (i == 0) {
				run = pair.first;
				break;
			}
			i--;
		}
	}

	if (!is_animation_known(run)) {
		log_warn_time(game.get_gametime(),
		              "Missing animation '%s' for soldier %s. Reverting to idle.\n", run.c_str(),
		              name().c_str());
		run = "idle";
	}
	return get_animation(run, soldier);
}

uint32_t SoldierDescr::get_animation(const std::string& anim, const MapObject* mo) const {
	const Soldier* soldier = dynamic_cast<const Soldier*>(mo);
	if ((soldier == nullptr) || anim != "idle") {
		// We only need to check for a level-dependent idle animation.
		// The walking anims can also be level-dependent,
		// but that is taken care of by get_right_walk_anims().
		// For battle animations, the level is already taken into account by the random selector.
		return WorkerDescr::get_animation(anim, mo);
	}
	for (const auto& pair : idle_name_) {
		if (pair.second.matches(soldier)) {
			// Use the parent method here, so we don't end up in
			// an endless loop if the idle anim is called "idle"
			return WorkerDescr::get_animation(pair.first, mo);
		}
	}
	throw GameDataError("This soldier does not have an idle animation for this training level!");
}

const DirAnimations& SoldierDescr::get_right_walk_anims(bool const ware, Worker* worker) const {
	Soldier* soldier = dynamic_cast<Soldier*>(worker);
	if (soldier == nullptr) {
		return WorkerDescr::get_right_walk_anims(ware, worker);
	}
	auto& cache = soldier->get_walking_animations_cache();
	if (cache.first && cache.first->matches(soldier)) {
		return *cache.second;
	}
	for (const auto& pair : walk_name_) {
		if (pair.first->matches(soldier)) {
			cache.first.reset(new SoldierLevelRange(*pair.first));
			cache.second.reset(new DirAnimations());
			for (uint8_t dir = 1; dir <= 6; ++dir) {
				cache.second->set_animation(dir, get_animation(pair.second.at(dir), worker));
			}
			return *cache.second;
		}
	}
	throw GameDataError(
	   "Soldier %s does not have walking animations for his level!", name().c_str());
}

/**
 * Create a new soldier
 */
Bob& SoldierDescr::create_object() const {
	return *new Soldier(*this);
}

void SoldierDescr::add_battle_animation(std::unique_ptr<LuaTable> table,
                                        SoldierAnimationsList* result) {
	for (const std::string& anim_name : table->keys<std::string>()) {
		// Store maximum height to prevent bobbing of the level icon
		uint32_t anim = get_animation(anim_name, nullptr);
		max_anim_height_ =
		   std::max(max_anim_height_,
		            static_cast<uint16_t>(g_animation_manager->get_animation(anim).height()));
		result->emplace(anim_name, SoldierLevelRange(*table->get_table(anim_name)));
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
	combat_walkstart_ = Time(0);
	combat_walkend_ = Time(0);
}

bool Soldier::init(EditorGameBase& egbase) {
	health_level_ = 0;
	attack_level_ = 0;
	defense_level_ = 0;
	evade_level_ = 0;
	retreat_health_ = 0;

	current_health_ = get_max_health();

	combat_walking_ = CD_NONE;
	combat_walkstart_ = Time(0);
	combat_walkend_ = Time(0);

	get_owner()->add_soldier(health_level_, attack_level_, defense_level_, evade_level_);

	return Worker::init(egbase);
}

void Soldier::cleanup(EditorGameBase& egbase) {
	get_owner()->remove_soldier(health_level_, attack_level_, defense_level_, evade_level_);
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

	get_owner()->remove_soldier(health_level_, attack_level_, defense_level_, evade_level_);
	health_level_ = health;
	get_owner()->add_soldier(health_level_, attack_level_, defense_level_, evade_level_);

	uint32_t newmax = get_max_health();
	current_health_ = current_health_ * newmax / oldmax;
}
void Soldier::set_attack_level(const uint32_t attack) {
	assert(attack_level_ <= attack);
	assert(attack <= descr().get_max_attack_level());

	get_owner()->remove_soldier(health_level_, attack_level_, defense_level_, evade_level_);
	attack_level_ = attack;
	get_owner()->add_soldier(health_level_, attack_level_, defense_level_, evade_level_);
}
void Soldier::set_defense_level(const uint32_t defense) {
	assert(defense_level_ <= defense);
	assert(defense <= descr().get_max_defense_level());

	get_owner()->remove_soldier(health_level_, attack_level_, defense_level_, evade_level_);
	defense_level_ = defense;
	get_owner()->add_soldier(health_level_, attack_level_, defense_level_, evade_level_);
}
void Soldier::set_evade_level(const uint32_t evade) {
	assert(evade_level_ <= evade);
	assert(evade <= descr().get_max_evade_level());

	get_owner()->remove_soldier(health_level_, attack_level_, defense_level_, evade_level_);
	evade_level_ = evade;
	get_owner()->add_soldier(health_level_, attack_level_, defense_level_, evade_level_);
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

unsigned Soldier::get_max_health() const {
	return descr().get_base_health() + health_level_ * descr().get_health_incr_per_level();
}

unsigned Soldier::get_min_attack() const {
	return descr().get_base_min_attack() + attack_level_ * descr().get_attack_incr_per_level();
}

unsigned Soldier::get_max_attack() const {
	return descr().get_base_max_attack() + attack_level_ * descr().get_attack_incr_per_level();
}

unsigned Soldier::get_defense() const {
	return descr().get_base_defense() + defense_level_ * descr().get_defense_incr_per_level();
}

unsigned Soldier::get_evade() const {
	return descr().get_base_evade() + evade_level_ * descr().get_evade_incr_per_level();
}

//  Unsignedness ensures that we can only heal, not hurt through this method.
void Soldier::heal(const unsigned health) {
	molog(owner().egbase().get_gametime(), "[soldier] healing (%u+)%u/%u\n", health, current_health_,
	      get_max_health());
	assert(health);
	assert(current_health_ < get_max_health());
	current_health_ += std::min(health, get_max_health() - current_health_);
	assert(current_health_ <= get_max_health());
}

/**
 * This only subs the specified number of health points, don't do anything more.
 */
void Soldier::damage(const unsigned value) {
	assert(current_health_ > 0);

	molog(owner().egbase().get_gametime(), "[soldier] damage %u(-%u)/%u\n", current_health_, value,
	      get_max_health());
	if (current_health_ < value) {
		current_health_ = 0;
	} else {
		current_health_ -= value;
	}
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
	Vector2f spos = field_on_dst;
	Vector2f epos = field_on_dst;

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
		const float f =
		   math::clamp(static_cast<float>(game.get_gametime().get() - combat_walkstart_.get()) /
		                  (combat_walkend_.get() - combat_walkstart_.get()),
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
                   const InfoToDraw& info_to_draw,
                   const Vector2f& field_on_dst,
                   const Coords& coords,
                   float scale,
                   RenderTarget* dst) const {
	const uint32_t anim = get_current_anim();
	if (anim == 0u) {
		return;
	}

	const Vector2f point_on_dst = calc_drawpos(game, field_on_dst, scale);
	draw_info_icon(
	   point_on_dst.cast<int>() - Vector2i(0, (descr().get_max_anim_height() - 7) * scale), scale,
	   InfoMode::kWalkingAround, info_to_draw, dst);
	draw_inner(game, point_on_dst, coords, scale, dst);
}

/**
 * Draw the info icon (level indicators + health bar) for this soldier.
 * 'draw_mode' determines whether the soldier info is displayed in a building window
 * or on top of a soldier walking around. 'info_to_draw' checks which info the user wants to see
 * for soldiers walking around.
 */
void Soldier::draw_info_icon(Vector2i draw_position,
                             float scale,
                             const InfoMode draw_mode,
                             const InfoToDraw info_to_draw,
                             RenderTarget* dst) const {
	if ((info_to_draw & InfoToDraw::kSoldierLevels) == 0) {
		return;
	}

	// Since the graphics below are all pixel perfect and scaling them as floats
	// looks weird, we round to the nearest fullest integer. We do allow half size though.
	scale = std::max(0.5f, std::round(scale));

#ifndef NDEBUG
	{
		// This function assumes stuff about our data files: level icons are all the
		// same size and this is smaller than the width of the healthbar. This
		// simplifies the drawing code below a lot. Before it had a lot of if () that
		// were never tested - since our data files never changed.
		const Image* healthpic = get_health_level_pic();

		const Image* attackpic = get_attack_level_pic();
		const Image* defensepic = get_defense_level_pic();
		const Image* evadepic = get_evade_level_pic();

		const int dimension = attackpic->width();
		assert(attackpic->height() == dimension);
		assert(healthpic->width() == dimension);
		assert(healthpic->height() == dimension);
		assert(defensepic->width() == dimension);
		assert(defensepic->height() == dimension);
		assert(evadepic->width() == dimension);
		assert(evadepic->height() == dimension);
		assert(kSoldierHealthBarWidth > dimension);
	}
#endif

	const int icon_size = get_health_level_pic()->height();

	// Draw health info in building windows, or if kSoldierLevels is on.
	const bool draw_health_bar =
	   draw_mode == InfoMode::kInBuilding || ((info_to_draw & InfoToDraw::kSoldierLevels) != 0);

	switch (draw_mode) {
	case InfoMode::kInBuilding:
		draw_position.x += kSoldierHealthBarWidth * scale;
		draw_position.y += 2 * icon_size * scale;
		break;
	case InfoMode::kWalkingAround:
		if (draw_health_bar) {
			draw_position.y -= 5 * scale;
		}
		break;
	default:
		NEVER_HERE();
	}

	if (draw_health_bar) {
		// Draw energy bar
		assert(get_max_health());
		const RGBColor& color = owner().get_playercolor();
		const uint16_t color_sum = color.r + color.g + color.b;

		// The frame gets a slight tint of player color
		const Recti energy_outer(draw_position - Vector2i(kSoldierHealthBarWidth, 0) * scale,
		                         kSoldierHealthBarWidth * 2 * scale, 5 * scale);
		dst->fill_rect(energy_outer, color);
		dst->brighten_rect(energy_outer, 230 - color_sum / 3);

		// Adjust health to current animation tick
		uint32_t health_to_show = current_health_;
		if (battle_ != nullptr) {
			uint32_t pending_damage = battle_->get_pending_damage(this);
			if (pending_damage > 0) {
				int32_t timeshift = owner().egbase().get_gametime().get() - get_animstart().get();
				timeshift = std::min(std::max(0, timeshift), 1000);

				pending_damage *= timeshift;
				pending_damage /= 1000;

				if (pending_damage > health_to_show) {
					health_to_show = 0;
				} else {
					health_to_show -= pending_damage;
				}
			}
		}

		// Now draw the health bar itself
		const int health_width = 2 * (kSoldierHealthBarWidth - 1) * health_to_show / get_max_health();

		Recti energy_inner(draw_position + Vector2i(-kSoldierHealthBarWidth + 1, 1) * scale,
		                   health_width * scale, 3 * scale);
		Recti energy_complement(energy_inner.origin() + Vector2i(health_width, 0) * scale,
		                        (2 * (kSoldierHealthBarWidth - 1) - health_width) * scale, 3 * scale);

		const RGBColor complement_color =
		   color_sum > 128 * 3 ? RGBColor(32, 32, 32) : RGBColor(224, 224, 224);
		dst->fill_rect(energy_inner, color);
		dst->fill_rect(energy_complement, complement_color);
	}

	// Draw level info in building windows, or if kSoldierLevels is on.
	if (draw_mode == InfoMode::kInBuilding || ((info_to_draw & InfoToDraw::kSoldierLevels) != 0)) {
		const Recti src_rect(0, 0, icon_size, icon_size);
		const auto draw_level_image = [icon_size, scale, &draw_position, dst, &src_rect](
		                                 const Vector2i& offset, const Image* image) {
			dst->blitrect_scale(
			   Rectf(draw_position + offset * icon_size * scale, icon_size * scale, icon_size * scale),
			   image, src_rect, 1.f, BlendMode::UseAlpha);
		};

		draw_level_image(Vector2i(-1, -2), get_attack_level_pic());
		draw_level_image(Vector2i(0, -2), get_defense_level_pic());
		draw_level_image(Vector2i(-1, -1), get_health_level_pic());
		draw_level_image(Vector2i(0, -1), get_evade_level_pic());
	}
}

/**
 * Compute the size of the info icon (level indicators + health bar) for soldiers of
 * the given tribe.
 */
void Soldier::calc_info_icon_size(const TribeDescr& tribe, int& w, int& h) {
	const SoldierDescr* soldierdesc =
	   dynamic_cast<const SoldierDescr*>(tribe.get_worker_descr(tribe.soldier()));
	// The function draw_info_icon() already assumes that all icons have the same dimensions,
	// so we can make the same assumption here too.
	const int dimension = soldierdesc->get_health_level_pic(0)->height();
	w = 2 * std::max(dimension, kSoldierHealthBarWidth);
	h = 5 + 2 * dimension;
}

void Soldier::pop_task_or_fight(Game& game) {
	if (battle_ != nullptr) {
		start_task_battle(game);
	} else {
		pop_task(game);
	}
}

/**
 *
 *
 */
void Soldier::start_animation(EditorGameBase& egbase,
                              const std::string& animname,
                              const Duration& time) {
	molog(egbase.get_gametime(), "[soldier] starting animation %s", animname.c_str());
	Game& game = dynamic_cast<Game&>(egbase);
	return start_task_idle(game, descr().get_rand_anim(game, animname, this), time.get());
}

/**
 * \return \c true if this soldier is considered to be on the battlefield
 */
bool Soldier::is_on_battlefield() {
	return (get_state(taskAttack) != nullptr) || (get_state(taskDefense) != nullptr) ||
	       (get_state(taskNavalInvasion) != nullptr);
}

/**
 * \return \c true if this soldier is considered to be attacking the player
 */
bool Soldier::is_attacking_player(Game& game, Player& player) {
	State* state = get_state(taskAttack);
	if (state != nullptr) {
		if (upcast(PlayerImmovable, imm, state->objvar1.get(game))) {
			return (imm->get_owner() == &player);
		}
	}
	return false;
}

Battle* Soldier::get_battle() const {
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
	if (battle_ == nullptr) {
		return true;
	}
	return !battle_->locked(dynamic_cast<Game&>(get_owner()->egbase()));
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
		molog(game.get_gametime(), "[soldier] init_auto_task: die\n");
		return start_task_die(game);
	}

	return Worker::init_auto_task(game);
}

/**
 * \return \c true if the defending soldier should not stray from
 * his home flag.
 */
bool Soldier::stay_home() {
	if (State const* const state = get_state(taskDefense)) {
		return (state->ivar1 & CF_DEFEND_STAYHOME) != 0;
	}
	return false;
}

/**
 * Accept a Bob if it is a Soldier, is not dead and is running taskAttack or
 * taskDefense.
 */
struct FindBobSoldierOnBattlefield : public FindBob {
	bool accept(Bob* const bob) const override {
		if (upcast(Soldier, soldier, bob)) {
			return soldier->is_on_battlefield() && (soldier->get_current_health() != 0u);
		}
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
	bool is_retreating = false;

	if (attackdefense == nullptr) {
		attackdefense = get_state(taskDefense);
	}

	if (attackdefense == nullptr) {
		attackdefense = get_state(taskNavalInvasion);
	} else {
		is_retreating = ((attackdefense->ivar1 & CF_RETREAT_WHEN_INJURED) != 0) &&
		                get_retreat_health() > get_current_health();
	}

	if (attackdefense == nullptr || is_retreating) {
		// Retreating or non-combatant soldiers act like normal bobs
		return Bob::check_node_blocked(game, field, commit);
	}

	if ((field.field->get_immovable() != nullptr) &&
	    field.field->get_immovable() == get_location(game)) {
		if (commit) {
			send_space_signals(game);
		}
		return false;  // we can always walk home
	}

	Soldier* foundsoldier = nullptr;
	bool foundbattle = false;
	bool foundopponent = false;
	bool multiplesoldiers = false;

	for (Bob* bob = field.field->get_first_bob(); bob != nullptr; bob = bob->get_next_on_field()) {
		if (bob == this || bob->descr().type() != MapObjectType::SOLDIER) {
			continue;
		}
		if (upcast(Soldier, soldier, bob)) {
			if (!soldier->is_on_battlefield() || (soldier->get_current_health() == 0u)) {
				continue;
			}

			if (soldier->get_state() != nullptr && soldier->get_state()->task == &taskNavalInvasion &&
			    get_state() != nullptr && get_state()->task == &taskNavalInvasion &&
			    soldier->get_owner() == get_owner()) {
				continue;
			}

			if (foundsoldier == nullptr) {
				foundsoldier = soldier;
			} else {
				multiplesoldiers = true;
			}

			if (soldier->get_battle() != nullptr && soldier->get_battle()->first() != nullptr &&
			    soldier->get_battle()->second() != nullptr &&
			    game.map().calc_distance(soldier->get_battle()->first()->get_position(),
			                             soldier->get_battle()->second()->get_position()) < 2) {
				foundbattle = true;

				if ((battle_ != nullptr) && battle_->opponent(*this) == soldier) {
					foundopponent = true;
				}
			}
		}
	}

	if (!foundopponent && (foundbattle || (foundsoldier != nullptr))) {
		if (commit && !foundbattle && !multiplesoldiers) {
			if (foundsoldier->owner().is_hostile(*get_owner()) && foundsoldier->can_be_challenged()) {
				molog(game.get_gametime(), "[check_node_blocked] attacking a soldier (%u)\n",
				      foundsoldier->serial());
				new Battle(game, this, foundsoldier);
			}
		}

		return true;
	}
	if (commit) {
		send_space_signals(game);
	}
	return false;
}

/**
 * Send a "wakeup" signal to all surrounding soldiers that are out in the open,
 * so that they may repeat pathfinding.
 */
void Soldier::send_space_signals(Game& game) {
	std::vector<Bob*> soldiers;

	game.map().find_bobs(
	   game, Area<FCoords>(get_position(), 1), &soldiers, FindBobSoldierOnBattlefield());

	for (Bob* temp_soldier : soldiers) {
		if (upcast(Soldier, soldier, temp_soldier)) {
			if (soldier != this) {
				soldier->send_signal(game, "wakeup");
			}
		}
	}

	PlayerNumber const land_owner = get_position().field->get_owned_by();
	// First check if the soldier is standing on someone else's territory
	if (land_owner != owner().player_number()) {
		// Let's collect all reachable attack_target sites in vicinity (militarysites mainly)
		std::vector<BaseImmovable*> attack_targets;
		game.map().find_reachable_immovables_unique(
		   game, Area<FCoords>(get_position(), kMaxProtectionRadius), attack_targets,
		   CheckStepWalkOn(descr().movecaps(), false), FindImmovableAttackTarget());

		for (BaseImmovable* temp_attack_target : attack_targets) {
			Building* building = dynamic_cast<Building*>(temp_attack_target);
			assert(building != nullptr && building->attack_target() != nullptr);
			const Player& attack_target_player = building->owner();
			// Let's inform the site that this (=enemy) soldier is nearby and within the site's owner's
			// territory
			if (attack_target_player.player_number() == land_owner &&
			    attack_target_player.is_hostile(*get_owner())) {
				building->attack_target()->enemy_soldier_approaches(*this);
			}
		}
	}
}

void Soldier::log_general_info(const EditorGameBase& egbase) const {
	Worker::log_general_info(egbase);
	molog(egbase.get_gametime(), "[Soldier]\n");
	molog(egbase.get_gametime(), "Levels: %u/%u/%u/%u\n", health_level_, attack_level_,
	      defense_level_, evade_level_);
	molog(egbase.get_gametime(), "Health:   %u/%u\n", current_health_, get_max_health());
	molog(egbase.get_gametime(), "Retreat:  %u\n", retreat_health_);
	molog(egbase.get_gametime(), "Attack:   %u-%u\n", get_min_attack(), get_max_attack());
	molog(egbase.get_gametime(), "Defense:  %u%%\n", get_defense());
	molog(egbase.get_gametime(), "Evade:    %u%%\n", get_evade());
	molog(egbase.get_gametime(), "CombatWalkingDir:   %i\n", combat_walking_);
	molog(egbase.get_gametime(), "CombatWalkingStart: %u\n", combat_walkstart_.get());
	molog(egbase.get_gametime(), "CombatWalkEnd:      %u\n", combat_walkend_.get());
	molog(egbase.get_gametime(), "HasBattle:   %s\n", battle_ != nullptr ? "yes" : "no");
	if (battle_ != nullptr) {
		molog(egbase.get_gametime(), "BattleSerial: %u\n", battle_->serial());
		molog(egbase.get_gametime(), "Opponent: %u\n", battle_->opponent(*this)->serial());
	}
}

/*
==============================

Load/save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 3;

void Soldier::Loader::load(FileRead& fr) {
	Worker::Loader::load(fr);

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {

			Soldier& soldier = get<Soldier>();
			soldier.current_health_ = fr.unsigned_32();
			soldier.retreat_health_ = fr.unsigned_32();

			soldier.health_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_health_level());
			soldier.attack_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_attack_level());
			soldier.defense_level_ =
			   std::min(fr.unsigned_32(), soldier.descr().get_max_defense_level());
			soldier.evade_level_ = std::min(fr.unsigned_32(), soldier.descr().get_max_evade_level());

			// During saveloading init() is not called so we were not registered in the statistics yet
			soldier.get_owner()->add_soldier(soldier.health_level_, soldier.attack_level_,
			                                 soldier.defense_level_, soldier.evade_level_);

			if (soldier.current_health_ > soldier.get_max_health()) {
				soldier.current_health_ = soldier.get_max_health();
			}
			if (soldier.retreat_health_ > soldier.get_max_health()) {
				soldier.retreat_health_ = soldier.get_max_health();
			}

			soldier.combat_walking_ = static_cast<CombatWalkingDir>(fr.unsigned_8());
			if (soldier.combat_walking_ != CD_NONE) {
				soldier.combat_walkstart_ = Time(fr);
				soldier.combat_walkend_ = Time(fr);
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

	if (battle_ != 0u) {
		soldier.battle_ = &mol().get<Battle>(battle_);
	}
}

const Bob::Task* Soldier::Loader::get_task(const std::string& name) {
	if (name == "attack") {
		return &taskAttack;
	}
	if (name == "defense") {
		return &taskDefense;
	}
	if (name == "battle") {
		return &taskBattle;
	}
	if (name == "moveInBattle") {
		return &taskMoveInBattle;
	}
	if (name == "die") {
		return &taskDie;
	}
	if (name == "naval_invasion") {
		return &taskNavalInvasion;
	}
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
		combat_walkstart_.save(fw);
		combat_walkend_.save(fw);
	}

	fw.unsigned_32(mos.get_object_file_index_or_zero(battle_));
}

}  // namespace Widelands
