/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include <memory>

#include <SDL_clipboard.h>

#include "base/i18n.h"
#include "base/log.h"
#include "build_info.h"
#include "graphic/font_handler.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/multilinetextarea.h"
#include "wlapplication.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int16_t kSpacing = 8;

About::About(MainMenu& fsmm, UI::UniqueWindow::Registry& r)
   : UI::UniqueWindow(&fsmm,
                      UI::WindowStyle::kFsMenu,
                      "about",
                      &r,
                      fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuAbout),
                      fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuAbout),
                      _("About Widelands")),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     tabs_(&box_, UI::TabPanelStyle::kFsMenu),
     close_(&box_, "close", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Close")) {
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> t(lua.run_script("txts/ABOUT.lua"));
		for (const auto& entry : t->array_entries<std::unique_ptr<LuaTable>>()) {
			try {
				tabs_.add_tab(entry->get_string("name"), entry->get_string("script"));
			} catch (LuaError& err) {
				tabs_.add_tab(_("Lua Error"), "");
				log_err("%s", err.what());
			}
		}
	} catch (LuaError& err) {
		tabs_.add_tab(_("Lua Error"), "");
		log_err("%s", err.what());
	}

	{
		struct ContentT {
			std::string label, localized_label, value, localized_value;
		};
		const std::vector<ContentT> content = {
		   {"Version:", _("Version:"), format("%1$s (%2$s)", build_id(), build_type()),
		    format(_("%1$s (%2$s)"), build_id(), build_type())},
		   {"Operating System:", _("Operating System:"),
#if defined(__APPLE__) || defined(__MACH__)
		    "MacOS", _("MacOS")
#elif defined(_WIN64)
		    "Windows (64 bit)", _("Windows (64 bit)")
#elif defined(_WIN32)
		    "Windows (32 bit)", _("Windows (32 bit)")
#elif defined(__linux__)
		    "Linux", _("Linux")
#elif defined(__FreeBSD__)
		    "FreeBSD", _("FreeBSD")
#elif defined(__unix) || defined(__unix__)
		    "Unix", _("Unix")
#else
		    "Unknown", _("Unknown")
#endif
		   },
		   {"Locale:", _("Locale:"), i18n::get_locale(), ""},
		   {"Home Directory:", _("Home Directory:"), i18n::get_homedir(), ""},
		   {"Configuration File:", _("Configuration File:"), get_config_file(), ""},
		   {"Data Directory:", _("Data Directory:"), WLApplication::get()->get_datadir(), ""},
		   {"Locale Directory:", _("Locale Directory:"), i18n::get_localedir(), ""},
		   {"Executable Directory:", _("Executable Directory:"), get_executable_directory(false), ""},
		};
		const bool mirror = UI::g_fh->fontset()->is_rtl();
		UI::Box* infobox = new UI::Box(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical);
		infobox->add_space(kSpacing);
		infobox->add(
		   new UI::Textarea(infobox, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsGameSetupHeadings,
		                    _("Technical Info"), UI::Align::kCenter),
		   UI::Box::Resizing::kFullSize);
		infobox->add_space(2 * kSpacing);

		UI::Box* hbox1 = new UI::Box(infobox, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal);
		UI::Box* hbox2 = new UI::Box(infobox, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal);
		UI::Box* vbox1 = new UI::Box(hbox1, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical);
		UI::Box* vbox2 = new UI::Box(hbox1, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical);
		std::string report;
		for (const ContentT& c : content) {
			report += "> ";
			report += c.label;
			report += ' ';
			report += c.value;
			report += "  \n";
			vbox1->add_space(kSpacing);
			vbox2->add_space(kSpacing);
			vbox1->add(
			   new UI::Textarea(vbox1, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuInfoPanelHeading,
			                    c.localized_label, UI::mirror_alignment(UI::Align::kLeft, mirror)),
			   UI::Box::Resizing::kExpandBoth);
			vbox2->add(new UI::Textarea(vbox2, UI::PanelStyle::kFsMenu,
			                            UI::FontStyle::kFsMenuInfoPanelParagraph,
			                            c.localized_value.empty() ? c.value : c.localized_value,
			                            UI::mirror_alignment(UI::Align::kRight, mirror)),
			           UI::Box::Resizing::kExpandBoth);
		}

		UI::Button* copy =
		   new UI::Button(hbox2, "copy", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Copy"),
		                  _("Copy the technical report to the clipboard"));
		copy->sigclicked.connect([report]() { SDL_SetClipboardText(report.c_str()); });

		hbox1->add_space(kSpacing);
		hbox1->add(mirror ? vbox2 : vbox1, UI::Box::Resizing::kExpandBoth);
		hbox1->add_inf_space();
		hbox1->add(mirror ? vbox1 : vbox2, UI::Box::Resizing::kExpandBoth);
		hbox1->add_space(kSpacing);

		hbox2->add_inf_space();
		hbox2->add(copy, UI::Box::Resizing::kExpandBoth);
		hbox2->add_inf_space();

		infobox->add(hbox1, UI::Box::Resizing::kFullSize);
		infobox->add_space(3 * kSpacing);
		infobox->add(hbox2, UI::Box::Resizing::kFullSize);

		tabs_.add_tab_without_script("info", _("Technical Info"), infobox);
	}

	close_.sigclicked.connect([this]() { die(); });

	box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kPadding);
	box_.add(&close_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add_space(kPadding);

	do_not_layout_on_resolution_change();

	layout();
	tabs_.load_tab_contents();
	center_to_parent();
	initialization_complete();
}

bool About::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			die();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void About::layout() {
	UI::Window::layout();
	if (!is_minimal()) {
		box_.set_size(get_inner_w(), get_inner_h());
	}
}

}  // namespace FsMenu
