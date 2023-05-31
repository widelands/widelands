/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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

#include "ui_fsmenu/addons/login_box.h"

#include "base/string.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "third_party/sha1/sha1.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "wlapplication_options.h"

namespace AddOnsUI {

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

constexpr const char* const kRegisterURL = "https://widelands.org/accounts/register/";

AddOnsLoginBox::AddOnsLoginBox(UI::Panel& parent, UI::WindowStyle style)
   : UI::Window(&parent, style, "login", 0, 0, 100, 100, _("Login")),
     password_sha1_(get_config_string("password_sha1", "")),
     box_(this, panel_style_, 0, 0, UI::Box::Vertical),
     hbox_(&box_, panel_style_, 0, 0, UI::Box::Horizontal),
     left_box_(&hbox_, panel_style_, 0, 0, UI::Box::Vertical),
     right_box_(&hbox_, panel_style_, 0, 0, UI::Box::Vertical),
     buttons_box_(&box_, panel_style_, 0, 0, UI::Box::Horizontal),
     username_(&right_box_, 0, 0, 400, panel_style_),
     password_(&right_box_, 0, 0, 400, panel_style_),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         kRowButtonSize,
         kRowButtonSize,
         style == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuPrimary :
                                             UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             kRowButtonSize,
             kRowButtonSize,
             style == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
                                                 UI::ButtonStyle::kWuiSecondary,
             _("Cancel")),
     reset_(&buttons_box_,
            "reset",
            0,
            0,
            kRowButtonSize,
            kRowButtonSize,
            style == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
                                                UI::ButtonStyle::kWuiSecondary,
            _("Reset")) {
	UI::MultilineTextarea* m =
	   new UI::MultilineTextarea(&box_, 0, 0, 100, 100, panel_style_, "", UI::Align::kLeft,
	                             UI::MultilineTextarea::ScrollMode::kNoScrolling);
	m->set_text(as_richtext(
	   g_style_manager
	      ->font_style(style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                                          UI::FontStyle::kWuiInfoPanelParagraph)
	      .as_font_tag(format(
	         _("In order to use a registered account, you need an account on the Widelands website. "
	           "Please log in at %s and set an online gaming password on your profile page."),
	         g_style_manager
	            ->font_style(style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsTooltip :
                                                                UI::FontStyle::kWuiTooltip)
	            .as_font_tag(as_url_hyperlink(kRegisterURL))))));

	left_box_.add_inf_space();
	left_box_.add(
	   new UI::Textarea(&left_box_, panel_style_,
	                    style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelHeading :
                                                           UI::FontStyle::kWuiInfoPanelHeading,
	                    _("Username:"), UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);
	left_box_.add_inf_space();
	left_box_.add(
	   new UI::Textarea(&left_box_, panel_style_,
	                    style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelHeading :
                                                           UI::FontStyle::kWuiInfoPanelHeading,
	                    _("Password:"), UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);
	left_box_.add_inf_space();

	right_box_.add(&username_, UI::Box::Resizing::kExpandBoth);
	right_box_.add_space(kRowButtonSpacing);
	right_box_.add(&password_, UI::Box::Resizing::kExpandBoth);

	hbox_.add(&left_box_, UI::Box::Resizing::kFullSize);
	hbox_.add_space(kRowButtonSpacing);
	hbox_.add(&right_box_, UI::Box::Resizing::kExpandBoth);

	buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&reset_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(m, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	ok_.sigclicked.connect([this]() { ok(); });
	username_.ok.connect([this]() { ok(); });
	password_.ok.connect([this]() { ok(); });
	cancel_.sigclicked.connect([this]() { die(); });
	username_.cancel.connect([this]() { die(); });
	password_.cancel.connect([this]() { die(); });
	reset_.sigclicked.connect([this]() {
		password_.set_can_focus(false);
		reset();
		password_.set_can_focus(true);
	});
	password_.set_password(true);

	reset();
	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

const std::string& AddOnsLoginBox::get_username() const {
	return username_.get_text();
}

std::string AddOnsLoginBox::get_password() const {
	const std::string& p = password_.get_text();
	return (p.empty() || p == password_sha1_) ? p : crypto::sha1(p);
}

bool AddOnsLoginBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			ok();
			return true;
		case SDLK_ESCAPE:
			die();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void AddOnsLoginBox::think() {
	UI::Window::think();
	ok_.set_enabled(!username_.get_text().empty() && !password_.get_text().empty());
	if (!password_sha1_.empty() && password_.has_focus() && password_.get_text() == password_sha1_) {
		password_.set_text("");
	}
}

void AddOnsLoginBox::ok() {
	if (!username_.get_text().empty() && !password_.get_text().empty()) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	}
}

void AddOnsLoginBox::reset() {
	if (get_config_bool("registered", false)) {
		username_.set_text(get_config_string("nickname", ""));
		password_.set_text(password_sha1_);
	} else {
		username_.set_text("");
		password_.set_text("");
	}
}

}  // namespace AddOnsUI
