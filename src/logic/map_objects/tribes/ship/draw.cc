/*
 * Copyright (C) 2010-2026 by the Widelands Development Team
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

#include "logic/map_objects/tribes/ship/ship.h"

#include "economy/portdock.h"
#include "economy/ship_fleet.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/player.h"

namespace Widelands {

void Ship::draw(const EditorGameBase& egbase,
                const InfoToDraw& info_to_draw,
                const Vector2f& field_on_dst,
                const Widelands::Coords& coords,
                const float scale,
                RenderTarget* dst) const {
	Bob::draw(egbase, info_to_draw, field_on_dst, coords, scale, dst);

	// Show ship name and current activity
	std::string statistics_string;
	if ((info_to_draw & InfoToDraw::kStatistics) != 0) {
		if (state_is_sinking()) {
			statistics_string = pgettext("ship_state", "Sinking");
		} else if (has_battle()) {
			statistics_string = pgettext("ship_state", "Fighting");
		} else if (is_refitting()) {
			switch (pending_refit_) {
			case ShipType::kTransport:
				statistics_string = pgettext("ship_state", "Refitting to Transport Ship");
				break;
			case ShipType::kWarship:
				statistics_string = pgettext("ship_state", "Refitting to Warship");
				break;
			default:
				NEVER_HERE();
			}
		} else {
			if (ship_type_ == ShipType::kWarship) {
				// TODO(Nordfriese): maybe show more state here
				statistics_string = pgettext("ship_state", "Warship");
			} else {
				switch (ship_state_) {
				case (ShipStates::kTransport): {
					const MapObject* dest = destination_object_.get(egbase);
					if (destination_coords_ != nullptr) {
						format(pgettext("ship_state", "Sailing to %s"),
						       destination_coords_->to_long_string(egbase));
					} else if (dest == nullptr) {
						/** TRANSLATORS: This is a ship state. The ship is ready
						 * to transport wares, but has nothing to do. */
						statistics_string = pgettext("ship_state", "Empty");
					} else if (dest->descr().type() == MapObjectType::SHIP) {
						statistics_string =
						   /** TRANSLATORS: This is a ship state. The ship is
						    * currently sailing to a specific destination ship. */
						   format(pgettext("ship_state", "Sailing to %s"),
						          dynamic_cast<const Ship*>(dest)->get_shipname());
					} else if (dest->descr().type() == MapObjectType::PINNED_NOTE) {
						statistics_string =
						   /** TRANSLATORS: This is a ship state. The ship is
						    * currently sailing to a specific destination note. */
						   format(pgettext("ship_state", "Sailing to %s"),
						          dynamic_cast<const PinnedNote*>(dest)->get_text());
					} else {
						const std::string& wh_name =
						   dynamic_cast<const PortDock*>(dest)->get_warehouse()->get_warehouse_name();
						if (fleet_->get_schedule().is_busy(*this)) {
							statistics_string =
							   /** TRANSLATORS: This is a ship state. The ship is currently
							    * transporting wares to a specific destination port. */
							   format(pgettext("ship_state", "Shipping to %s"), wh_name);
						} else {
							statistics_string =
							   /** TRANSLATORS: This is a ship state. The ship is currently sailing
							    * to a specific destination port without transporting wares. */
							   format(pgettext("ship_state", "Sailing to %s"), wh_name);
						}
					}
					break;
				}
				case (ShipStates::kExpeditionWaiting):
					/** TRANSLATORS: This is a ship state. An expedition is waiting for your commands. */
					statistics_string = pgettext("ship_state", "Waiting");
					break;
				case (ShipStates::kExpeditionScouting):
					/** TRANSLATORS: This is a ship state. An expedition is scouting for port spaces. */
					statistics_string = pgettext("ship_state", "Scouting");
					break;
				case (ShipStates::kExpeditionPortspaceFound):
					/** TRANSLATORS: This is a ship state. An expedition has found a port space. */
					statistics_string = pgettext("ship_state", "Port Space Found");
					break;
				case (ShipStates::kExpeditionColonizing):
					/** TRANSLATORS: This is a ship state. An expedition is unloading wares/workers to
					 * build a port. */
					statistics_string = pgettext("ship_state", "Founding a Colony");
					break;
				case (ShipStates::kSinkRequest):
				case (ShipStates::kSinkAnimation):
					break;
				default:
					NEVER_HERE();
				}
			}
		}

		statistics_string = StyleManager::color_tag(
		   statistics_string, g_style_manager->building_statistics_style().medium_color());
	}

	const Vector2f point_on_dst = calc_drawpos(egbase, field_on_dst, scale);
	do_draw_info(
	   info_to_draw, richtext_escape(shipname_), statistics_string, point_on_dst, scale, dst);

	if ((info_to_draw & InfoToDraw::kSoldierLevels) != 0 &&
	    (ship_type_ == ShipType::kWarship || hitpoints_ < descr().max_hitpoints_)) {
		draw_healthbar(egbase, dst, point_on_dst, scale);
	}
}

void Ship::draw_healthbar(const EditorGameBase& egbase,
                          RenderTarget* dst,
                          const Vector2f& point_on_dst,
                          float scale) const {
	// TODO(Nordfriese): Common code with Soldier::draw_info_icon
	const RGBColor& color = owner().get_playercolor();
	const uint16_t color_sum = color.r + color.g + color.b;
	const int brighten_factor = 230 - color_sum / 3;

	const Vector2i draw_position = point_on_dst.cast<int>();

	// The frame gets a slight tint of player color
	Recti energy_outer(draw_position - Vector2i(kShipHalfHealthBarWidth, 0) * scale,
	                   kShipHalfHealthBarWidth * 2 * scale, 5 * scale);
	dst->fill_rect(energy_outer, color);
	dst->brighten_rect(energy_outer, brighten_factor);

	// Adjust health to current animation tick
	uint32_t health_to_show = hitpoints_;
	if (has_battle() &&
	    battles_.back().phase == (battles_.back().is_first ? Battle::Phase::kDefenderAttacking :
	                                                         Battle::Phase::kAttackerAttacking)) {
		uint32_t pending_damage =
		   battles_.back().pending_damage *
		   (owner().egbase().get_gametime() - battles_.back().time_of_last_action).get() /
		   kAttackAnimationDuration;
		if (pending_damage > health_to_show) {
			health_to_show = 0;
		} else {
			health_to_show -= pending_damage;
		}
	}

	// Now draw the health bar itself
	constexpr int kInnerHealthBarWidth = 2 * (kShipHalfHealthBarWidth - 1);
	int health_width = kInnerHealthBarWidth * health_to_show / descr().max_hitpoints_;

	Recti energy_inner(draw_position + Vector2i(-kShipHalfHealthBarWidth + 1, 1) * scale,
	                   health_width * scale, 3 * scale);
	Recti energy_complement(energy_inner.origin() + Vector2i(health_width, 0) * scale,
	                        (kInnerHealthBarWidth - health_width) * scale, 3 * scale);

	const RGBColor complement_color =
	   color_sum > 128 * 3 ? RGBColor(32, 32, 32) : RGBColor(224, 224, 224);
	dst->fill_rect(energy_inner, color);
	dst->fill_rect(energy_complement, complement_color);

	// Now soldier strength bonus bars
	if (ship_type_ != ShipType::kWarship) {
		return;
	}
	const unsigned bonus = get_sea_attack_soldier_bonus(egbase);
	if (bonus > 0) {
		assert(bonus < 2000);  // Sanity check
		constexpr unsigned kBonusPerBar = kInnerHealthBarWidth;

		energy_outer.y += energy_outer.h + 2 * scale;
		energy_inner.y = energy_outer.y + scale;
		energy_outer.h = (ceilf(static_cast<float>(bonus) / kBonusPerBar) * 3 + 2) * scale;
		dst->fill_rect(energy_outer, color);
		dst->brighten_rect(energy_outer, brighten_factor);

		energy_inner.w = kInnerHealthBarWidth * scale;
		energy_inner.h = static_cast<int>(bonus / kBonusPerBar) * 3 * scale;
		dst->fill_rect(energy_inner, color);

		if (const unsigned remainder = bonus % kBonusPerBar; remainder != 0) {
			assert(remainder < kBonusPerBar);
			energy_inner.y += energy_inner.h;
			energy_complement.y = energy_inner.y;

			health_width = kInnerHealthBarWidth * remainder * scale / kBonusPerBar;
			energy_complement.x = energy_inner.x + health_width;
			energy_complement.w = energy_inner.w - health_width;
			energy_inner.w = health_width;
			energy_inner.h = energy_complement.h;

			dst->fill_rect(energy_inner, color);
			dst->fill_rect(energy_complement, complement_color);
		}
	}
}

}  // namespace Widelands
