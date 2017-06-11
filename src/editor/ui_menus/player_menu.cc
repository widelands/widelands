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

#include "editor/ui_menus/player_menu.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "ui_basic/editbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"
#include "wui/field_overlay_manager.h"

#define UNDEFINED_TRIBE_NAME "<undefined>"

inline EditorInteractive& EditorPlayerMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorPlayerMenu::EditorPlayerMenu(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "players_menu", &registry, 340, 400, _("Player Options")),
     add_player_(this,
                 "add_player",
                 get_inner_w() - 5 - 20,
                 5,
                 20,
                 20,
                 UI::Button::Style::kWuiSecondary,
                 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
                 _("Add player")),
     remove_last_player_(this,
                         "remove_last_player",
                         5,
                         5,
                         20,
                         20,
                         UI::Button::Style::kWuiSecondary,
                         g_gr->images().get("images/ui_basic/scrollbar_down.png"),
                         _("Remove last player")),
     tribenames_(Widelands::get_all_tribenames()) {
	add_player_.set_enabled(parent.egbase().map().get_nrplayers() < kMaxPlayers);
	add_player_.sigclicked.connect(
	   boost::bind(&EditorPlayerMenu::clicked_add_player, boost::ref(*this)));
	remove_last_player_.sigclicked.connect(
	   boost::bind(&EditorPlayerMenu::clicked_remove_last_player, boost::ref(*this)));

	int32_t const spacing = 5;
	int32_t const width = 20;
	int32_t posy = 0;

	set_inner_size(375, 135);

	UI::Textarea* ta = new UI::Textarea(this, 0, 0, _("Number of Players"));
	ta->set_pos(Vector2i((get_inner_w() - ta->get_w()) / 2, posy + 5));
	posy += spacing + width;

	nr_of_players_ta_ = new UI::Textarea(this, 0, 0, "5");
	nr_of_players_ta_->set_pos(Vector2i((get_inner_w() - nr_of_players_ta_->get_w()) / 2, posy + 5));

	posy += width + spacing + spacing;

	posy_ = posy;

	for (Widelands::PlayerNumber i = 0; i < kMaxPlayers; ++i) {
		plr_names_[i] = nullptr;
		plr_set_pos_buts_[i] = nullptr;
		plr_set_tribes_buts_[i] = nullptr;
		plr_make_infrastructure_buts_[i] = nullptr;
	}
	update();

	set_thinks(true);
}

/**
 * Think function. Some things may change while this window
 * is open
 */
void EditorPlayerMenu::think() {
	update();
}

/**
 * Update all
*/
void EditorPlayerMenu::update() {
	if (is_minimal())
		return;

	Widelands::Map& map = eia().egbase().map();
	Widelands::PlayerNumber const nr_players = map.get_nrplayers();
	{
		assert(nr_players <= 99);  //  2 decimal digits
		char text[3];
		if (char const nr_players_10 = nr_players / 10) {
			text[0] = '0' + nr_players_10;
			text[1] = '0' + nr_players % 10;
			text[2] = '\0';
		} else {
			text[0] = '0' + nr_players;
			text[1] = '\0';
		}
		nr_of_players_ta_->set_text(text);
	}

	//  Now remove all the unneeded stuff.
	for (Widelands::PlayerNumber i = nr_players; i < kMaxPlayers; ++i) {
		delete plr_names_[i];
		plr_names_[i] = nullptr;
		delete plr_set_pos_buts_[i];
		plr_set_pos_buts_[i] = nullptr;
		delete plr_set_tribes_buts_[i];
		plr_set_tribes_buts_[i] = nullptr;
	}
	int32_t posy = posy_;
	int32_t const spacing = 5;
	int32_t const size = 20;

	iterate_player_numbers(p, nr_players) {
		int32_t posx = spacing;
		if (!plr_names_[p - 1]) {
			plr_names_[p - 1] = new UI::EditBox(
			   this, posx, posy, 140, size, 2, Panel::Style::kWui);
			plr_names_[p - 1]->changed.connect(
			   boost::bind(&EditorPlayerMenu::name_changed, this, p - 1));
			posx += 140 + spacing;
			plr_names_[p - 1]->set_text(map.get_scenario_player_name(p));
		}

		if (!plr_set_tribes_buts_[p - 1]) {
			plr_set_tribes_buts_[p - 1] =
			   new UI::Button(this, "tribe", posx, posy, 140, size,
			                  UI::Button::Style::kWuiSecondary, "");
			plr_set_tribes_buts_[p - 1]->sigclicked.connect(
			   boost::bind(&EditorPlayerMenu::player_tribe_clicked, boost::ref(*this), p - 1));
			posx += 140 + spacing;
		}

		// Get/Set (localized) tribe names
		if (map.get_scenario_player_tribe(p) != UNDEFINED_TRIBE_NAME) {
			selected_tribes_[p - 1] = map.get_scenario_player_tribe(p);
		} else {
			selected_tribes_[p - 1] = tribenames_[0];
			map.set_scenario_player_tribe(p, selected_tribes_[p - 1]);
		}

		plr_set_tribes_buts_[p - 1]->set_title(
		   Widelands::get_tribeinfo(selected_tribes_[p - 1]).descname);

		// Set default AI and closeable to false (always default - should be changed by hand)
		map.set_scenario_player_ai(p, "");
		map.set_scenario_player_closeable(p, false);

		//  Set Starting pos button.
		if (!plr_set_pos_buts_[p - 1]) {
			plr_set_pos_buts_[p - 1] =
			   new UI::Button(this, "starting_pos", posx, posy, size, size,
			                  UI::Button::Style::kWuiSecondary, nullptr, "");
			plr_set_pos_buts_[p - 1]->sigclicked.connect(
			   boost::bind(&EditorPlayerMenu::set_starting_pos_clicked, boost::ref(*this), p));
		}
		const Image* player_image =
		   playercolor_image(p - 1, "images/players/player_position_menu.png");
		assert(player_image);

		plr_set_pos_buts_[p - 1]->set_pic(player_image);
		posy += size + spacing;
	}
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(1 < nr_players);
	set_inner_size(get_inner_w(), posy + spacing);
}

void EditorPlayerMenu::clicked_add_player() {
	Widelands::Map& map = eia().egbase().map();
	Widelands::PlayerNumber const nr_players = map.get_nrplayers() + 1;
	assert(nr_players <= kMaxPlayers);
	map.set_nrplayers(nr_players);
	{                             //  register new default name for this players
		assert(nr_players <= 99);  //  2 decimal digits
		/** TRANSLATORS: Default player name, e.g. Player 1 */
		const std::string name =
		   (boost::format(_("Player %u")) % static_cast<unsigned int>(nr_players)).str();
		map.set_scenario_player_name(nr_players, name);
	}
	map.set_scenario_player_tribe(nr_players, tribenames_[0]);
	eia().set_need_save(true);
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(true);
	update();
}

void EditorPlayerMenu::clicked_remove_last_player() {
	EditorInteractive& menu = eia();
	Widelands::Map& map = menu.egbase().map();
	Widelands::PlayerNumber const old_nr_players = map.get_nrplayers();
	Widelands::PlayerNumber const nr_players = old_nr_players - 1;
	assert(1 <= nr_players);

	if (!menu.is_player_tribe_referenced(old_nr_players)) {
		if (const Widelands::Coords sp = map.get_starting_pos(old_nr_players)) {
			//  Remove starting position marker.
			const Image* player_image =
			   playercolor_image(old_nr_players - 1, "images/players/player_position.png");
			assert(player_image);
			menu.mutable_field_overlay_manager()->remove_overlay(sp, player_image);
		}
		// if removed player was selected switch to the next highest player
		if (old_nr_players == menu.tools()->set_starting_pos.get_current_player())
			set_starting_pos_clicked(nr_players);
	}
	map.set_nrplayers(nr_players);
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(1 < nr_players);
	update();
	// TODO(SirVer): Take steps when the player is referenced someplace. Not
	// TODO(SirVer): currently possible in the editor though.
}

/**
 * Player Tribe Button clicked
 */
void EditorPlayerMenu::player_tribe_clicked(uint8_t n) {
	EditorInteractive& menu = eia();
	if (!menu.is_player_tribe_referenced(n + 1)) {
		if (!Widelands::tribe_exists(selected_tribes_[n])) {
			throw wexception(
			   "Map defines tribe %s, but it does not exist!", selected_tribes_[n].c_str());
		}
		uint32_t i;
		for (i = 0; i < tribenames_.size(); ++i) {
			if (tribenames_[i] == selected_tribes_[n]) {
				break;
			}
		}
		selected_tribes_[n] = i == tribenames_.size() - 1 ? tribenames_[0] : tribenames_[++i];
		menu.egbase().map().set_scenario_player_tribe(n + 1, selected_tribes_[n]);
		menu.set_need_save(true);
	} else {
		UI::WLMessageBox mmb(&menu, _("Error!"),
		                     _("Cannot remove player. It is referenced someplace. Remove all"
		                       " buildings and animals that depend on this player and try again."),
		                     UI::WLMessageBox::MBoxType::kOk);
		mmb.run<UI::Panel::Returncodes>();
	}
	update();
}

/**
 * Set Current Start Position button selected
 */
void EditorPlayerMenu::set_starting_pos_clicked(uint8_t n) {
	EditorInteractive& menu = eia();
	//  jump to the current node
	Widelands::Map& map = menu.egbase().map();
	if (Widelands::Coords const sp = map.get_starting_pos(n)) {
		menu.scroll_to_field(sp, MapView::Transition::Smooth);
	}

	//  select tool set mplayer
	menu.select_tool(menu.tools()->set_starting_pos, EditorTool::First);
	menu.tools()->set_starting_pos.set_current_player(n);

	//  reselect tool, so everything is in a defined state
	menu.select_tool(menu.tools()->current(), EditorTool::First);

	//  Register callback function to make sure that only valid locations are
	//  selected.
	menu.mutable_field_overlay_manager()->register_overlay_callback_function(
	   boost::bind(&editor_tool_set_starting_pos_callback, _1, boost::ref(map)));
	map.recalc_whole_map(menu.egbase().world());
	update();
}

/**
 * Player name has changed
 */
void EditorPlayerMenu::name_changed(int32_t m) {
	//  Player name has been changed.
	std::string text = plr_names_[m]->text();
	EditorInteractive& menu = eia();
	Widelands::Map& map = menu.egbase().map();
	map.set_scenario_player_name(m + 1, text);
	plr_names_[m]->set_text(map.get_scenario_player_name(m + 1));
	menu.set_need_save(true);
}

/*
 * Make infrastructure button clicked
 */
void EditorPlayerMenu::make_infrastructure_clicked(uint8_t n) {
	EditorInteractive& parent = dynamic_cast<EditorInteractive&>(*get_parent());
	// Check if starting position is valid (was checked before
	// so must be true)
	Widelands::EditorGameBase& egbase = parent.egbase();
	Widelands::Map& map = egbase.map();
	auto* overlay_manager = parent.mutable_field_overlay_manager();
	const Widelands::Coords start_pos = map.get_starting_pos(n);
	assert(start_pos);

	Widelands::Player* p = egbase.get_player(n);
	if (!p) {
		// This player is unknown, register it,
		// place a hq and reference the tribe
		// so that this tribe can not be changed
		egbase.add_player(n, 0,  // TODO(SirVer): initialization index makes no sense here
		                  Widelands::get_tribeinfo(selected_tribes_[n]).descname,
		                  plr_names_[n - 1]->text());

		p = egbase.get_player(n);
	}

	// If the player is already created in the editor, this means
	// that there might be already a hq placed somewhere. This needs to be
	// deleted before a starting position change can occure
	const Widelands::PlayerNumber player_number = p->player_number();
	const Widelands::Coords starting_pos = map.get_starting_pos(player_number);
	Widelands::BaseImmovable* const imm = map[starting_pos].get_immovable();
	if (!imm) {
		// place HQ
		const Widelands::TribeDescr& tribe = p->tribe();
		const Widelands::DescriptionIndex idx = tribe.headquarters();
		if (!tribe.has_building(idx))
			throw wexception("Tribe %s lacks headquarters", tribe.name().c_str());
		// Widelands::Warehouse & headquarter = dynamic_cast<Widelands::Warehouse &>
		//         (egbase.warp_building(starting_pos, player_number, idx));
		// egbase.conquer_area
		//         (PlayerArea
		//          (player_number, Area(starting_pos, headquarter.get_conquers())));
		// tribe.load_warehouse_with_start_wares(editor, headquarter);

		parent.reference_player_tribe(n, &tribe);

		// Remove the player overlay from this starting pos.
		// A HQ is overlay enough
		const Image* player_image = playercolor_image(n - 1, "images/players/player_position.png");
		assert(player_image);
		overlay_manager->remove_overlay(start_pos, player_image);
	}

	parent.select_tool(parent.tools()->make_infrastructure, EditorTool::First);
	parent.tools()->make_infrastructure.set_player(n);
	overlay_manager->register_overlay_callback_function(
	   boost::bind(&editor_make_infrastructure_tool_callback, _1, boost::ref(egbase), n));
	map.recalc_whole_map(egbase.world());
}
