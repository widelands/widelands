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

#include "ui_fsmenu/mapdetailsbox.h"

#include "graphic/image_cache.h"
#include "map_io/map_loader.h"

// Helper functions for localizable assembly of info strings

static std::string tribe_of(const GameSettings& game_settings, const PlayerSettings& p) {
	for (const Widelands::TribeBasicInfo& tribeinfo : game_settings.tribes) {
		if (tribeinfo.name == p.tribe) {
			return tribeinfo.descname;
		}
	}
	return g_style_manager->font_style(UI::FontStyle::kDisabled)
	   .as_font_tag((boost::format(_("invalid tribe ‘%s’")) % p.tribe).str());
}

static std::string assemble_infotext_for_savegame(const GameSettings& game_settings) {
	std::string infotext_fmt = "<rt>%s<vspace gap=28><p>";
	for (unsigned i = game_settings.players.size(); i; --i) {
		infotext_fmt += "%s";
		infotext_fmt += i > 1 ? "<br>" : "</p></rt>";
	}
	boost::format infotext(infotext_fmt + "</p></rt>");

	infotext % g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)
	              .as_font_tag(_("Saved Players"));

	for (unsigned i = 0; i < game_settings.players.size(); ++i) {
		const PlayerSettings& current_player = game_settings.players.at(i);

		if (current_player.state == PlayerSettings::State::kClosed) {
			infotext % g_style_manager->font_style(UI::FontStyle::kDisabled)
			              .as_font_tag((boost::format(_("Player %u: –")) % (i + 1)).str());
			continue;
		}

		// Check if this is a list of names, or just one name:
		std::string name;
		if (current_player.name.compare(0, 1, " ")) {
			name = current_player.name;
		} else {
			std::string temp = current_player.name;
			std::vector<std::string> names;
			while (temp.find(' ', 1) < temp.size()) {
				const uint32_t x = temp.find(' ', 1);
				names.push_back(temp.substr(1, x));
				temp = temp.substr(x + 1, temp.size());
			}
			name = i18n::localize_list(names, i18n::ConcatenateWith::AMPERSAND);
		}

		infotext %
		   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		      .as_font_tag((boost::format(
		                       /** TRANSLATORS: "Player 1 (Barbarians): Playername" */
		                       _("Player %1$u (%2$s): %3$s")) %
		                    (i + 1) % tribe_of(game_settings, current_player) %
		                    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		                       .as_font_tag(name))
		                      .str());
	}

	return infotext.str();
}

static std::string assemble_infotext_for_map(const Widelands::Map& map,
                                             const GameSettings& game_settings) {
	std::string infotext_fmt = "<rt>%s<vspace gap=28><p>%s<br>%s<br>%s";
	if (!map.get_hint().empty()) {
		infotext_fmt += "<br>%s";
	}
	boost::format infotext(infotext_fmt + "</p></rt>");

	infotext % g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)
	              .as_font_tag(game_settings.scenario ? _("Scenario Details") : _("Map Details"));
	infotext %
	   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	      .as_font_tag(
	         (boost::format(_("Size: %s")) %
	          g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	             .as_font_tag(
	                (boost::format(_("%1$u×%2$u")) % map.get_width() % map.get_height()).str()))
	            .str());
	infotext % g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	              .as_font_tag((boost::format(_("Players: %s")) %
	                            g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                               .as_font_tag(std::to_string(game_settings.players.size())))
	                              .str());
	infotext % g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	              .as_font_tag((boost::format(_("Description: %s")) %
	                            g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                               .as_font_tag(map.get_description()))
	                              .str());
	if (!map.get_hint().empty()) {
		infotext %
		   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		      .as_font_tag((boost::format(_("Hint: %s")) %
		                    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		                       .as_font_tag(map.get_hint()))
		                      .str());
	}

	return infotext.str();
}

// MapDetailsBox implementation

MapDetailsBox::MapDetailsBox(Panel* parent,
                             uint32_t,
                             uint32_t standard_element_height,
                             uint32_t padding)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     title_(this,
            0,
            0,
            0,
            0,
            _("Map"),
            UI::Align::kCenter,
            g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)),
     title_box_(this, 0, 0, UI::Box::Horizontal),
     content_box_(this, 0, 0, UI::Box::Vertical),
     map_name_(&title_box_,
               0,
               0,
               0,
               0,
               _("No map selected"),
               UI::Align::kLeft,
               g_style_manager->font_style(UI::FontStyle::kLabel)),
     select_map_(&title_box_,
                 "change_map_or_save",
                 0,
                 0,
                 standard_element_height,
                 standard_element_height,
                 UI::ButtonStyle::kFsMenuSecondary,
                 g_image_cache->get("images/wui/menus/toggle_minimap.png"),
                 _("Change map or saved game")),
     map_description_(&content_box_,
                      0,
                      0,
                      UI::Scrollbar::kSize,  // min width must be set to avoid assertion failure...
                      0,
                      UI::PanelStyle::kFsMenu,
                      "",
                      UI::Align::kLeft,
                      UI::MultilineTextarea::ScrollMode::kNoScrolling) {
	content_box_.set_scrolling(true);
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add_space(3 * padding);
	title_box_.add(&map_name_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	title_box_.add_inf_space();
	title_box_.add(&select_map_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	add(&title_box_, UI::Box::Resizing::kFullSize);
	add_space(3 * padding);
	add(&content_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add(&map_description_, UI::Box::Resizing::kExpandBoth);
	content_box_.add_space(3 * padding);
}
MapDetailsBox::~MapDetailsBox() {
}

void MapDetailsBox::update_from_savegame(GameSettingsProvider* settings) {
	const GameSettings& game_settings = settings->settings();

	select_map_.set_visible(settings->can_change_map());
	select_map_.set_enabled(settings->can_change_map());

	show_map_description_savegame(game_settings);
	show_map_name(game_settings);
}

void MapDetailsBox::show_map_description_savegame(const GameSettings& game_settings) {
	set_map_description_text(assemble_infotext_for_savegame(game_settings));
}

void MapDetailsBox::update(GameSettingsProvider* settings, Widelands::Map& map) {
	const GameSettings& game_settings = settings->settings();

	select_map_.set_visible(settings->can_change_map());
	select_map_.set_enabled(settings->can_change_map());

	show_map_name(game_settings);
	show_map_description(map, settings);
}

void MapDetailsBox::show_map_name(const GameSettings& game_settings) {
	// Translate the map's name
	const char* nomap = _("(no map)");
	i18n::Textdomain td("maps");
	map_name_.set_text(game_settings.mapname.size() != 0 ? _(game_settings.mapname) : nomap);
}

void MapDetailsBox::show_map_description(Widelands::Map& map, GameSettingsProvider* settings) {
	set_map_description_text(assemble_infotext_for_map(map, settings->settings()));
}

void MapDetailsBox::set_select_map_action(const std::function<void()>& action) {
	select_map_.sigclicked.connect(action);
}

void MapDetailsBox::force_new_dimensions(float scale,
                                         uint32_t standard_element_width,
                                         uint32_t standard_element_height) {
	title_.set_font_scale(scale);
	map_name_.set_font_scale(scale);
	map_name_.set_fixed_width(standard_element_width - standard_element_height);
	select_map_.set_desired_size(standard_element_height, standard_element_height);
	content_box_.set_max_size(standard_element_width, 6 * standard_element_height);
}

void MapDetailsBox::set_map_description_text(const std::string& text) {
	map_description_.set_style(g_style_manager->font_style(UI::FontStyle::kLabel));
	map_description_.set_text(text);
}
void MapDetailsBox::show_warning(const std::string& text) {
	map_description_.set_style(g_style_manager->font_style(UI::FontStyle::kWarning));
	map_description_.set_text(text);
}
