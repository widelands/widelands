/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#include "wui/interactive_gamebase.h"

#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "wui/constructionsitewindow.h"
#include "wui/dismantlesitewindow.h"
#include "wui/game_summary.h"
#include "wui/militarysitewindow.h"
#include "wui/productionsitewindow.h"
#include "wui/shipwindow.h"
#include "wui/trainingsitewindow.h"
#include "wui/unique_window_handler.h"
#include "wui/warehousewindow.h"

namespace {

std::string speed_string(int const speed) {
	if (speed) {
		return (boost::format("%u.%ux") % (speed / 1000) % (speed / 100 % 10)).str();
	}
	return _("PAUSE");
}

}  // namespace

InteractiveGameBase::InteractiveGameBase(Widelands::Game& g,
                                         Section& global_s,
                                         PlayerType pt,
                                         bool const multiplayer)
   : InteractiveBase(g, global_s),
     chat_provider_(nullptr),
     multiplayer_(multiplayer),
     playertype_(pt) {
	buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   switch (note.action) {
		   case Widelands::NoteBuilding::Action::kFinishWarp: {
			   if (upcast(
			          Widelands::Building const, building, game().objects().get_object(note.serial))) {
				   const Widelands::Coords coords = building->get_position();
				   // Check whether the window is wanted
				   if (wanted_building_windows_.count(coords.hash()) == 1) {
					   UI::UniqueWindow* building_window = show_building_window(coords, true);
					   building_window->set_pos(wanted_building_windows_.at(coords.hash()).first);
					   if (wanted_building_windows_.at(coords.hash()).second) {
						   building_window->minimize();
					   }
					   wanted_building_windows_.erase(coords.hash());
				   }
			   }
		   } break;
		   default:
			   break;
		   }
		});
}

/// \return a pointer to the running \ref Game instance.
Widelands::Game* InteractiveGameBase::get_game() const {
	return dynamic_cast<Widelands::Game*>(&egbase());
}

Widelands::Game& InteractiveGameBase::game() const {
	return dynamic_cast<Widelands::Game&>(egbase());
}

void InteractiveGameBase::set_chat_provider(ChatProvider& chat) {
	chat_provider_ = &chat;
	chat_overlay_->set_chat_provider(chat);
}

ChatProvider* InteractiveGameBase::get_chat_provider() {
	return chat_provider_;
}

void InteractiveGameBase::draw_overlay(RenderTarget& dst) {
	InteractiveBase::draw_overlay(dst);

	GameController* game_controller = game().game_controller();
	// Display the gamespeed.
	if (game_controller != nullptr) {
		std::string game_speed;
		uint32_t const real = game_controller->real_speed();
		uint32_t const desired = game_controller->desired_speed();
		if (real == desired) {
			if (real != 1000) {
				game_speed = as_condensed(speed_string(real));
			}
		} else {
			game_speed = as_condensed((boost::format
			                           /** TRANSLATORS: actual_speed (desired_speed) */
			                           (_("%1$s (%2$s)")) %
			                           speed_string(real) % speed_string(desired))
			                             .str());
		}

		if (!game_speed.empty()) {
			dst.blit(Vector2f(get_w() - 5, 5), UI::g_fh1->render(game_speed), BlendMode::UseAlpha,
			         UI::Align::kRight);
		}
	}
}

/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void InteractiveGameBase::postload() {
	Widelands::Map& map = egbase().map();
	auto* overlay_manager = mutable_field_overlay_manager();
	show_buildhelp(false);
	on_buildhelp_changed(buildhelp());

	overlay_manager->register_overlay_callback_function(
	   boost::bind(&InteractiveGameBase::calculate_buildcaps, this, _1));

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map(egbase().world());

	// Close game-relevant UI windows (but keep main menu open)
	fieldaction_.destroy();
	hide_minimap();
}

void InteractiveGameBase::on_buildhelp_changed(const bool value) {
	toggle_buildhelp_->set_perm_pressed(value);
}

void InteractiveGameBase::add_wanted_building_window(const Widelands::Coords& coords,
                                                     const Vector2i point,
                                                     bool was_minimal) {
	wanted_building_windows_.insert(
	   std::make_pair(coords.hash(), std::make_pair(point, was_minimal)));
}

UI::UniqueWindow* InteractiveGameBase::show_building_window(const Widelands::Coords& coord,
                                                            bool avoid_fastclick) {
	Widelands::BaseImmovable* immovable = game().map().get_immovable(coord);
	upcast(Widelands::Building, building, immovable);
	assert(building);
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("building_%d") % building->serial()).str());

	switch (building->descr().type()) {
	case Widelands::MapObjectType::CONSTRUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new ConstructionSiteWindow(*this, registry,
			                           *dynamic_cast<Widelands::ConstructionSite*>(building),
			                           avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::DISMANTLESITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new DismantleSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::DismantleSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::MILITARYSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new MilitarySiteWindow(
			   *this, registry, *dynamic_cast<Widelands::MilitarySite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::PRODUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new ProductionSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::ProductionSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::TRAININGSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new TrainingSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::TrainingSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::WAREHOUSE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new WarehouseWindow(
			   *this, registry, *dynamic_cast<Widelands::Warehouse*>(building), avoid_fastclick);
		};
		break;
	default:
		log("Unable to show window for building '%s', type '%s'.\n", building->descr().name().c_str(),
		    to_string(building->descr().type()).c_str());
		NEVER_HERE();
	}
	registry.create();
	return registry.window;
}

/**
 * See if we can reasonably open a ship window at the current selection position.
 * If so, do it and return true; otherwise, return false.
 */
bool InteractiveGameBase::try_show_ship_window() {
	Widelands::Map& map(game().map());
	Widelands::Area<Widelands::FCoords> area(map.get_fcoords(get_sel_pos().node), 1);

	if (!(area.field->nodecaps() & Widelands::MOVECAPS_SWIM))
		return false;

	std::vector<Widelands::Bob*> ships;
	if (!map.find_bobs(area, &ships, Widelands::FindBobShip()))
		return false;

	for (Widelands::Bob* temp_ship : ships) {
		if (upcast(Widelands::Ship, ship, temp_ship)) {
			if (can_see(ship->get_owner()->player_number())) {
				show_ship_window(ship);
				return true;
			}
		}
	}
	return false;
}

void InteractiveGameBase::show_ship_window(Widelands::Ship* ship) {
	UI::UniqueWindow::Registry& registry =
		unique_windows().get_registry((boost::format("ship_%d") % ship->serial()).str());
	registry.open_window = [this, &registry, ship] {
		new ShipWindow(*this, registry, *ship);
	};
	registry.create();
}

void InteractiveGameBase::show_game_summary() {
	if (game_summary_.window) {
		game_summary_.window->set_visible(true);
		game_summary_.window->think();
		return;
	}
	new GameSummaryScreen(this, &game_summary_);
}
