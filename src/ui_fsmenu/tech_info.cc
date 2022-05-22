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

#include "ui_fsmenu/tech_info.h"

#include <SDL.h>
#include <SDL_clipboard.h>

#include "base/i18n.h"
#include "build_info.h"
#include "graphic/font_handler.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "wlapplication.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int16_t kSpacing = 8;

TechInfoBox::TechInfoBox(UI::Panel* parent, TechInfoBox::Type t)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical) {
	struct ContentT {
		std::string label, localized_label, value, localized_value;
	};
	std::vector<ContentT> content;

#define ADD_CONTENT(a, b, c, d) content.emplace_back(ContentT({a, b, c, d}))

	ADD_CONTENT("Version:", _("Version:"), build_ver_details(), "");

	const std::string os =
#if defined(__APPLE__) || defined(__MACH__)
	   gettext_noop("MacOS");
#elif defined(_WIN64)
	   gettext_noop("Windows (64 bit)");
#elif defined(_WIN32)
	   gettext_noop("Windows (32 bit)");
#elif defined(__linux__)
	   gettext_noop("Linux");
#elif defined(__FreeBSD__)
	   gettext_noop("FreeBSD");
#elif defined(__unix) || defined(__unix__)
	   gettext_noop("Unix");
#else
	   gettext_noop("Unknown");
#endif
	ADD_CONTENT("Operating System:", _("Operating System:"), os, _(os));
	ADD_CONTENT("Compiled with SDL version:", _("Compiled with SDL version:"),
	            format("%d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL), "");

	SDL_version sdl_current = {0, 0, 0};
	SDL_GetVersion(&sdl_current);
	if (sdl_current.major != SDL_MAJOR_VERSION || sdl_current.minor != SDL_MINOR_VERSION ||
	    sdl_current.patch != SDL_PATCHLEVEL) {
		ADD_CONTENT("Using SDL version:", _("Using SDL version:"),
		            format("%d.%d.%d", sdl_current.major, sdl_current.minor, sdl_current.patch), "");
	}

	ADD_CONTENT(
	   "SDL video driver:", _("SDL video driver:"), std::string(SDL_GetCurrentVideoDriver()), "");

	// Doesn't hurt if we report it in About too
	if (get_mousewheel_option_bool(MousewheelOptionID::kInvertedXDetected)) {
		ADD_CONTENT("SDL horizontal scroll:", _("SDL horizontal scroll:"), "assuming inverted",
		            _("assuming inverted"));
	}

	if (t != TechInfoBox::Type::kMousewheelReport) {
		ADD_CONTENT("Locale:", _("Locale:"), i18n::get_locale(), "");
		ADD_CONTENT("Home Directory:", _("Home Directory:"), i18n::get_homedir(), "");
		ADD_CONTENT("Configuration File:", _("Configuration File:"), get_config_file(), "");
		ADD_CONTENT("Data Directory:", _("Data Directory:"), WLApplication::get()->get_datadir(), "");
		ADD_CONTENT("Locale Directory:", _("Locale Directory:"), i18n::get_localedir(), "");
		ADD_CONTENT(
		   "Executable Directory:", _("Executable Directory:"), get_executable_directory(false), "");
	}

/**** Done filling content *****/
#undef ADD_CONTENT

	const bool mirror = UI::g_fh->fontset()->is_rtl();
	add_space(kSpacing);
	add(new UI::Textarea(this, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsGameSetupHeadings,
	                     _("Technical Info"), UI::Align::kCenter),
	    UI::Box::Resizing::kFullSize);
	add_space(2 * kSpacing);

	UI::Box* hbox1 = new UI::Box(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox2 = new UI::Box(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal);
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
		vbox2->add(
		   new UI::Textarea(vbox2, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuInfoPanelParagraph,
		                    c.localized_value.empty() ? c.value : c.localized_value,
		                    UI::mirror_alignment(UI::Align::kRight, mirror)),
		   UI::Box::Resizing::kExpandBoth);
	}

	UI::Button* copy = new UI::Button(hbox2, "copy", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary,
	                                  _("Copy"), _("Copy the technical report to the clipboard"));
	copy->sigclicked.connect([report]() { SDL_SetClipboardText(report.c_str()); });

	hbox1->add_space(kSpacing);
	hbox1->add(mirror ? vbox2 : vbox1, UI::Box::Resizing::kExpandBoth);
	hbox1->add_inf_space();
	hbox1->add(mirror ? vbox1 : vbox2, UI::Box::Resizing::kExpandBoth);
	hbox1->add_space(kSpacing);

	hbox2->add_inf_space();
	hbox2->add(copy, UI::Box::Resizing::kExpandBoth);
	hbox2->add_inf_space();

	add(hbox1, UI::Box::Resizing::kFullSize);
	add_space(3 * kSpacing);
	add(hbox2, UI::Box::Resizing::kFullSize);
}

}  // namespace FsMenu
