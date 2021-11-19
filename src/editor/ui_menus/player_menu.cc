/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <memory>

#include "base/i18n.h"
#include "base/string.h"
#include "editor/editorinteractive.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "graphic/playercolor.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/widelands.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "wlapplication_options.h"

namespace {
constexpr int kMargin = 4;
// Make room for 8 players
// If this ever gets changed, don't forget to change the strings in the warning box as well.
constexpr Widelands::PlayerNumber kMaxRecommendedPlayers = 8;
}  // namespace

class EditorPlayerMenuWarningBox : public UI::Window {
public:
	explicit EditorPlayerMenuWarningBox(UI::Panel* parent)
	   : Window(parent,
	            UI::WindowStyle::kWui,
	            "editor_player_menu_warning_box",
	            0,
	            0,
	            500,
	            220,
	            /** TRANSLATORS: Window title in the editor when a player has selected more than the
	               recommended number of players */
	            _("Too Many Players")),
	     box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, 2 * kMargin),
	     warning_label_(
	        &box_,
	        0,
	        0,
	        300,
	        0,
	        UI::PanelStyle::kWui,
	        /** TRANSLATORS: Info text in editor player menu when a player has selected more than the
	           recommended number of players. Choice is made by OK/Abort. */
	        _("We do not recommend setting more than 8 players except for testing "
	          "purposes. Are you sure that you want more than 8 players?"),
	        UI::Align::kLeft,
	        UI::MultilineTextarea::ScrollMode::kNoScrolling),
	     reminder_choice_(
	        &box_,
	        UI::PanelStyle::kWui,
	        Vector2i::zero(),
	        /** TRANSLATORS: Checkbox for: 'We do not recommend setting more than 8 players except
	           for testing purposes. Are you sure that you want more than 8 players?' */
	        _("Do not remind me again")),
	     button_box_(
	        &box_, UI::PanelStyle::kWui, kMargin, kMargin, UI::Box::Horizontal, 0, 0, 2 * kMargin),
	     ok_(&button_box_, "ok", 0, 0, 120, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),
	     cancel_(&button_box_, "cancel", 0, 0, 120, 0, UI::ButtonStyle::kWuiSecondary, _("Abort")) {

		set_center_panel(&box_);

		box_.add(&warning_label_, UI::Box::Resizing::kFullSize);
		box_.add(&reminder_choice_, UI::Box::Resizing::kFullSize);

		button_box_.add_inf_space();
		button_box_.add(&cancel_);
		button_box_.add_inf_space();
		button_box_.add(&ok_);
		button_box_.add_inf_space();
		box_.add_space(kMargin);
		box_.add(&button_box_, UI::Box::Resizing::kFullSize);
		box_.add_space(kMargin);

		ok_.sigclicked.connect([this]() { ok(); });
		cancel_.sigclicked.connect([this]() { cancel(); });

		initialization_complete();
	}

	void ok() {
		write_option();
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	}

	void cancel() {
		write_option();
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}

	void write_option() {
		if (reminder_choice_.get_state()) {
			set_config_bool("editor_player_menu_warn_too_many_players", false);
		}
	}

private:
	UI::Box box_;
	UI::MultilineTextarea warning_label_;
	UI::Checkbox reminder_choice_;
	UI::Box button_box_;
	UI::Button ok_;
	UI::Button cancel_;
};

inline EditorInteractive& EditorPlayerMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorPlayerMenu::EditorPlayerMenu(EditorInteractive& parent,
                                   EditorSetStartingPosTool& tool,
                                   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Player Options"), tool),
     box_(this, UI::PanelStyle::kWui, kMargin, kMargin, UI::Box::Vertical),
     no_of_players_(&box_,
                    "dropdown_map_players",
                    0,
                    0,
                    50,
                    kMaxRecommendedPlayers,
                    24,
                    _("Number of players"),
                    UI::DropdownType::kTextual,
                    UI::PanelStyle::kWui,
                    UI::ButtonStyle::kWuiSecondary) {
	box_.set_size(100, 100);  // Prevent assert failures
	box_.add(&no_of_players_, UI::Box::Resizing::kFullSize);
	box_.add_space(2 * kMargin);

	const Widelands::Map& map = eia().egbase().map();

	// Ensure that there is at least 1 player
	if (map.get_nrplayers() < 1) {
		Widelands::Map* mutable_map = eia().egbase().mutable_map();
		mutable_map->set_nrplayers(1);
		// Init player 1
		mutable_map->set_scenario_player_ai(1, "");
		mutable_map->set_scenario_player_closeable(1, false);
		/** TRANSLATORS: Default player name, e.g. Player 1 */
		mutable_map->set_scenario_player_name(1, bformat(_("Player %u"), 1));
		mutable_map->set_scenario_player_tribe(1, "");
		eia().set_need_save(true);
	}

	const Widelands::PlayerNumber nr_players = map.get_nrplayers();
	iterate_player_numbers(p, kMaxPlayers) {
		const bool map_has_player = p <= nr_players;

		no_of_players_.add(as_string(p), p, nullptr, p == nr_players);
		no_of_players_.selected.connect([this]() { no_of_players_clicked(); });

		UI::Box* row = new UI::Box(&box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

		// Name
		UI::EditBox* plr_name = new UI::EditBox(row, 0, 0, 0, UI::PanelStyle::kWui);
		if (map_has_player) {
			plr_name->set_text(map.get_scenario_player_name(p));
		}
		plr_name->changed.connect([this, p]() { name_changed(p - 1); });

		// Tribe
		UI::Dropdown<std::string>* plr_tribe = new UI::Dropdown<std::string>(
		   row, bformat("dropdown_tribe%d", static_cast<unsigned int>(p)), 0, 0, 50, 16,
		   plr_name->get_h(), _("Tribe"), UI::DropdownType::kPictorial, UI::PanelStyle::kWui,
		   UI::ButtonStyle::kWuiSecondary);

		for (const Widelands::TribeBasicInfo& tribeinfo : parent.egbase().all_tribes()) {
			plr_tribe->add(tribeinfo.descname, tribeinfo.name, g_image_cache->get(tribeinfo.icon),
			               false, tribeinfo.tooltip);
		}

		plr_tribe->add(pgettext("tribe", "Random"), "",
		               g_image_cache->get("images/ui_fsmenu/random.png"), false,
		               _("The tribe will be selected at random"));

		plr_tribe->select(
		   (p <= map.get_nrplayers() &&
		    Widelands::tribe_exists(map.get_scenario_player_tribe(p), parent.egbase().all_tribes())) ?
            map.get_scenario_player_tribe(p) :
            "");
		plr_tribe->selected.connect([this, p]() { player_tribe_clicked(p - 1); });

		// Starting position
		const Image* player_image =
		   playercolor_image(p - 1, "images/players/player_position_menu.png");
		assert(player_image);

		UI::Button* plr_position = new UI::Button(
		   row, "tribe", 0, 0, plr_tribe->get_h(), plr_tribe->get_h(), UI::ButtonStyle::kWuiSecondary,
		   /** TRANSLATORS: Button tooltip in the editor for using a player's starting position tool
		    */
		   player_image, _("Set this player’s starting position"));
		plr_position->sigclicked.connect([this, p]() { set_starting_pos_clicked(p); });

		// Add the elements to the row
		row->add(plr_name, UI::Box::Resizing::kFillSpace);
		row->add_space(kMargin);

		row->add(plr_tribe);
		row->add_space(kMargin);

		row->add(plr_position);

		// Add the row itself
		box_.add(row, UI::Box::Resizing::kFullSize);
		box_.add_space(kMargin);
		row->set_visible(map_has_player);

		rows_.push_back(
		   std::unique_ptr<PlayerEditRow>(new PlayerEditRow(row, plr_name, plr_position, plr_tribe)));
	}

	no_of_players_.select(nr_players);

	// Init button states
	set_starting_pos_clicked(1);
	layout();

	initialization_complete();
}

void EditorPlayerMenu::layout() {
	if (is_minimal()) {
		return;
	}
	EditorToolOptionsMenu::layout();
	assert(!rows_.empty());
	const Widelands::PlayerNumber nr_players = eia().egbase().map().get_nrplayers();
	box_.set_size(310, no_of_players_.get_h() + kMargin +
	                      nr_players * (rows_.front()->name->get_h() + kMargin));
	set_inner_size(box_.get_w() + 2 * kMargin, box_.get_h() + 2 * kMargin);
}

void EditorPlayerMenu::no_of_players_clicked() {
	EditorInteractive& menu = eia();
	Widelands::Map* map = menu.egbase().mutable_map();
	Widelands::PlayerNumber const old_nr_players = map->get_nrplayers();
	Widelands::PlayerNumber const nr_players = no_of_players_.get_selected();
	assert(1 <= nr_players);
	assert(nr_players <= kMaxPlayers);

	if (old_nr_players == nr_players) {
		return;
	}

	// Display a warning if there are too many players
	if (nr_players > kMaxRecommendedPlayers) {
		if (get_config_bool("editor_player_menu_warn_too_many_players", true)) {
			EditorPlayerMenuWarningBox warning(get_parent());
			if (warning.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
				// Abort setting of players
				no_of_players_.select(std::min(old_nr_players, kMaxRecommendedPlayers));
			}
		}
	}

	if (old_nr_players < nr_players) {
		// Add new players
		map->set_nrplayers(nr_players);

		for (Widelands::PlayerNumber pn = old_nr_players + 1; pn <= nr_players; ++pn) {
			map->set_scenario_player_ai(pn, "");
			map->set_scenario_player_closeable(pn, false);

			// Register new default name and tribe for these players
			const std::string name =
			   /** TRANSLATORS: Default player name, e.g. Player 1 */
			   bformat(_("Player %u"), static_cast<unsigned int>(pn));
			map->set_scenario_player_name(pn, name);
			rows_.at(pn - 1)->name->set_text(name);

			const std::string& tribename = rows_.at(pn - 1)->tribe->get_selected();
			assert(tribename.empty() ||
			       Widelands::tribe_exists(tribename, eia().egbase().all_tribes()));
			map->set_scenario_player_tribe(pn, tribename);
			rows_.at(pn - 1)->box->set_visible(true);
		}
		// Update button states
		set_starting_pos_clicked(menu.tools()->set_starting_pos.get_current_player());
	} else {
		// If a removed player was selected, switch starting pos tool to the highest available player
		if (old_nr_players >= menu.tools()->set_starting_pos.get_current_player()) {
			set_starting_pos_clicked(nr_players);
		}

		// Hide extra players
		map->set_nrplayers(nr_players);
		for (Widelands::PlayerNumber pn = nr_players; pn < old_nr_players; ++pn) {
			rows_.at(pn)->box->set_visible(false);
		}
	}
	menu.set_need_save(true);
	layout();
}

void EditorPlayerMenu::player_tribe_clicked(size_t row) {
	const std::string& tribename = rows_.at(row)->tribe->get_selected();
	assert(tribename.empty() || Widelands::tribe_exists(tribename, eia().egbase().all_tribes()));
	EditorInteractive& menu = eia();
	menu.egbase().mutable_map()->set_scenario_player_tribe(row + 1, tribename);
	menu.set_need_save(true);
}

void EditorPlayerMenu::set_starting_pos_clicked(size_t row) {
	EditorInteractive& menu = eia();
	//  jump to the current node
	Widelands::Map* map = menu.egbase().mutable_map();
	if (Widelands::Coords const sp = map->get_starting_pos(row)) {
		menu.map_view()->scroll_to_field(sp, MapView::Transition::Smooth);
	}

	//  select tool set mplayer
	menu.select_tool(menu.tools()->set_starting_pos, EditorTool::First);
	menu.tools()->set_starting_pos.set_current_player(row);

	//  reselect tool, so everything is in a defined state
	menu.select_tool(menu.tools()->current(), EditorTool::First);

	// Signal player position states via button states
	iterate_player_numbers(pn, map->get_nrplayers()) {
		if (pn == row) {
			rows_.at(pn - 1)->position->set_style(UI::ButtonStyle::kWuiPrimary);
			rows_.at(pn - 1)->position->set_perm_pressed(true);
		} else {
			rows_.at(pn - 1)->position->set_style(UI::ButtonStyle::kWuiSecondary);
			rows_.at(pn - 1)->position->set_perm_pressed(map->get_starting_pos(pn) !=
			                                             Widelands::Coords::null());
		}
	}
}

void EditorPlayerMenu::name_changed(size_t row) {
	//  Player name has been changed.
	const std::string& text = rows_.at(row)->name->text();
	EditorInteractive& menu = eia();
	Widelands::Map* map = menu.egbase().mutable_map();
	map->set_scenario_player_name(row + 1, text);
	menu.set_need_save(true);
}
