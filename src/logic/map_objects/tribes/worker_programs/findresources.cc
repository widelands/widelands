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

#include "logic/map_objects/tribes/worker.h"

#include <memory>

#include "graphic/text_layout.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Check resources at the current position, and plant a marker object when
 * possible.
 */
bool Worker::run_findresources(Game& game, State& state, const Action& /* action */) {
	const FCoords position = game.map().get_fcoords(get_position());
	BaseImmovable const* const imm = position.field->get_immovable();
	const Descriptions& descriptions = game.descriptions();

	if ((imm == nullptr) || imm->get_size() <= BaseImmovable::NONE) {

		const ResourceDescription* rdescr =
		   descriptions.get_resource_descr(position.field->get_resources());
		const TribeDescr& t = owner().tribe();
		if ((rdescr != nullptr) && !t.uses_resource(rdescr->name())) {
			rdescr = nullptr;
		}
		const Immovable& ri = game.create_immovable(
		   position,
		   t.get_resource_indicator(rdescr, ((rdescr != nullptr) && rdescr->detectable()) ?
		                                       position.field->get_resources_amount() :
		                                       0),
		   get_owner());

		// Geologist also sends a message notifying the player
		// TODO(GunChleoc): We keep formatting this even when timeout has not elapsed
		if ((rdescr != nullptr) && rdescr->detectable() &&
		    (position.field->get_resources_amount() != 0u)) {
			const std::string rt_description = as_mapobject_message(
			   ri.descr().name(), g_image_cache->get(rdescr->representative_image())->width(),
			   _("A geologist found resources."));

			//  We should add a message to the player's message queue - but only,
			//  if there is not already a similar one in list.
			get_owner()->add_message_with_timeout(
			   game,
			   std::unique_ptr<Message>(new Message(Message::Type::kGeologists, game.get_gametime(),
			                                        rdescr->descname(), rdescr->representative_image(),
			                                        ri.descr().descname(), rt_description, position,
			                                        serial_, rdescr->name())),
			   Duration(rdescr->timeout_ms()), rdescr->timeout_radius());
		}
	}

	++state.ivar1;
	return false;
}

}  // namespace Widelands
