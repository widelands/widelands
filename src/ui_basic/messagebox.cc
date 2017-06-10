/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_basic/messagebox.h"

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "ui_basic/window.h"

namespace UI {

WLMessageBox::WLMessageBox(Panel* const parent,
                           const std::string& caption,
                           const std::string& text,
                           const MBoxType type,
                           Align align)
   : Window(parent, "message_box", 0, 0, 20, 20, caption.c_str()), type_(type) {
	// Calculate textarea dimensions depending on text size
	const int outerwidth = parent ? parent->get_inner_w() : g_gr->get_xres();
	const int outerheight = parent ? parent->get_inner_h() : g_gr->get_yres();

	const int button_w = 120;
	const int minwidth = 3.5 * button_w;

	// Ample space for the buttons, but not hugely wide
	const int maxwidth = std::min(600, std::max(outerwidth * 2 / 3, minwidth));
	// Make sure that there is space for buttons + message, but not too tall
	const int maxheight = std::min(400, std::max(outerheight * 2 / 3, 200));

	const int button_space = 50;
	const int margin = 5;
	int width, height = 0;
	{
		std::shared_ptr<const UI::RenderedText> temp_rendered_text =
		   g_fh1->render(as_uifont(text), maxwidth);
		width = temp_rendered_text->width();
		height = temp_rendered_text->height();
	}

	// Stupid heuristic to avoid excessively long lines
	if (height < 2 * UI_FONT_SIZE_SMALL) {
		std::shared_ptr<const UI::RenderedText> temp_rendered_text =
		   g_fh1->render(as_uifont(text), maxwidth / 2);
		width = temp_rendered_text->width();
		height = temp_rendered_text->height();
	}

	// Make sure that the buttons really fit
	width = std::max(std::min(width, maxwidth), minwidth);
	height += button_space;

	// Find out whether the textarea needs a scrollbar
	MultilineTextarea::ScrollMode scrollmode = MultilineTextarea::ScrollMode::kNoScrolling;
	if (height > maxheight) {
		height = maxheight - button_space;
		scrollmode = MultilineTextarea::ScrollMode::kScrollNormal;
	}

	textarea_.reset(new MultilineTextarea(this, margin, margin, width - 2 * margin, height, UI::Panel::Style::kWui, text,
	                                      align,
	                                      scrollmode));

	// Now add the buttons
	const int button_y = textarea_->get_y() + textarea_->get_h() + 2 * margin;
	const int left_button_x = width / 3 - button_w / 2;
	const int right_button_x = width * 2 / 3 - button_w / 2;

	ok_button_.reset(new Button(
	   this, "ok",
	   type_ == MBoxType::kOk ? (width - button_w) / 2 :
	                            UI::g_fh1->fontset()->is_rtl() ? left_button_x : right_button_x,
	   button_y, button_w, 0, g_gr->images().get("images/wui/button_main.png"), _("OK")));
	ok_button_->sigclicked.connect(boost::bind(&WLMessageBox::clicked_ok, boost::ref(*this)));

	if (type_ == MBoxType::kOkCancel) {
		cancel_button_.reset(new Button(
		   this, "cancel", UI::g_fh1->fontset()->is_rtl() ? right_button_x : left_button_x, button_y,
		   button_w, 0, g_gr->images().get("images/wui/button_secondary.png"), _("Cancel")));
		cancel_button_->sigclicked.connect(
		   boost::bind(&WLMessageBox::clicked_back, boost::ref(*this)));
	}

	set_inner_size(width, button_y + ok_button_->get_h() + margin);
	center_to_parent();
	focus();
}

/**
 * Handle mouseclick.
 *
 * Clicking the right mouse button inside the window acts like pressing
 * Ok or No, depending on the message box type.
 */
bool WLMessageBox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
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

bool WLMessageBox::handle_mouserelease(const uint8_t, int32_t, int32_t) {
	return true;
}

bool WLMessageBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
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
	return UI::Panel::handle_key(down, code);
}

void WLMessageBox::clicked_ok() {
	ok();
	if (is_modal())
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}

void WLMessageBox::clicked_back() {
	cancel();
	if (is_modal())
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}
}
