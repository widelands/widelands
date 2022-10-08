/*
 * Copyright (C) 2022 by the Widelands Development Team
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

#include "ui_fsmenu/mousewheel_report.h"

#include <SDL.h>

#include "base/i18n.h"
#include "graphic/style_manager.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/menu.h"
#include "ui_fsmenu/tech_info.h"

namespace FsMenu {

static constexpr int kReportWindowWidth = 650;
static constexpr int kReportWindowHeight = 500;
constexpr int16_t kSpacing = 8;

constexpr const char* const kReportURL = "https://github.com/widelands/widelands/discussions/5367";

#if SDL_VERSION_ATLEAST(2, 0, 14)
const std::string url_button_text = gettext_noop("Open Link");
void url_button_action() {
	SDL_OpenURL(kReportURL);
}
#else
const std::string url_button_text = gettext_noop("Copy Link");
void url_button_action() {
	SDL_SetClipboardText(kReportURL);
}
#endif

// Help users give feedback when inverted horizontal scrolling detection is wrong
InvertedScrollFeedbackWindow::InvertedScrollFeedbackWindow(UI::Panel* parent)
   : UI::Window(parent,
                UI::WindowStyle::kFsMenu,
                "inverted_scroll_feedback",
                0,
                0,
                kReportWindowWidth,
                kReportWindowHeight,
                _("Send Feedback for Inverted Horizontal Scrolling")),
     content_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     header_(
        &content_,
        0,
        0,
        kReportWindowWidth,
        0,
        UI::PanelStyle::kFsMenu,
        format(
           "<rt><p>%1$s</p></rt>",
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(
                 /** TRANSLATORS: %s is a URL */
                 format(_("Please help us improve the detection of inverted horizontal scrolling."
                          " To do so, please report at %s that horizontal scroll direction is"
                          " wrong with your configuration. Please include the below technical"
                          " information."),
                        format("<font underline=true>%s</font>", kReportURL)))),
        UI::Align::kLeft,
        UI::MultilineTextarea::ScrollMode::kNoScrolling),
     url_button_(
        &content_, "url", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _(url_button_text)),
     infobox_(&content_, TechInfoBox::Type::kMousewheelReport),
     close_(&content_, "close", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Close")) {
	do_not_layout_on_resolution_change();

	url_button_.sigclicked.connect(url_button_action);
	close_.sigclicked.connect([this]() { die(); });

	content_.add_space(3 * kSpacing);
	content_.add(&header_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	content_.add_space(3 * kSpacing);
	content_.add(&url_button_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	content_.add_inf_space();
	content_.add(&infobox_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	content_.add_inf_space();
	content_.add(&close_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	content_.add_space(kSpacing);

	int32_t min_w = std::min(get_lborder() + infobox_.get_w() + get_rborder(), parent->get_w());
	if (get_w() < min_w) {
		set_size(min_w, get_h());
	}
	content_.set_size(get_inner_w(), get_inner_h());
	header_.set_size(content_.get_inner_w(), 0);
	Vector2i ub_pos = url_button_.get_pos();
	ub_pos.x = (content_.get_inner_w() - url_button_.get_w()) / 3;
	url_button_.set_pos(ub_pos);
	url_button_.set_size(content_.get_inner_w() - 2 * ub_pos.x, url_button_.get_h());
	infobox_.set_size(content_.get_inner_w(), infobox_.get_h());
	layout();
	center_to_parent();
	initialization_complete();
}

}  // namespace FsMenu
