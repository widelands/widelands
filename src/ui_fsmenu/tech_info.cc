/*
 * Copyright (C) 2016-2024 by the Widelands Development Team
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
#include "graphic/style_manager.h"
#include "graphic/styles/paragraph_style.h"
#include "graphic/text_layout.h"
#include "logic/addons.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "wlapplication.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"

namespace {

std::pair<std::vector<std::string>, std::vector<std::string>> list_addons() {
	std::vector<std::string> enabled;
	std::vector<std::string> disabled;
	for (auto& addon : AddOns::g_addons) {
		if (addon.second) {
			enabled.emplace_back(addon.first->internal_name);
		} else {
			disabled.emplace_back(addon.first->internal_name);
		}
	}
	return make_pair(enabled, disabled);
}

std::string merge_list(const std::vector<std::string>& list, const std::string& separator,
                       const bool localize) {
	std::string rv;
	auto it = list.begin();
	while (it != list.end()) {
		rv += localize ? _(*it) : *it;
		if (++it != list.end()) {
			rv += separator;
		}
	}
	return rv;
}

}  // namespace

namespace FsMenu {

constexpr int16_t kSpacing = 8;

// Smallest allowed width of the about window (see ui_fsmenu/main.cc) minus borders, etc.
constexpr int kMinWidth = 520;

TechInfo::TechInfo(const TechInfo::Type t) {
	add_plain_entry(gettext_noop("Version:"), build_ver_details(), false);

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
	add_plain_entry(gettext_noop("Operating System:"), os, true);
	add_plain_entry(gettext_noop("Compiled with SDL version:"),
	                format("%d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL),
                   false);

	SDL_version sdl_current = {0, 0, 0};
	SDL_GetVersion(&sdl_current);
	if (sdl_current.major != SDL_MAJOR_VERSION || sdl_current.minor != SDL_MINOR_VERSION ||
	    sdl_current.patch != SDL_PATCHLEVEL) {
		add_plain_entry(gettext_noop("Using SDL version:"),
		                format("%d.%d.%d", sdl_current.major, sdl_current.minor, sdl_current.patch),
		                false);
	}

	add_plain_entry(gettext_noop("SDL video driver:"), std::string(SDL_GetCurrentVideoDriver()), false);

	// Doesn't hurt if we report it in About too
	if (get_mousewheel_option_bool(MousewheelOptionID::kInvertedXDetected)) {
		add_plain_entry(gettext_noop("SDL horizontal scroll:"), gettext_noop("assuming inverted"), true);
	}

	if (t != TechInfo::Type::kMousewheelReport) {
		add_plain_entry(gettext_noop("Locale:"), i18n::get_locale(), false);
		add_plain_entry(gettext_noop("Home Directory:"), i18n::get_homedir(), false);
		add_plain_entry(gettext_noop("Configuration File:"), get_config_file(), false);
		add_plain_entry(gettext_noop("Data Directory:"), WLApplication::get().get_datadir(), false);
		add_plain_entry(gettext_noop("Locale Directory:"), i18n::get_localedir(), false);
		add_plain_entry(gettext_noop("Executable Directory:"), get_executable_directory(false), false);

		auto addons_lists = list_addons();
		entries_.emplace_back(Entry {gettext_noop("Enabled Add-Ons:"), addons_lists.first, false});
		entries_.emplace_back(Entry {gettext_noop("Inactive Installed Add-Ons:"), addons_lists.second, false});
	}
}

inline void TechInfo::add_plain_entry(
   const std::string& label, const std::string& single_value, const bool localize_value) {
	entries_.emplace_back(Entry {label, std::vector<std::string> {single_value}, localize_value});
}

std::string TechInfo::get_markdown() const {
	std::string report;
	for (const Entry& entry : entries_) {
		report += format("> %s %s  \n", entry.label, merge_list(entry.values, ", ", false));
	}
	return report;
}

std::string TechInfo::get_richtext() const {
	const bool rtl = UI::g_fh->fontset()->is_rtl();
	const std::string label_alignment = rtl ? "right" : "left";

	// This will be added before long first values to help the richtext renderer to put long lines
	// below the label. For LTR, a non-breaking plus a normal space work to make wrapping automatic
	// depending on the available space. Unfortunately in RTL text these stay on the left, so there
	// we always add an empty first line.
	const std::string first_wrapping = rtl ? "&nbsp;<br>\n" : "&nbsp; ";

	const UI::FontStyleInfo& label_font = g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading);
	const UI::FontStyleInfo& value_font = g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph);

	std::string report = "<rt>\n";
	for (const Entry& entry : entries_) {
		std::string localized_label = _(entry.label);

		// Pevent wrapping the label
		replace_all(localized_label, " ", "&nbsp;");

		// Start a <div> for the entry
		report += "<div width=100%%>";

		// Put the label in a floating box, aligned according to the writing direction.
		// Paragraph alignment is mirrored by the richtext renderer for RTL.
		report += format("<div float=%1$s><p align=left>%2$s</p></div>", label_alignment,
		                 label_font.as_font_tag(localized_label));

		// Check for long first values
		bool wrap = !entry.values.empty() &&
		               text_width(localized_label, label_font) +
		               // Only some short ones are translatable, let's keep it simple...
		               text_width(entry.values[0], value_font) > kMinWidth;

		// TODO(tothxa): text_width() is not reliable for RTL (bidi?) (make wrap const above if fixed)
		if (rtl && !entry.values.empty() && !wrap) {
			// Unicode is hard, use untranslated strings for approximation...
			const int label_approx = entry.label.length() * label_font.size() * 3 / 5;  // bold
			const int value_approx = entry.values[0].length() * value_font.size() * 5 / 9;
			wrap = label_approx + value_approx > kMinWidth;
		}

		std::string values = wrap ? first_wrapping : "";
		values += merge_list(entry.values, "<br>\n", entry.localize_values);

		// The content is a plain paragraph that can flow around the label.
		report += format("<p align=right>%s<vspace gap=%d></p>", value_font.as_font_tag(values), kSpacing);

		// End the <div> of the entry
		report += "</div>\n";
	}
	report += "</rt>\n";
	return report;
}

TechInfoBox::TechInfoBox(UI::Panel* parent, TechInfo::Type t)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, "tech_info_box", 0, 0, UI::Box::Vertical),
     // Size will be overridden by layout()
     report_area_(this, "tech_info", 0, 0, 100, 100, UI::PanelStyle::kFsMenu) {

	const TechInfo tech_info(t);
	markdown_report_ = tech_info.get_markdown();
	richtext_report_ = tech_info.get_richtext();

	const UI::ParagraphStyleInfo& title_style =
	   g_style_manager->paragraph_style(UI::ParagraphStyle::kAboutTitle);
	int space = title_style.space_before();
	if (space > 0) {
		add_space(space);
	}
	UI::Textarea* title = new UI::Textarea(this, UI::PanelStyle::kFsMenu, "tech_info_heading",
	                                       UI::FontStyle::kFsGameSetupHeadings, _("Technical Info"),
	                                       UI::Align::kCenter);
	add(title, UI::Box::Resizing::kFullSize);
	if (t == TechInfo::Type::kAbout) {
		title->set_style_override(title_style.font());
	}
	space = title_style.space_after();
	if (space > 0) {
		add_space(space);
	}

	report_area_.set_text(richtext_report_);
	add(&report_area_, UI::Box::Resizing::kExpandBoth);

	UI::Box* buttonbox =
	   new UI::Box(this, UI::PanelStyle::kFsMenu, "buttons_box", 0, 0, UI::Box::Horizontal);
	UI::Button* copy =
	   new UI::Button(buttonbox, "copy", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Copy"),
	                  _("Copy the technical report to the clipboard"));
	copy->sigclicked.connect([this]() { SDL_SetClipboardText(markdown_report_.c_str()); });

	buttonbox->add_inf_space();
	buttonbox->add(copy, UI::Box::Resizing::kExpandBoth);
	buttonbox->add_inf_space();

	add_space(3 * kSpacing);
	add(buttonbox, UI::Box::Resizing::kFullSize);
	add_space(kSpacing);
}

}  // namespace FsMenu
