/*
 * Copyright (C) 2007-2016 by the Widelands Development Team
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
#include "wui/game_summary.h"
#include "wui/shipwindow.h"

namespace {

std::string speed_string(int const speed) {
	if (speed) {
		return (boost::format("%u.%ux") % (speed / 1000) % (speed / 100 % 10)).str();
	}
	return _("PAUSE");
}

}  // namespace

InteractiveGameBase::MapObjectWindowManager::MapObjectWindowManager(InteractiveGameBase& igb)
   : igb_(igb) {
	// TODO(GunChleoc): Check if we can have a general NoteMapObjectWindow when we have all of them.
	shipnotes_subscriber_ = Notifications::subscribe<Widelands::NoteShipWindow>(
	   [this](const Widelands::NoteShipWindow& note) {
		   switch (note.action) {
		   // The ship state has changed or the user requested a new window
		   case Widelands::NoteShipWindow::Action::kRefresh:
			   refresh(note.serial);
			   break;
		   // The ship has been removed
		   case Widelands::NoteShipWindow::Action::kRemove:
			   close(note.serial);
			   break;
		   // The user closed the window
		   case Widelands::NoteShipWindow::Action::kClosed:
			   unregister(note.serial);
			   break;
		   }
		});
}

void InteractiveGameBase::MapObjectWindowManager::refresh(const Widelands::Serial& serial) {
	bool is_refreshing = false;
	Point pos(0, 0);
	if (map_object_windows_.count(serial) == 1) {
		UI::Window* window = map_object_windows_.at(serial);
		if (window) {
			is_refreshing = true;
			pos = window->get_pos();
			delete window;
			window = nullptr;
		}
	}
	Widelands::MapObject* mo = igb_.game().objects().get_object(serial);
	// TODO(GunChleoc): Make this a switch statement when we have more of these.
	if (mo->descr().type() == Widelands::MapObjectType::SHIP) {
		upcast(Widelands::Ship, ship, mo);
		ShipWindow* shipwindow = new ShipWindow(igb_, *ship, is_refreshing);
		map_object_windows_.insert(std::pair<Widelands::Serial, UI::Window*>(serial, shipwindow));
		if (is_refreshing) {
			shipwindow->set_pos(pos);
		}
	} else {
		NEVER_HERE();
	}
}

void InteractiveGameBase::MapObjectWindowManager::close(Widelands::Serial serial) {
	if (map_object_windows_.count(serial) == 1) {
		UI::Window* window = map_object_windows_.at(serial);
		if (window) {
			delete window;
			window = nullptr;
		}
		unregister(serial);
	}
}

void InteractiveGameBase::MapObjectWindowManager::unregister(Widelands::Serial serial) {
	if (map_object_windows_.count(serial) == 1) {
		map_object_windows_.erase(serial);
	}
}

InteractiveGameBase::InteractiveGameBase(Widelands::Game& g,
                                         Section& global_s,
                                         PlayerType pt,
                                         bool const chatenabled,
                                         bool const multiplayer)
   : InteractiveBase(g, global_s),
     chat_provider_(nullptr),
     chatenabled_(chatenabled),
     multiplayer_(multiplayer),
     playertype_(pt),

#define INIT_BTN(picture, name, tooltip)                                                           \
	TOOLBAR_BUTTON_COMMON_PARAMETERS(name), g_gr->images().get("images/" picture ".png"), tooltip

     toggle_buildhelp_(INIT_BTN(
        "wui/menus/menu_toggle_buildhelp", "buildhelp", _("Show Building Spaces (on/off)"))),
     window_manager_(new MapObjectWindowManager(*this)) {
	toggle_buildhelp_.sigclicked.connect(boost::bind(&InteractiveGameBase::toggle_buildhelp, this));
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

	chatenabled_ = true;
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
			dst.blit(Point(get_w() - 5, 5), UI::g_fh1->render(game_speed), BlendMode::UseAlpha,
			         UI::Align::kTopRight);
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
	toggle_buildhelp_.set_perm_pressed(buildhelp());

	overlay_manager->register_overlay_callback_function(
	   boost::bind(&InteractiveGameBase::calculate_buildcaps, this, _1));

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map(egbase().world());

	// Close game-relevant UI windows (but keep main menu open)
	delete fieldaction_.window;
	fieldaction_.window = nullptr;

	hide_minimap();
}

void InteractiveGameBase::on_buildhelp_changed(const bool value) {
	toggle_buildhelp_.set_perm_pressed(value);
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
				Notifications::publish(Widelands::NoteShipWindow(
				   ship->serial(), Widelands::NoteShipWindow::Action::kRefresh));
				return true;
			}
		}
	}

	return false;
}

void InteractiveGameBase::show_game_summary() {
	game().game_controller()->set_desired_speed(0);
	if (game_summary_.window) {
		game_summary_.window->set_visible(true);
		game_summary_.window->think();
		return;
	}
	new GameSummaryScreen(this, &game_summary_);
}
