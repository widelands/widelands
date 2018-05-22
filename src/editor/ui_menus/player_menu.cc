/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "ui_basic/messagebox.h"

namespace {
constexpr int kMargin = 4;
constexpr int kButtonHeight = 24;
} // namespace

inline EditorInteractive& EditorPlayerMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorPlayerMenu::EditorPlayerMenu(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "players_menu", &registry, 340, 400, _("Player Options")),
	 box_(this, kMargin, kMargin + 20, UI::Box::Vertical), // NOCOM buttons
     add_player_(this,
                 "add_player",
                 get_inner_w() - 5 - 20,
                 5,
                 20,
                 20,
                 UI::ButtonStyle::kWuiSecondary,
                 g_gr->images().get("images/ui_basic/scrollbar_up.png"),
                 _("Add player")),
     remove_last_player_(this,
                         "remove_last_player",
                         5,
                         5,
                         20,
                         20,
                         UI::ButtonStyle::kWuiSecondary,
                         g_gr->images().get("images/ui_basic/scrollbar_down.png"),
                         _("Remove last player")),
     default_tribe_(Widelands::get_all_tribenames().front()) {

	Widelands::Map* map = eia().egbase().mutable_map();
	Widelands::PlayerNumber const nr_players = map->get_nrplayers();

	iterate_player_numbers(p, kMaxPlayers) {
		const bool map_has_player = p <= nr_players;

		UI::Box* row = new UI::Box(&box_, 0, 0, UI::Box::Horizontal);
		// NOCOM refactor height

		// Name
		UI::EditBox* plr_name = new UI::EditBox(row, 0, 0, 0, kButtonHeight, kMargin, UI::PanelStyle::kWui);
		if (map_has_player) {
			plr_name->set_text(map->get_scenario_player_name(p));
		}
		plr_name->changed.connect(boost::bind(&EditorPlayerMenu::name_changed, this, p - 1));
		row->add(plr_name, UI::Box::Resizing::kFillSpace);
		row->add_space(kMargin);

		// Tribe
		UI::Dropdown<std::string>* plr_tribe = new UI::Dropdown<std::string>(row,
																			 0,
																			 0,
																			 50,
																			 200,
																			 kButtonHeight,
																			 _("Tribe"),
																			 UI::DropdownType::kPictorial,
																			 UI::PanelStyle::kFsMenu);
		{
			i18n::Textdomain td("tribes");
			for (const Widelands::TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
				plr_tribe->add(_(tribeinfo.descname), tribeinfo.name,
									 g_gr->images().get(tribeinfo.icon), false, tribeinfo.tooltip);
			}
		}
		const std::string player_scenario_tribe = map_has_player ? map->get_scenario_player_tribe(p) : default_tribe_;
		plr_tribe->select(Widelands::tribe_exists(player_scenario_tribe) ? player_scenario_tribe : default_tribe_);
		plr_tribe->selected.connect(
		   boost::bind(&EditorPlayerMenu::player_tribe_clicked, boost::ref(*this), p - 1));
		row->add(plr_tribe);
		row->add_space(kMargin);


		// Starting position
		const Image* player_image =
		   playercolor_image(p - 1, "images/players/player_position_menu.png");
		assert(player_image);

		UI::Button* plr_position = new UI::Button(row, "tribe", 0, 0, kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiSecondary, player_image);
		plr_position->sigclicked.connect(
		   boost::bind(&EditorPlayerMenu::set_starting_pos_clicked, boost::ref(*this), p));
		row->add(plr_position);
		rows_.push_back(row);

		box_.add(row, UI::Box::Resizing::kFullSize);
		box_.add_space(kMargin);
		row->set_visible(map_has_player);

		player_edit_.push_back(std::unique_ptr<PlayerEdit>(new PlayerEdit(plr_name, plr_position, plr_tribe)));
	}

	// NOCOM hack
	box_.set_size(get_inner_w() - 2 * kMargin, nr_players * (kButtonHeight + kMargin));

	// Set default AI and closeable to false (always default - should be changed by hand)
	// NOCOM handle this
	// map->set_scenario_player_ai(p, "");
	// map->set_scenario_player_closeable(p, false);

	add_player_.set_enabled(parent.egbase().map().get_nrplayers() < kMaxPlayers);
	add_player_.sigclicked.connect(
	   boost::bind(&EditorPlayerMenu::clicked_add_player, boost::ref(*this)));
	remove_last_player_.sigclicked.connect(
	   boost::bind(&EditorPlayerMenu::clicked_remove_last_player, boost::ref(*this)));

	int32_t const spacing = 5;
	int32_t const width = 20;
	int32_t posy = box_.get_h();

	// NOCOM set_inner_size(375, 135);

	UI::Textarea* ta = new UI::Textarea(this, 0, 0, _("Number of Players"));
	ta->set_pos(Vector2i((get_inner_w() - ta->get_w()) / 2, posy + 5));
	posy += spacing + width;

	nr_of_players_ta_ = new UI::Textarea(this, 0, 0, "5");
	nr_of_players_ta_->set_pos(Vector2i((get_inner_w() - nr_of_players_ta_->get_w()) / 2, posy + 5));

	posy += width + spacing + spacing;

	posy_ = posy;
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

	Widelands::Map* map = eia().egbase().mutable_map();
	Widelands::PlayerNumber const nr_players = map->get_nrplayers();
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

	int32_t posy = posy_;
	int32_t const spacing = 5;
	int32_t const size = 20;

	iterate_player_numbers(p, nr_players) {
		// Set default AI and closeable to false (always default - should be changed by hand)
		map->set_scenario_player_ai(p, "");
		map->set_scenario_player_closeable(p, false);
		posy += size + spacing;
	}
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(1 < nr_players);
	set_inner_size(get_inner_w(), posy + spacing);
}

void EditorPlayerMenu::clicked_add_player() {
	Widelands::Map* map = eia().egbase().mutable_map();
	Widelands::PlayerNumber const nr_players = map->get_nrplayers() + 1;
	assert(nr_players <= kMaxPlayers);
	map->set_nrplayers(nr_players);
	{                             //  register new default name for this players
		assert(nr_players <= 99);  //  2 decimal digits
		const std::string name =
		   /** TRANSLATORS: Default player name, e.g. Player 1 */
		   (boost::format(_("Player %u")) % static_cast<unsigned int>(nr_players)).str();
		map->set_scenario_player_name(nr_players, name);
	}
	const std::string& tribename = player_edit_.at(nr_players - 1)->tribe->get_selected();
	assert(Widelands::tribe_exists(tribename));
	map->set_scenario_player_tribe(nr_players, tribename);
	eia().set_need_save(true);
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(true);
	rows_.at(nr_players - 1)->set_visible(true);
	player_edit_.at(nr_players - 1)->name->set_visible(true);
	player_edit_.at(nr_players - 1)->position->set_visible(true);
	player_edit_.at(nr_players - 1)->tribe->set_visible(true);
	update();
}

void EditorPlayerMenu::clicked_remove_last_player() {
	EditorInteractive& menu = eia();
	Widelands::Map* map = menu.egbase().mutable_map();
	Widelands::PlayerNumber const old_nr_players = map->get_nrplayers();
	Widelands::PlayerNumber const nr_players = old_nr_players - 1;
	assert(1 <= nr_players);

	// if removed player was selected switch to the next highest player
	if (old_nr_players == menu.tools()->set_starting_pos.get_current_player()) {
		set_starting_pos_clicked(nr_players);
	}

	map->set_nrplayers(nr_players);
	add_player_.set_enabled(nr_players < kMaxPlayers);
	remove_last_player_.set_enabled(1 < nr_players);
	rows_.at(old_nr_players - 1)->set_visible(false);
	update();
}

/**
 * Player Tribe Button clicked
 */
void EditorPlayerMenu::player_tribe_clicked(uint8_t n) {
	const std::string& tribename = player_edit_.at(n)->tribe->get_selected();
	assert(Widelands::tribe_exists(tribename));
	EditorInteractive& menu = eia();
	menu.egbase().mutable_map()->set_scenario_player_tribe(n + 1, tribename);
	menu.set_need_save(true);
	update();
}

/**
 * Set Current Start Position button selected
 */
void EditorPlayerMenu::set_starting_pos_clicked(uint8_t n) {
	EditorInteractive& menu = eia();
	//  jump to the current node
	Widelands::Map* map = menu.egbase().mutable_map();
	if (Widelands::Coords const sp = map->get_starting_pos(n)) {
		menu.map_view()->scroll_to_field(sp, MapView::Transition::Smooth);
	}

	//  select tool set mplayer
	menu.select_tool(menu.tools()->set_starting_pos, EditorTool::First);
	menu.tools()->set_starting_pos.set_current_player(n);

	//  reselect tool, so everything is in a defined state
	menu.select_tool(menu.tools()->current(), EditorTool::First);
	update();
}

/**
 * Player name has changed
 */
void EditorPlayerMenu::name_changed(int32_t n) {
	//  Player name has been changed.
	const std::string& text = player_edit_.at(n)->name->text();
	EditorInteractive& menu = eia();
	Widelands::Map* map = menu.egbase().mutable_map();
	map->set_scenario_player_name(n + 1, text);
	menu.set_need_save(true);
}
