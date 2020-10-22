/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_random_map.h"

#include <cstdlib>
#include <sstream>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/map_generator.h"
#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/world/world.h"
#include "logic/single_player_game_settings_provider.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"

namespace {
// The map generator can't find starting positions for too many players
constexpr uint8_t kMaxMapgenPlayers = 8;
}  // namespace

MainMenuNewRandomMap::MainMenuNewRandomMap(UI::Panel& parent,
                                           UI::WindowStyle s,
                                           UI::UniqueWindow::Registry& registry,
                                           const uint32_t w,
                                           const uint32_t h)
   : UI::UniqueWindow(&parent, s, "random_map_menu", &registry, 400, 500, _("New Random Map")),
     style_(s),
     // UI elements
     margin_(4),
     box_width_(get_inner_w() - 2 * margin_),
     label_height_(text_height(UI::FontStyle::kLabel) + 2),
     box_(this, margin_, margin_, UI::Box::Vertical, 0, 0, margin_),
     // Size
     map_size_box_(box_, "random_map_menu", 4, w, h),
     max_players_(2),
     players_(&box_,
              0,
              0,
              box_width_,
              box_width_ / 3,
              2,
              1,
              max_players_,
              UI::PanelStyle::kWui,
              _("Players:"),
              UI::SpinBox::Units::kNone,
              UI::SpinBox::Type::kSmall),
     // World + Resources
     current_world_(std::rand() % Widelands::Map::kOldWorldNames.size()),  // NOLINT
     resource_amounts_({
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("Low"),
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("Medium"),
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("High"),
     }),
     resource_amount_(2),
     world_(&box_,
            "world",
            0,
            0,
            box_width_,
            8,
            label_height_,
            _("Climate"),
            UI::DropdownType::kTextual,
            UI::PanelStyle::kWui,
            UI::ButtonStyle::kWuiSecondary),
     resources_(&box_,
                "resources",
                0,
                0,
                box_width_,
                8,
                label_height_,
                _("Resources"),
                UI::DropdownType::kTextual,
                UI::PanelStyle::kWui,
                UI::ButtonStyle::kWuiSecondary),
     // Terrain
     waterval_(20),
     landval_(60),
     wastelandval_(0),
     mountainsval_(100 - waterval_ - landval_ - wastelandval_),
     water_(&box_,
            0,
            0,
            box_width_,
            box_width_ / 3,
            waterval_,
            0,
            60,
            UI::PanelStyle::kWui,
            _("Water:"),
            UI::SpinBox::Units::kPercent,
            UI::SpinBox::Type::kSmall,
            5),
     land_(&box_,
           0,
           0,
           box_width_,
           box_width_ / 3,
           landval_,
           0,
           100,
           UI::PanelStyle::kWui,
           _("Land:"),
           UI::SpinBox::Units::kPercent,
           UI::SpinBox::Type::kSmall,
           5),
     wasteland_(&box_,
                0,
                0,
                box_width_,
                box_width_ / 3,
                wastelandval_,
                0,
                70,
                UI::PanelStyle::kWui,
                _("Wasteland:"),
                UI::SpinBox::Units::kPercent,
                UI::SpinBox::Type::kSmall,
                5),
     mountains_box_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, margin_),
     mountains_label_(&mountains_box_, 0, 0, 0, 0, _("Mountains:")),
     mountains_(&mountains_box_,
                0,
                0,
                box_width_ / 3,
                mountains_label_.get_h(),
                (boost::format(_("%i %%")) % mountainsval_).str(),
                UI::Align::kCenter),
     island_mode_(&box_, UI::PanelStyle::kWui, Vector2i::zero(), _("Island mode")),
     // Geeky stuff
     map_number_and_id_hbox_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, margin_),
     map_number_and_id_vbox_1_(&map_number_and_id_hbox_, 0, 0, UI::Box::Vertical, 0, 0, margin_),
     map_number_and_id_vbox_2_(&map_number_and_id_hbox_, 0, 0, UI::Box::Vertical, 0, 0, margin_),
     map_number_label_(&map_number_and_id_vbox_1_, 0, 0, 0, 0, _("Random number:")),
     map_number_edit_(&map_number_and_id_vbox_2_,
                      0,
                      0,
                      box_width_ - 2 * margin_ - map_number_label_.get_w(),
                      UI::PanelStyle::kWui),
     map_id_label_(&map_number_and_id_vbox_1_, 0, 0, 0, 0, _("Map ID:")),
     map_id_edit_(&map_number_and_id_vbox_2_,
                  0,
                  0,
                  box_width_ - 2 * margin_ - map_id_label_.get_w(),
                  UI::PanelStyle::kWui),

     // Buttons
     button_box_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, margin_),
     ok_button_(&button_box_,
                "generate_map",
                0,
                0,
                box_width_ / 2 - margin_,
                0,
                UI::ButtonStyle::kWuiPrimary,
                _("Generate Map")),
     cancel_button_(&button_box_,
                    "generate_map",
                    0,
                    0,
                    box_width_ / 2 - margin_,
                    0,
                    UI::ButtonStyle::kWuiSecondary,
                    _("Cancel")) {
	box_.set_size(100, 20);  // Prevent assert failures

	// ---------- Width + Height ----------

	map_size_box_.set_selection_function([this] { button_clicked(ButtonId::kMapSize); });
	box_.add(&map_size_box_, UI::Box::Resizing::kExpandBoth);

	// ---------- Players -----------

	box_.add(&players_, UI::Box::Resizing::kExpandBoth);

	box_.add_space(margin_);

	players_.changed.connect([this]() { button_clicked(ButtonId::kPlayers); });

	// ---------- Worlds ----------

	world_.add(_("Random"), Widelands::Map::kOldWorldNames.size(), nullptr, true);
	for (size_t i = 0; i < Widelands::Map::kOldWorldNames.size(); ++i) {
		world_.add(Widelands::Map::kOldWorldNames[i].descname(), i);
	}

	world_.selected.connect([this]() {
		current_world_ = world_.get_selected();
		if (current_world_ == static_cast<int>(Widelands::Map::kOldWorldNames.size())) {
			current_world_ = std::rand() % Widelands::Map::kOldWorldNames.size();  // NOLINT
		}
		nr_edit_box_changed();
	});
	box_.add(&world_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(margin_);

	// ---------- Amount of Resources (Low/Medium/High) ----------

	for (size_t i = 0; i < resource_amounts_.size(); ++i) {
		resources_.add(resource_amounts_[i], i, nullptr, i == resource_amount_);
	}

	resources_.selected.connect([this]() {
		resource_amount_ = resources_.get_selected();
		nr_edit_box_changed();
	});
	box_.add(&resources_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(margin_);

	// ---------- Water -----------
	water_.get_buttons()[0]->sigclicked.connect([this]() { button_clicked(ButtonId::kWater); });
	water_.get_buttons()[1]->sigclicked.connect([this]() { button_clicked(ButtonId::kWater); });

	box_.add(&water_, UI::Box::Resizing::kExpandBoth);

	// ---------- Land -----------

	land_.get_buttons()[0]->sigclicked.connect([this]() { button_clicked(ButtonId::kLand); });
	land_.get_buttons()[1]->sigclicked.connect([this]() { button_clicked(ButtonId::kLand); });

	box_.add(&land_, UI::Box::Resizing::kExpandBoth);

	// ---------- Wasteland -----------

	wasteland_.get_buttons()[0]->sigclicked.connect(
	   [this]() { button_clicked(ButtonId::kWasteland); });
	wasteland_.get_buttons()[1]->sigclicked.connect(
	   [this]() { button_clicked(ButtonId::kWasteland); });

	box_.add(&wasteland_, UI::Box::Resizing::kExpandBoth);

	// ---------- Mountains -----------

	mountains_box_.add(&mountains_label_);

	// Convince the value label to align with the spinbox labels above
	mountains_box_.add_space(box_width_ - box_width_ / 6 - mountains_label_.get_w() -
	                         mountains_.get_w() + margin_ + 3);
	mountains_.set_fixed_width(box_width_ / 3 - margin_);
	mountains_box_.add(&mountains_);
	mountains_box_.set_size(box_width_, mountains_label_.get_h());

	box_.add(&mountains_box_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(margin_);

	// ---------- Island mode ----------

	island_mode_.set_state(true);
	box_.add(&island_mode_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(margin_);

	island_mode_.changed.connect([this]() { button_clicked(ButtonId::kIslandMode); });

	// ---------- Random map number edit ----------

	map_number_and_id_vbox_1_.add(&map_number_label_, UI::Box::Resizing::kExpandBoth);

	map_number_edit_.changed.connect([this]() { nr_edit_box_changed(); });
	RNG rng;
	rng.seed(clock());
	rng.rand();
	map_number_ = rng.rand();
	map_number_edit_.set_text(std::to_string(static_cast<unsigned int>(map_number_)));
	map_number_and_id_vbox_2_.add(&map_number_edit_, UI::Box::Resizing::kExpandBoth);

	box_.add_space(margin_);

	// ---------- Map ID String edit ----------

	map_number_and_id_vbox_1_.add(&map_id_label_, UI::Box::Resizing::kExpandBoth);

	map_id_edit_.set_text("abcd-efgh-ijkl-mnop");
	map_id_edit_.changed.connect([this]() { id_edit_box_changed(); });
	map_number_and_id_vbox_2_.add(&map_id_edit_, UI::Box::Resizing::kExpandBoth);

	map_number_and_id_hbox_.add(&map_number_and_id_vbox_1_, UI::Box::Resizing::kExpandBoth);
	map_number_and_id_hbox_.add(&map_number_and_id_vbox_2_, UI::Box::Resizing::kExpandBoth);
	box_.add(&map_number_and_id_hbox_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(margin_);

	// ---------- "Generate Map" button ----------
	cancel_button_.sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });
	ok_button_.sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });
	if (UI::g_fh->fontset()->is_rtl()) {
		button_box_.add(&ok_button_, UI::Box::Resizing::kExpandBoth);
		button_box_.add(&cancel_button_, UI::Box::Resizing::kExpandBoth);
	} else {
		button_box_.add(&cancel_button_, UI::Box::Resizing::kExpandBoth);
		button_box_.add(&ok_button_, UI::Box::Resizing::kExpandBoth);
	}
	box_.add(&button_box_, UI::Box::Resizing::kExpandBoth);

	set_center_panel(&box_);

	nr_edit_box_changed();
	center_to_parent();
}

bool MainMenuNewRandomMap::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
			return true;
		default:
			break;
		}
	}
	return UI::UniqueWindow::handle_key(down, code);
}

// Helper function for setting the highest number of allowed players dependent on the map size
static size_t find_dimension_index(int32_t value) {
	size_t result = 0;
	for (; result < Widelands::kMapDimensions.size() && Widelands::kMapDimensions[result] < value;
	     ++result) {
	}
	return result;
}
/**
 * Called, when button get clicked
 */
void MainMenuNewRandomMap::button_clicked(MainMenuNewRandomMap::ButtonId n) {
	switch (n) {
	case ButtonId::kWater:
		waterval_ = water_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kLand:
		landval_ = land_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kWasteland:
		wastelandval_ = wasteland_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kIslandMode:
		break;
	case ButtonId::kPlayers:  // intended fall-through
	case ButtonId::kMapSize:
	case ButtonId::kNone:
		// Restrict maximum players according to map size, but allow at least 2 players.
		max_players_ = std::min(static_cast<size_t>(kMaxMapgenPlayers),
		                        (find_dimension_index(map_size_box_.selected_width()) +
		                         find_dimension_index(map_size_box_.selected_height())) /
		                              2 +
		                           2);
		players_.set_interval(1, max_players_);
		if (players_.get_value() > max_players_) {
			players_.set_value(max_players_);
		}
		// Make sure that landmass is consistent
		normalize_landmass(n);
	}
	nr_edit_box_changed();  // Update ID String
}

// If the the sum of our landmass is < 0% or > 100% change the mountain value.
// If the mountain value gets out of range, change the other values.
void MainMenuNewRandomMap::normalize_landmass(ButtonId clicked_button) {
	int32_t sum_without_mountainsval = waterval_ + landval_ + wastelandval_;

	// Prefer changing mountainsval to keep consistency with old behaviour
	while (sum_without_mountainsval + mountainsval_ > 100) {
		--mountainsval_;
	}
	while (sum_without_mountainsval + mountainsval_ < 100) {
		++mountainsval_;
	}

	// Compensate if mountainsval got above 100% / below 0%
	while (mountainsval_ < 0) {
		if (clicked_button != ButtonId::kWasteland && wastelandval_ > 0) {
			wastelandval_ -= 5;
			mountainsval_ += 5;
		}
		if (mountainsval_ < 0 && clicked_button != ButtonId::kLand && landval_ > 0) {
			landval_ -= 5;
			mountainsval_ += 5;
		}
		if (mountainsval_ < 0 && clicked_button != ButtonId::kWater && waterval_ > 0) {
			waterval_ -= 5;
			mountainsval_ += 5;
		}
	}

	while (mountainsval_ > 100) {
		if (clicked_button != ButtonId::kWasteland && wastelandval_ < 100) {
			wastelandval_ += 5;
			mountainsval_ -= 5;
		}
		if (mountainsval_ > 100 && clicked_button != ButtonId::kLand && landval_ < 100) {
			landval_ += 5;
			mountainsval_ -= 5;
		}
		if (mountainsval_ > 100 && clicked_button != ButtonId::kWater && waterval_ < 100) {
			waterval_ += 5;
			mountainsval_ -= 5;
		}
	}

	water_.set_value(waterval_);
	land_.set_value(landval_);
	wasteland_.set_value(wastelandval_);
	mountains_.set_text((boost::format(_("%i %%")) % mountainsval_).str());
}

bool MainMenuNewRandomMap::do_generate_map(Widelands::EditorGameBase& egbase,
                                           EditorInteractive* eia,
                                           SinglePlayerGameSettingsProvider* sp) {
	ok_button_.set_enabled(false);
	cancel_button_.set_enabled(false);

	assert((eia == nullptr) ^ (sp == nullptr));
	assert((sp == nullptr) ^ egbase.is_game());

	Widelands::Map* map = egbase.mutable_map();

	if (eia) {
		egbase.create_loader_ui({"editor"}, true, "", kEditorSplashImage);
		eia->cleanup_for_load();
	} else {
		egbase.cleanup_for_load();
	}

	Widelands::UniqueRandomMapInfo map_info;
	set_map_info(map_info);

	std::stringstream sstrm;
	sstrm << "Random generated map\nRandom number = " << map_info.mapNumber << "\n"
	      << "Water = " << waterval_ << " %\n"
	      << "Land = " << landval_ << " %\n"
	      << "Wasteland = " << wastelandval_ << " %\n"
	      << "Resources = " << resource_amounts_[resources_.get_selected()] << "\n"
	      << "ID = " << map_id_edit_.text() << "\n";

	Widelands::MapGenerator gen(*map, map_info, egbase);
	map->create_empty_map(egbase, map_info.w, map_info.h, 0, _("No Name"),
	                      get_config_string("realname", pgettext("author_name", "Unknown")),
	                      sstrm.str().c_str());
	Notifications::publish(UI::NoteLoadingMessage(_("Generating random map…")));

	log_info("============== Generating Map ==============\n");
	log_info("ID:            %s\n", map_id_edit_.text().c_str());
	log_info("Random number: %u\n", map_info.mapNumber);
	log_info("Dimensions:    %d x %d\n", map_info.w, map_info.h);
	log_info("Players:       %d\n", map_info.numPlayers);
	log_info("World:         %s\n", map_info.world_name.c_str());
	switch (map_info.resource_amount) {
	case Widelands::UniqueRandomMapInfo::ResourceAmount::raLow:
		log_info("Resources:     low\n");
		break;
	case Widelands::UniqueRandomMapInfo::ResourceAmount::raMedium:
		log_info("Resources:     medium\n");
		break;
	case Widelands::UniqueRandomMapInfo::ResourceAmount::raHigh:
		log_info("Resources:     high\n");
		break;
	}
	log_info("Land: %0.2f  Water: %0.2f  Wasteland: %0.2f\n", map_info.landRatio,
	         map_info.waterRatio, map_info.wastelandRatio);
	if (map_info.islandMode) {
		log_info("Using Island Mode\n");
	}
	log_info("\n");

	const bool result = gen.create_random_map();

	egbase.create_tempfile_and_save_mapdata(FileSystem::ZIP);

	map->recalc_whole_map(egbase);
	map->set_background_theme(map_info.world_name);
	if (eia) {
		eia->map_changed(EditorInteractive::MapWas::kReplaced);
		egbase.remove_loader_ui();

		UI::WLMessageBox mbox(
		   eia, style_,
		   /** TRANSLATORS: Window title. This is shown after a random map has been created in the
		     editor.*/
		   _("Random Map"),
		   /** TRANSLATORS: This is shown after a random map has been created in the editor. */
		   /** TRANSLATORS: You don't need to be literal with your translation, */
		   /** TRANSLATORS: as long as the user understands that he needs to check the player
		     positions.*/
		   _("The map has been generated. Please double-check the player starting positions to make "
		     "sure that your carriers won’t drown, or be stuck on an island or on top of a "
		     "mountain."),
		   UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
	} else {
		if (result) {
			// Initialize with some good default values

			const unsigned nr_players = map->get_nrplayers();
			const unsigned plnum = std::rand() % nr_players;  // NOLINT

			map->set_name(_("Random Map"));
			map->set_author(_("The Widelands Random Map Generator"));
			map->set_description(
			   _("This map was generated automatically by the Widelands Random Map Generator."));
			map->set_waterway_max_length((std::rand() % 5) * (std::rand() % 6));  // NOLINT

			sp->set_map("", "", map_info.world_name, "", nr_players, false);
			sp->set_scenario(false);
			sp->set_player_number(plnum);
			sp->set_peaceful_mode(false);
			sp->set_custom_starting_positions(false);

			for (unsigned p = 0; p < nr_players; ++p) {
				sp->set_player_name(
				   p, p == plnum ? _("Player") :
				                   (boost::format(_("Computer %u")) % (p > plnum ? p : p + 1)).str());
				sp->set_player_tribe(p, "", true);
				sp->set_player_team(p, p == plnum ? 0 : 1);
				sp->set_player_init(p, 0);
			}
		} else {
			ok_button_.set_enabled(true);
			cancel_button_.set_enabled(true);
		}
	}

	return result;
}

void MainMenuNewRandomMap::id_edit_box_changed() {
	Widelands::UniqueRandomMapInfo map_info;

	std::string str = map_id_edit_.text();

	std::vector<std::string> world_names;
	world_names.reserve(Widelands::Map::kOldWorldNames.size());
	for (const Widelands::Map::OldWorldInfo& descr : Widelands::Map::kOldWorldNames) {
		world_names.push_back(descr.name);
	}

	if (!Widelands::UniqueRandomMapInfo::set_from_id_string(map_info, str, world_names)) {
		ok_button_.set_enabled(false);
	} else {
		std::stringstream sstrm;
		sstrm << map_info.mapNumber;
		map_number_edit_.set_text(sstrm.str());

		map_size_box_.select_width(map_info.w);
		map_size_box_.select_height(map_info.h);

		players_.set_interval(1, map_info.numPlayers);  // hack to make sure we can set the value
		players_.set_value(map_info.numPlayers);

		landval_ = map_info.landRatio * 100.0 + 0.49;
		waterval_ = map_info.waterRatio * 100.0 + 0.49;
		wastelandval_ = map_info.wastelandRatio * 100.0 + 0.49;

		resource_amount_ = map_info.resource_amount;
		resources_.select(resource_amount_);

		current_world_ = std::find(world_names.cbegin(), world_names.cend(), map_info.world_name) -
		                 world_names.cbegin();
		world_.select(current_world_);

		island_mode_.set_state(map_info.islandMode);

		// Update other values in UI as well
		button_clicked(ButtonId::kNone);

		ok_button_.set_enabled(true);
	}
}

void MainMenuNewRandomMap::nr_edit_box_changed() {

	try {
		std::string const text = map_number_edit_.text();
		std::stringstream sstrm(text);
		unsigned int number;
		sstrm >> number;

		if (!sstrm.fail()) {
			map_number_ = number;

			Widelands::UniqueRandomMapInfo map_info;
			set_map_info(map_info);

			std::string id_string;
			Widelands::UniqueRandomMapInfo::generate_id_string(id_string, map_info);

			map_id_edit_.set_text(id_string);

			ok_button_.set_enabled(true);
		} else {
			ok_button_.set_enabled(false);
		}
	} catch (...) {
		ok_button_.set_enabled(false);
	}
}

void MainMenuNewRandomMap::set_map_info(Widelands::UniqueRandomMapInfo& map_info) const {
	map_info.w = map_size_box_.selected_width();
	map_info.h = map_size_box_.selected_height();
	map_info.waterRatio = static_cast<double>(waterval_) / 100.0;
	map_info.landRatio = static_cast<double>(landval_) / 100.0;
	map_info.wastelandRatio = static_cast<double>(wastelandval_) / 100.0;
	map_info.mapNumber = map_number_;
	map_info.islandMode = island_mode_.get_state();
	map_info.numPlayers = players_.get_value();
	map_info.resource_amount =
	   static_cast<Widelands::UniqueRandomMapInfo::ResourceAmount>(resource_amount_);
	map_info.world_name = Widelands::Map::kOldWorldNames[current_world_].name;
}
