/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_fsmenu/mapdetailsbox.h"

#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "map_io/map_loader.h"

// Helper functions for localizable assembly of info strings

static std::string tribe_of(const GameSettings& game_settings, const PlayerSettings& p) {
	if (p.random_tribe) {
		return pgettext("tribe", "Random");
	}
	for (const Widelands::TribeBasicInfo& tribeinfo : game_settings.tribes) {
		if (tribeinfo.name == p.tribe) {
			return tribeinfo.descname;
		}
	}
	return g_style_manager->font_style(UI::FontStyle::kDisabled)
	   .as_font_tag(format(_("invalid tribe ‘%s’"), p.tribe));
}

static std::string assemble_infotext_for_savegame(const GameSettings& game_settings) {
	std::string infotext_fmt = "<rt>%s<vspace gap=28><p>";
	for (unsigned i = game_settings.players.size(); i; --i) {
		infotext_fmt += "%s";
		infotext_fmt += i > 1 ? "<br>" : "</p></rt>";
	}
	infotext_fmt += "</p></rt>";
	std::vector<std::string> format_arg_strings;

	format_arg_strings.emplace_back(g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)
	                                   .as_font_tag(_("Saved Players")));

	for (unsigned i = 0; i < game_settings.players.size(); ++i) {
		const PlayerSettings& current_player = game_settings.players.at(i);

		if (current_player.state == PlayerSettings::State::kClosed) {
			format_arg_strings.emplace_back(g_style_manager->font_style(UI::FontStyle::kDisabled)
			                                   .as_font_tag(format(_("Player %u: –"), (i + 1))));
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

		format_arg_strings.emplace_back(
		   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		      .as_font_tag(format(
		         /** TRANSLATORS: "Player 1 (Barbarians): Playername" */
		         _("Player %1$u (%2$s): %3$s"), (i + 1), tribe_of(game_settings, current_player),
		         g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		            .as_font_tag(name))));
	}

	format_impl::ArgsVector fmt_args;
	format_impl::ArgsPair arg;
	arg.first = format_impl::AbstractNode::ArgType::kString;
	for (const std::string& str : format_arg_strings) {
		arg.second.string_val = str.c_str();
		fmt_args.emplace_back(arg);
	}
	return format(infotext_fmt, fmt_args);
}

static std::string assemble_infotext_for_map(const Widelands::Map& map,
                                             const GameSettings& game_settings) {
	std::string infotext_fmt = "<rt>%s<vspace gap=28><p>%s<br>%s<br>%s";
	if (!map.get_hint().empty()) {
		infotext_fmt += "<br>%s";
	}
	infotext_fmt += "</p></rt>";
	std::vector<std::string> format_arg_strings;

	format_arg_strings.emplace_back(
	   g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)
	      .as_font_tag(game_settings.scenario ? _("Scenario Details") : _("Map Details")));

	format_arg_strings.emplace_back(
	   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	      .as_font_tag(
	         format(_("Size: %s"),
	                g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                   .as_font_tag(format(_("%1$u×%2$u"), map.get_width(), map.get_height())))));

	format_arg_strings.emplace_back(
	   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	      .as_font_tag(format(
	         _("Players: %s"), g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                              .as_font_tag(std::to_string(game_settings.players.size())))));

	format_arg_strings.emplace_back(
	   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
	      .as_font_tag(format(_("Description: %s"),
	                          g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
	                             .as_font_tag(richtext_escape(map.get_description())))));

	if (!map.get_hint().empty()) {
		format_arg_strings.emplace_back(
		   g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		      .as_font_tag(format(
		         _("Hint: %s"), g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
		                           .as_font_tag(map.get_hint()))));
	}

	format_impl::ArgsVector fmt_args;
	format_impl::ArgsPair arg;
	arg.first = format_impl::AbstractNode::ArgType::kString;
	for (const std::string& str : format_arg_strings) {
		arg.second.string_val = str.c_str();
		fmt_args.emplace_back(arg);
	}
	return format(infotext_fmt, fmt_args);
}

// MapDetailsBox implementation

MapDetailsBox::MapDetailsBox(Panel* parent, const uint32_t padding)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     padding_(padding),
     title_(this,
            UI::PanelStyle::kFsMenu,
            UI::FontStyle::kFsGameSetupHeadings,
            0,
            0,
            0,
            0,
            _("Map"),
            UI::Align::kCenter),
     title_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     content_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     map_name_(&title_box_,
               UI::PanelStyle::kFsMenu,
               UI::FontStyle::kFsMenuLabel,
               0,
               0,
               0,
               0,
               _("No map selected"),
               UI::Align::kLeft),
     map_description_(&content_box_,
                      0,
                      0,
                      UI::Scrollbar::kSize,  // min width must be set to avoid assertion failure...
                      0,
                      UI::PanelStyle::kFsMenu,
                      "",
                      UI::Align::kLeft,
                      UI::MultilineTextarea::ScrollMode::kNoScrolling),
     suggested_teams_box_(
        &content_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, padding, 0, 0, 0) {
	content_box_.set_scrolling(true);
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add_space(3 * padding);
	title_box_.add(&map_name_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	title_box_.add_inf_space();
	add(&title_box_, UI::Box::Resizing::kFullSize);
	add_space(3 * padding);
	add(&content_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add(&map_description_, UI::Box::Resizing::kExpandBoth);
	content_box_.add_space(3 * padding);
	content_box_.add(&suggested_teams_box_, UI::Box::Resizing::kExpandBoth);
}

void MapDetailsBox::update_from_savegame(GameSettingsProvider* settings) {
	const GameSettings& game_settings = settings->settings();
	show_map_description_savegame(game_settings);
	show_map_name(game_settings);
}

void MapDetailsBox::show_map_description_savegame(const GameSettings& game_settings) {
	set_map_description_text(assemble_infotext_for_savegame(game_settings));
}

void MapDetailsBox::update(GameSettingsProvider* settings, Widelands::Map& map) {
	map_name_.set_text(map.get_name());
	show_map_description(map, settings);
	suggested_teams_box_.show(map.get_suggested_teams());
}

void MapDetailsBox::show_map_name(const GameSettings& game_settings) {
	// Translate the map's name
	const char* nomap = _("(no map)");
	// TODO(Nordfriese): If the map was defined by an add-on, use that add-on's textdomain
	// instead (if available). We'll need to store the add-on name in the savegame for this.
	i18n::Textdomain td("maps");
	map_name_.set_text(!game_settings.mapname.empty() ? _(game_settings.mapname) : nomap);
}

void MapDetailsBox::show_map_description(const Widelands::Map& map,
                                         GameSettingsProvider* settings) {
	set_map_description_text(assemble_infotext_for_map(map, settings->settings()));
}

void MapDetailsBox::force_new_dimensions(uint32_t width, uint32_t height) {
	map_name_.set_fixed_width(width - height);
	content_box_.set_max_size(
	   width, get_h() - title_.get_h() - title_box_.get_h() - 2 * 3 * padding_);
}

void MapDetailsBox::set_map_description_text(const std::string& text) {
	map_description_.set_style(UI::FontStyle::kFsMenuLabel);
	map_description_.set_text(text);
}
void MapDetailsBox::show_warning(const std::string& text) {
	map_description_.set_style(UI::FontStyle::kWarning);
	map_description_.set_text(text);
}
