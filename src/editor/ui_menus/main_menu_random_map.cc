/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
#include "logic/single_player_game_settings_provider.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"

namespace {
// The map generator can't find starting positions for too many players
constexpr uint8_t kMaxMapgenPlayers = 8;
constexpr uint8_t kMargin = 4;
}  // namespace

MainMenuNewRandomMapPanel::MainMenuNewRandomMapPanel(UI::Panel& parent,
                                                     UI::PanelStyle s,
                                                     const int32_t inner_w,
                                                     const uint32_t w,
                                                     const uint32_t h,
                                                     UI::Button& ok_button,
                                                     UI::Button& cancel_button)
   : UI::Box(&parent, s, kMargin, kMargin, UI::Box::Vertical, 0, 0, kMargin),
     label_style_(s == UI::PanelStyle::kWui ? UI::FontStyle::kWuiLabel :
                                              UI::FontStyle::kFsMenuLabel),
     // UI elements
     label_height_(text_height(label_style_) + 2),
     generator_(this,
                "generator",
                0,
                0,
                inner_w,
                8,
                label_height_,
                _("Map Generator"),
                UI::DropdownType::kTextual,
                panel_style_,
                panel_style_ == UI::PanelStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                                       UI::ButtonStyle::kFsMenuSecondary),
     // Size
     map_size_box_(*this, panel_style_, "random_map_menu", 4, w, h),
     max_players_(compute_max_players()),
     players_(this,
              0,
              0,
              inner_w,
              inner_w / 3,
              max_players_,
              1,
              max_players_,
              panel_style_,
              _("Players:"),
              UI::SpinBox::Units::kNone,
              UI::SpinBox::Type::kSmall),
     // World + Resources
     current_world_(RNG::static_rand(Widelands::Map::kOldWorldNames.size())),
     resource_amounts_({
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("Low"),
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("Medium"),
        /** TRANSLATORS: Amount of resources in the random map generator in the editor */
        _("High"),
     }),

     world_(this,
            "world",
            0,
            0,
            inner_w,
            8,
            label_height_,
            _("Climate"),
            UI::DropdownType::kTextual,
            panel_style_,
            panel_style_ == UI::PanelStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                                   UI::ButtonStyle::kFsMenuSecondary),
     resources_(this,
                "resources",
                0,
                0,
                inner_w,
                8,
                label_height_,
                _("Resources"),
                UI::DropdownType::kTextual,
                panel_style_,
                panel_style_ == UI::PanelStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                                       UI::ButtonStyle::kFsMenuSecondary),
     terrains_distribution_(this,
                            "terrains_distribution",
                            0,
                            0,
                            inner_w,
                            8,
                            label_height_,
                            _("Terrain Distribution"),
                            UI::DropdownType::kTextual,
                            panel_style_,
                            panel_style_ == UI::PanelStyle::kWui ?
                               UI::ButtonStyle::kWuiSecondary :
                               UI::ButtonStyle::kFsMenuSecondary),

     water_(this,
            0,
            0,
            inner_w,
            inner_w / 3,
            waterval_,
            0,
            60,
            panel_style_,
            _("Water:"),
            UI::SpinBox::Units::kPercent,
            UI::SpinBox::Type::kSmall,
            5),
     land_(this,
           0,
           0,
           inner_w,
           inner_w / 3,
           landval_,
           0,
           100,
           panel_style_,
           _("Land:"),
           UI::SpinBox::Units::kPercent,
           UI::SpinBox::Type::kSmall,
           5),
     wasteland_(this,
                0,
                0,
                inner_w,
                inner_w / 3,
                wastelandval_,
                0,
                70,
                panel_style_,
                _("Wasteland:"),
                UI::SpinBox::Units::kPercent,
                UI::SpinBox::Type::kSmall,
                5),
     mountains_box_(this, panel_style_, 0, 0, UI::Box::Horizontal, 0, 0, kMargin),
     mountains_label_(&mountains_box_, panel_style_, label_style_, 0, 0, 0, 0, _("Mountains:")),
     mountains_(&mountains_box_,
                panel_style_,
                label_style_,
                0,
                0,
                inner_w / 3,
                mountains_label_.get_h(),
                format(_("%i %%"), mountainsval_),
                UI::Align::kCenter),
     island_mode_(this, panel_style_, Vector2i::zero(), _("Island mode")),
     // Geeky stuff
     map_number_and_id_hbox_(this, panel_style_, 0, 0, UI::Box::Horizontal, 0, 0, kMargin),
     map_number_and_id_vbox_1_(
        &map_number_and_id_hbox_, panel_style_, 0, 0, UI::Box::Vertical, 0, 0, kMargin),
     map_number_and_id_vbox_2_(
        &map_number_and_id_hbox_, panel_style_, 0, 0, UI::Box::Vertical, 0, 0, kMargin),
     random_number_hbox_(
        &map_number_and_id_vbox_2_, panel_style_, 0, 0, UI::Box::Horizontal, 0, 0, kMargin),
     map_number_label_(
        &map_number_and_id_vbox_1_, panel_style_, label_style_, 0, 0, 0, 0, _("Random number:")),
     map_number_edit_(&random_number_hbox_,
                      0,
                      0,
                      inner_w - 2 * kMargin - map_number_label_.get_w(),
                      panel_style_),
     map_number_randomize_(&random_number_hbox_,
                           "new_random_number",
                           0,
                           0,
                           map_number_edit_.get_h(),
                           map_number_edit_.get_h(),
                           UI::ButtonStyle::kWuiSecondary,
                           g_image_cache->get("images/ui_fsmenu/random.png"),
                           _("Generate a new random number")),
     map_id_label_(
        &map_number_and_id_vbox_1_, panel_style_, label_style_, 0, 0, 0, 0, _("Map ID:")),
     map_id_edit_(&map_number_and_id_vbox_2_,
                  0,
                  0,
                  inner_w - 2 * kMargin - map_id_label_.get_w(),
                  panel_style_),
     ok_button_(ok_button),
     cancel_button_(cancel_button) {
	set_size(inner_w, 50);  // Prevent assert failures

	generator_.set_enabled(false);
	generator_.add(_("Default"), nullptr, g_image_cache->get("images/logos/wl-ico-64.png"), true);
	for (const auto& addon : AddOns::g_addons) {
		if (addon.second && addon.first->category == AddOns::AddOnCategory::kMapGenerator) {
			generator_.add(addon.first->descname(), addon.first, addon.first->icon, false);
			generator_.set_enabled(true);
		}
	}
	add(&generator_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// ---------- Width + Height ----------

	map_size_box_.set_selection_function([this] { button_clicked(ButtonId::kMapSize); });
	add(&map_size_box_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// ---------- Players -----------

	add(&players_, UI::Box::Resizing::kExpandBoth);

	add_space(kMargin);

	players_.changed.connect([this]() { button_clicked(ButtonId::kPlayers); });

	// ---------- Worlds ----------

	world_.add(_("Random"), Widelands::Map::kOldWorldNames.size(), nullptr, true);
	for (size_t i = 0; i < Widelands::Map::kOldWorldNames.size(); ++i) {
		world_.add(Widelands::Map::kOldWorldNames[i].descname(), i);
	}

	world_.selected.connect([this]() {
		current_world_ = world_.get_selected();
		if (current_world_ == static_cast<int>(Widelands::Map::kOldWorldNames.size())) {
			current_world_ = RNG::static_rand(Widelands::Map::kOldWorldNames.size());
		}
		nr_edit_box_changed();
	});
	add(&world_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// ---------- Amount of Resources (Low/Medium/High) ----------

	for (size_t i = 0; i < resource_amounts_.size(); ++i) {
		resources_.add(resource_amounts_[i], i, nullptr, i == resource_amount_);
	}

	resources_.selected.connect([this]() {
		resource_amount_ = resources_.get_selected();
		nr_edit_box_changed();
	});
	add(&resources_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// Terrains Distribution

	terrains_distribution_.add(_("Default"), TerrainDistribution::kDefault, nullptr, true);
	terrains_distribution_.add(_("Alpine"), TerrainDistribution::kAlpine);
	terrains_distribution_.add(_("Atoll"), TerrainDistribution::kAtoll);
	terrains_distribution_.add(_("Wasteland"), TerrainDistribution::kWasteland);
	terrains_distribution_.add(_("Random"), TerrainDistribution::kRandom);

	select_terrains_distribution();
	terrains_distribution_.selected.connect([this]() { select_terrains_distribution(); });
	add(&terrains_distribution_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// ---------- Water -----------
	water_.get_buttons()[0]->sigclicked.connect([this]() { button_clicked(ButtonId::kWater); });
	water_.get_buttons()[1]->sigclicked.connect([this]() { button_clicked(ButtonId::kWater); });

	add(&water_, UI::Box::Resizing::kExpandBoth);

	// ---------- Land -----------

	land_.get_buttons()[0]->sigclicked.connect([this]() { button_clicked(ButtonId::kLand); });
	land_.get_buttons()[1]->sigclicked.connect([this]() { button_clicked(ButtonId::kLand); });

	add(&land_, UI::Box::Resizing::kExpandBoth);

	// ---------- Wasteland -----------

	wasteland_.get_buttons()[0]->sigclicked.connect(
	   [this]() { button_clicked(ButtonId::kWasteland); });
	wasteland_.get_buttons()[1]->sigclicked.connect(
	   [this]() { button_clicked(ButtonId::kWasteland); });

	add(&wasteland_, UI::Box::Resizing::kExpandBoth);

	// ---------- Mountains -----------

	mountains_box_.add(&mountains_label_);

	// Convince the value label to align with the spinbox labels above
	mountains_box_.add_inf_space();
	mountains_.set_fixed_width(inner_w / 3);
	mountains_box_.add(&mountains_);

	add(&mountains_box_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	// ---------- Island mode ----------

	island_mode_.set_state(true);
	add(&island_mode_, UI::Box::Resizing::kExpandBoth);
	add_space(kMargin);

	island_mode_.changed.connect([this]() { button_clicked(ButtonId::kIslandMode); });

	// ---------- Random map number edit ----------

	map_number_and_id_vbox_1_.add(&map_number_label_, UI::Box::Resizing::kExpandBoth);

	map_number_edit_.changed.connect([this]() { nr_edit_box_changed(); });
	map_number_edit_.ok.connect([this]() { ok_button_.sigclicked(); });
	map_number_ = RNG::static_rand();
	map_number_edit_.set_text(std::to_string(static_cast<unsigned int>(map_number_)));
	map_number_and_id_vbox_2_.add(&random_number_hbox_, UI::Box::Resizing::kExpandBoth);
	random_number_hbox_.add(&map_number_edit_, UI::Box::Resizing::kExpandBoth);

	map_number_randomize_.sigclicked.connect([this]() {
		map_number_edit_.set_text(std::to_string(RNG::static_rand()));
		nr_edit_box_changed();
	});
	random_number_hbox_.add(&map_number_randomize_);

	add_space(kMargin);

	// ---------- Map ID String edit ----------

	map_number_and_id_vbox_1_.add(&map_id_label_, UI::Box::Resizing::kExpandBoth);

	map_id_edit_.set_text("abcd-efgh-ijkl-mnop");
	map_id_edit_.changed.connect([this]() { id_edit_box_changed(); });
	map_id_edit_.ok.connect([this]() { ok_button_.sigclicked(); });
	map_number_and_id_vbox_2_.add(&map_id_edit_, UI::Box::Resizing::kExpandBoth);

	map_number_and_id_hbox_.add(&map_number_and_id_vbox_1_, UI::Box::Resizing::kExpandBoth);
	map_number_and_id_hbox_.add(&map_number_and_id_vbox_2_, UI::Box::Resizing::kExpandBoth);
	add(&map_number_and_id_hbox_, UI::Box::Resizing::kExpandBoth);

	nr_edit_box_changed();
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
void MainMenuNewRandomMapPanel::button_clicked(MainMenuNewRandomMapPanel::ButtonId n) {
	switch (n) {
	case ButtonId::kWater:
		terrains_distribution_.clear_selection();
		waterval_ = water_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kLand:
		terrains_distribution_.clear_selection();
		landval_ = land_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kWasteland:
		terrains_distribution_.clear_selection();
		wastelandval_ = wasteland_.get_value();
		normalize_landmass(n);
		break;
	case ButtonId::kIslandMode:
		break;
	case ButtonId::kPlayers:  // intended fall-through
	case ButtonId::kMapSize:
	case ButtonId::kNone:
		// Restrict maximum players according to map size, but allow at least 2 players.
		max_players_ = compute_max_players();
		players_.set_interval(1, max_players_);
		if (players_.get_value() > max_players_) {
			players_.set_value(max_players_);
		}
		// Make sure that landmass is consistent
		normalize_landmass(n);
	}
	nr_edit_box_changed();  // Update ID String
}

size_t MainMenuNewRandomMapPanel::compute_max_players() const {
	return std::min(static_cast<size_t>(kMaxMapgenPlayers),
	                (find_dimension_index(map_size_box_.selected_width()) +
	                 find_dimension_index(map_size_box_.selected_height())) /
	                      2 +
	                   2);
}

// If the the sum of our landmass is < 0% or > 100% change the mountain value.
// If the mountain value gets out of range, change the other values.
void MainMenuNewRandomMapPanel::normalize_landmass(ButtonId clicked_button) {
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
	mountains_.set_text(format(_("%i %%"), mountainsval_));
}

void MainMenuNewRandomMapPanel::select_terrains_distribution() {
	switch (terrains_distribution_.get_selected()) {
	case TerrainDistribution::kDefault:
		waterval_ = 20;
		landval_ = 55;
		wastelandval_ = 5;
		mountainsval_ = 20;
		break;
	case TerrainDistribution::kAlpine:
		waterval_ = 10;
		landval_ = 35;
		wastelandval_ = 5;
		mountainsval_ = 50;
		break;
	case TerrainDistribution::kAtoll:
		waterval_ = 50;
		landval_ = 30;
		wastelandval_ = 5;
		mountainsval_ = 15;
		break;
	case TerrainDistribution::kWasteland:
		waterval_ = 15;
		landval_ = 35;
		wastelandval_ = 35;
		mountainsval_ = 15;
		break;
	case TerrainDistribution::kRandom: {
		// Decide the values randomly (within reasonable intervals)

		waterval_ = 5 + 5 * RNG::static_rand(7);       // [ 5, 35]
		landval_ = 15 + 5 * RNG::static_rand(6);       // [15, 40]
		mountainsval_ = 10 + 5 * RNG::static_rand(6);  // [10, 35]

		unsigned sum = waterval_ + landval_ + mountainsval_;
		assert(sum % 5 == 0);
		assert(sum >= 30);
		assert(sum <= 110);

		while (sum < 50) {
			if (landval_ < mountainsval_) {
				landval_ += 5;
			} else if (waterval_ < landval_) {
				waterval_ += 5;
			} else {
				mountainsval_ += 5;
			}
			sum = waterval_ + landval_ + mountainsval_;
		}

		while (sum > 100) {
			if (landval_ > mountainsval_) {
				landval_ -= 5;
			} else if (waterval_ > landval_) {
				waterval_ -= 5;
			} else {
				mountainsval_ -= 5;
			}
			sum = waterval_ + landval_ + mountainsval_;
		}

		wastelandval_ = 100 - sum;
	} break;
	default:
		NEVER_HERE();
	}

	assert(waterval_ + landval_ + wastelandval_ + mountainsval_ == 100);
	normalize_landmass(ButtonId::kNone);  // update spinboxes
	nr_edit_box_changed();
}

bool MainMenuNewRandomMapPanel::do_generate_map(Widelands::EditorGameBase& egbase,
                                                EditorInteractive* eia,
                                                SinglePlayerGameSettingsProvider* sp) {
	ok_button_.set_enabled(false);
	cancel_button_.set_enabled(false);

	assert((eia == nullptr) ^ (sp == nullptr));
	assert((sp == nullptr) ^ egbase.is_game());

	Widelands::Map* map = egbase.mutable_map();

	// TODO(Nordfriese): Perhaps support add-ons in the future?
	// Any new units defined by add-ons, and any modifications made to the default units,
	// can not be taken into account when generating a map currently.

	if (eia != nullptr) {
		egbase.create_loader_ui({"editor"}, true, "", kEditorSplashImage, false);
		eia->cleanup_for_load();

		egbase.enabled_addons().clear();
		egbase.init_addons(true);

		// cleanup_for_load() deleted the world and tribes – reload them now
		EditorInteractive::load_world_units(eia, egbase);
	} else {
		egbase.enabled_addons().clear();
		egbase.init_addons(true);

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
	      << "ID = " << map_id_edit_.text() << "\n"
	      << "Generator = "
	      << (generator_.get_selected() == nullptr ? "default" :
                                                    generator_.get_selected()->internal_name)
	      << "\n";

	map->create_empty_map(egbase, map_info.w, map_info.h, 0, _("No Name"),
	                      get_config_string("realname", pgettext("author_name", "Unknown")),
	                      sstrm.str());
	map->set_nrplayers(map_info.numPlayers);
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
	log_info("Generator:     %s\n", generator_.get_selected() == nullptr ?
                                      "default" :
                                      generator_.get_selected()->internal_name.c_str());
	log_info("\n");

	bool result;
	if (generator_.get_selected() == nullptr) {
		Widelands::MapGenerator gen(*map, map_info, egbase);
		result = gen.create_random_map();
	} else {
		try {
			LuaInterface& lua = egbase.lua();
			lua.interpret_string(format("kClimate = \"%s\"", map_info.world_name));
			lua.interpret_string(
			   format("kResourceAmount = %d", static_cast<int>(map_info.resource_amount)));
			lua.interpret_string(format("kWater = %d", map_info.waterRatio * 100));
			lua.interpret_string(format("kLand = %d", map_info.landRatio * 100));
			lua.interpret_string(format("kWasteland = %d", map_info.wastelandRatio * 100));
			lua.interpret_string(format("kIslandMode = %b", map_info.islandMode));
			lua.interpret_string(format("kRandomNumber = %d", map_info.mapNumber));
			lua.interpret_string(format("kMapID = \"%s\"", map_id_edit_.text()));
			lua.run_script(kAddOnDir + FileSystem::file_separator() +
			               generator_.get_selected()->internal_name + FileSystem::file_separator() +
			               "init.lua");
			result = true;
		} catch (const std::exception& e) {
			log_err("Map generation Lua error: %s", e.what());
			result = false;
		}
	}

	egbase.create_tempfile_and_save_mapdata(FileSystem::ZIP);

	map->recalc_whole_map(egbase);
	map->set_background_theme(map_info.world_name);
	if (eia != nullptr) {
		eia->map_changed(EditorInteractive::MapWas::kReplaced);
		egbase.remove_loader_ui();

		UI::WLMessageBox mbox(
		   eia, UI::WindowStyle::kWui,
		   /** TRANSLATORS: Window title. This is shown after a random map has been created in the
		     editor.*/
		   _("Random Map"),
		   result ?
              /** TRANSLATORS: This is shown after a random map has been created in the editor. */
              /** TRANSLATORS: You don't need to be literal with your translation, */
              /** TRANSLATORS: as long as the user understands that he needs to check the player
                positions.*/
              _("The map has been generated. Please double-check the player starting positions to "
              "make sure that your carriers won’t drown, or be stuck on an island or on top of a "
              "mountain.") :
              /** TRANSLATORS: This is shown after a random map has been created in the editor. */
              /** TRANSLATORS: You don't need to be literal with your translation, as long as */
              /** TRANSLATORS: the user understands that the map's quality is pretty poor.*/
              _("The map generator could not produce a good-enough map with the provided settings. "
              "This map will probably need extensive editing to be playable. Consider creating a "
              "new map with different parameters."),
		   UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
	} else {
		const unsigned nr_players = map->get_nrplayers();
		if (result) {
			// Check that the starting positions are not too close
			for (unsigned i = 1; i <= nr_players && result; ++i) {
				for (unsigned j = 1; j < i && result; ++j) {
					result &= (map->calc_distance(map->get_starting_pos(i), map->get_starting_pos(j)) >
					           Widelands::kMinSpaceAroundPlayers);
				}
			}
		}

		if (result) {
			// Initialize with some good default values

			const unsigned plnum = RNG::static_rand(nr_players);

			map->set_name(_("Random Map"));
			map->set_author(_("The Widelands Random Map Generator"));
			map->set_description(
			   _("This map was generated automatically by the Widelands Random Map Generator."));

			sp->set_map("", "", map_info.world_name, "", nr_players, false);
			sp->set_scenario(false);
			sp->set_player_number(plnum);
			sp->set_peaceful_mode(false);
			sp->set_fogless(false);
			sp->set_custom_starting_positions(false);

			for (unsigned p = 0; p < nr_players; ++p) {
				sp->set_player_name(
				   p, p == plnum ? _("Player") : format(_("Computer %u"), (p > plnum ? p : p + 1)));
				sp->set_player_tribe(p, "", true);
				sp->set_player_team(p, p == plnum ? 0 : 1);
				sp->set_player_init(p, 0);
			}
		}
	}

	ok_button_.set_enabled(true);
	cancel_button_.set_enabled(true);
	return result;
}

void MainMenuNewRandomMapPanel::id_edit_box_changed() {
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

		terrains_distribution_.select(TerrainDistribution::kCustom);

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

void MainMenuNewRandomMapPanel::nr_edit_box_changed() {

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

void MainMenuNewRandomMapPanel::set_map_info(Widelands::UniqueRandomMapInfo& map_info) const {
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

MainMenuNewRandomMap::MainMenuNewRandomMap(UI::Panel& parent,
                                           UI::UniqueWindow::Registry& r,
                                           const uint32_t w,
                                           const uint32_t h)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "random_map_menu", &r, 400, 500, _("New Random Map")),
     box_(this, panel_style_, 0, 0, UI::Box::Vertical),
     button_box_(&box_, panel_style_, 0, 0, UI::Box::Horizontal, 0, 0, kMargin),
     ok_button_(&button_box_,
                "generate_map",
                0,
                0,
                get_inner_w() / 2 - kMargin,
                0,
                UI::ButtonStyle::kWuiPrimary,
                _("Generate Map")),
     cancel_button_(&button_box_,
                    "generate_map",
                    0,
                    0,
                    get_inner_w() / 2 - kMargin,
                    0,
                    UI::ButtonStyle::kWuiSecondary,
                    _("Cancel")),
     panel_(box_, UI::PanelStyle::kWui, get_inner_w(), w, h, ok_button_, cancel_button_) {
	box_.add(&panel_, UI::Box::Resizing::kAlign);

	cancel_button_.sigclicked.connect([this]() { die(); });
	ok_button_.sigclicked.connect([this]() { clicked_ok(); });
	if (UI::g_fh->fontset()->is_rtl()) {
		button_box_.add(&ok_button_, UI::Box::Resizing::kExpandBoth);
		button_box_.add(&cancel_button_, UI::Box::Resizing::kExpandBoth);
	} else {
		button_box_.add(&cancel_button_, UI::Box::Resizing::kExpandBoth);
		button_box_.add(&ok_button_, UI::Box::Resizing::kExpandBoth);
	}
	box_.add(&button_box_, UI::Box::Resizing::kExpandBoth);

	set_center_panel(&box_);
	center_to_parent();
	initialization_complete();
}

void MainMenuNewRandomMap::clicked_ok() {
	EditorInteractive& eia = dynamic_cast<EditorInteractive&>(*get_parent());
	panel_.do_generate_map(eia.egbase(), &eia, nullptr);
	die();
}

bool MainMenuNewRandomMap::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		default:
			break;
		}
	}
	return UI::UniqueWindow::handle_key(down, code);
}
