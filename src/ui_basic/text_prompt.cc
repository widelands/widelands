/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#include "ui_basic/text_prompt.h"

#include "base/i18n.h"
#include "ui_basic/textarea.h"

namespace UI {

constexpr int16_t kButtonSize = 32;
constexpr int16_t kSpacing = 4;

TextPrompt::TextPrompt(UI::Panel& parent,
                       const UI::WindowStyle s,
                       const std::string& title,
                       const std::string& text)
   : UI::Window(&parent, s, "text_prompt", 0, 0, 0, 0, title),
     box_(this, panel_style_, 0, 0, UI::Box::Vertical),
     content_box_(&box_, panel_style_, 0, 0, UI::Box::Vertical),
     buttonsbox_(&box_, panel_style_, 0, 0, UI::Box::Horizontal),
     editbox_(*new UI::EditBox(&box_, 0, 0, 200, panel_style_)),
     ok_(&buttonsbox_,
         "ok",
         0,
         0,
         kButtonSize,
         kButtonSize,
         s == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuPrimary :
                                         UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     cancel_(&buttonsbox_,
             "cancel",
             0,
             0,
             kButtonSize,
             kButtonSize,
             s == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
                                             UI::ButtonStyle::kWuiSecondary,
             _("Cancel")) {
	buttonsbox_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
	buttonsbox_.add_space(kSpacing);
	buttonsbox_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_.add(
	   new UI::Textarea(&box_, panel_style_,
	                    s == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelHeading :
                                                       UI::FontStyle::kWuiInfoPanelHeading,
	                    text),
	   UI::Box::Resizing::kFullSize);
	box_.add_space(kSpacing);
	box_.add(&editbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(kSpacing);
	box_.add(&content_box_, UI::Box::Resizing::kFullSize);
	box_.add_space(kSpacing);
	box_.add(&buttonsbox_, UI::Box::Resizing::kFullSize);

	ok_.sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });
	cancel_.sigclicked.connect([this]() { die(); });
	editbox_.ok.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });
	editbox_.cancel.connect([this]() { die(); });

	set_center_panel(&box_);
	editbox_.focus();
	center_to_parent();
	initialization_complete();
}

bool TextPrompt::handle_key(const bool down, const SDL_Keysym code) {
	if (down && code.sym == SDLK_RETURN) {
		end_modal<Panel::Returncodes>(Panel::Returncodes::kOk);
		return true;
	}
	return Window::handle_key(down, code);
}

}  // namespace UI
