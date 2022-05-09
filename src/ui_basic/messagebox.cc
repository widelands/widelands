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

#include "ui_basic/messagebox.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/text_layout.h"
#include "ui_basic/window.h"

namespace UI {

WLMessageBox::WLMessageBox(Panel* const parent,
                           const WindowStyle s,
                           const std::string& caption,
                           std::string text,
                           const MBoxType type,
                           Align align)
   : Window(parent, s, "message_box", 0, 0, 20, 20, caption), type_(type) {
	// Calculate textarea dimensions depending on text size
	const int outerwidth = parent != nullptr ? parent->get_inner_w() : g_gr->get_xres();
	const int outerheight = parent != nullptr ? parent->get_inner_h() : g_gr->get_yres();

	const int button_w = 120;
	const int minwidth = 3.5 * button_w;

	// Ample space for the buttons, but not hugely wide
	const int maxwidth = std::min(600, std::max(outerwidth * 2 / 3, minwidth));
	// Make sure that there is space for buttons + message, but not too tall
	const int maxheight = std::min(260, std::max(outerheight * 2 / 3, 200));

	const UI::FontStyle font_style =
	   s == WindowStyle::kWui ? UI::FontStyle::kWuiLabel : UI::FontStyle::kFsMenuLabel;

	const int margin = 5;
	int width;
	int height = 0;
	try {
		std::shared_ptr<const UI::RenderedText> temp_rendered_text =
		   g_fh->render(is_richtext(text) ? text : as_richtext_paragraph(text, font_style), maxwidth);
		width = temp_rendered_text->width();
		height = temp_rendered_text->height();
	} catch (const std::exception& e) {
		log_err("Invalid richtext: %s", e.what());
		text = richtext_escape(text);
		std::shared_ptr<const UI::RenderedText> temp_rendered_text =
		   g_fh->render(as_richtext_paragraph(text, font_style), maxwidth);
		width = temp_rendered_text->width();
		height = temp_rendered_text->height();
	}

	// Stupid heuristic to avoid excessively long lines
	if (height < 2 * text_height(font_style)) {
		std::shared_ptr<const UI::RenderedText> temp_rendered_text =
		   g_fh->render(as_richtext_paragraph(text, font_style), maxwidth / 2);
		width = temp_rendered_text->width();
		height = temp_rendered_text->height();
	}

	// Make sure that the buttons really fit
	width = std::max(std::min(width, maxwidth), minwidth);

	// Find out whether the textarea needs a scrollbar
	MultilineTextarea::ScrollMode scrollmode = MultilineTextarea::ScrollMode::kNoScrolling;
	if (height > maxheight) {
		height = maxheight;
		scrollmode = MultilineTextarea::ScrollMode::kScrollNormal;
	}

	textarea_.reset(
	   new MultilineTextarea(this, margin, margin, width - 2 * margin, height,
	                         s == WindowStyle::kWui ? UI::PanelStyle::kWui : UI::PanelStyle::kFsMenu,
	                         text, align, scrollmode));

	// Now add the buttons
	const int button_y = textarea_->get_y() + textarea_->get_h() + 2 * margin;
	const int left_button_x = width / 3 - button_w / 2;
	const int right_button_x = width * 2 / 3 - button_w / 2;

	ok_button_.reset(new Button(
	   this, "ok",
	   type_ == MBoxType::kOk        ? (width - button_w) / 2 :
	   UI::g_fh->fontset()->is_rtl() ? left_button_x :
                                      right_button_x,
	   button_y, button_w, 0,
	   s == WindowStyle::kWui ? UI::ButtonStyle::kWuiPrimary : UI::ButtonStyle::kFsMenuPrimary,
	   _("OK")));
	ok_button_->sigclicked.connect([this]() { clicked_ok(); });

	if (type_ == MBoxType::kOkCancel) {
		cancel_button_.reset(
		   new Button(this, "cancel", UI::g_fh->fontset()->is_rtl() ? right_button_x : left_button_x,
		              button_y, button_w, 0,
		              s == WindowStyle::kWui ? UI::ButtonStyle::kWuiSecondary :
                                             UI::ButtonStyle::kFsMenuSecondary,
		              _("Cancel")));
		cancel_button_->sigclicked.connect([this]() { clicked_back(); });
	}

	set_inner_size(width, button_y + ok_button_->get_h() + margin);
	center_to_parent();
	focus();

	initialization_complete();
}

/**
 * Handle mouseclick.
 *
 * Clicking the right mouse button inside the window acts like pressing
 * Ok or No, depending on the message box type.
 */
bool WLMessageBox::handle_mousepress(const uint8_t btn, int32_t /*mx*/, int32_t /*my*/) {
	if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		if (type_ == MBoxType::kOk) {
			clicked_ok();
		} else {
			clicked_back();
		}
	}
	return true;
}

bool WLMessageBox::handle_mouserelease(const uint8_t /*btn*/, int32_t /*mx*/, int32_t /*my*/) {
	return true;
}

bool WLMessageBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			if (type_ == MBoxType::kOk) {
				clicked_ok();
			} else {
				clicked_back();
			}
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::Window::handle_key(down, code);
}

void WLMessageBox::clicked_ok() {
	set_visible(false);
	ok();
	if (is_modal()) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	}
}

void WLMessageBox::clicked_back() {
	set_visible(false);
	cancel();
	if (is_modal()) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}
}
}  // namespace UI
