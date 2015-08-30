/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
#include "base/log.h" // NOCOM
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

namespace UI {


struct WLMessageBoxImpl {
	MultilineTextarea * textarea;
	WLMessageBox::MBoxType type;
};


WLMessageBox::WLMessageBox
	(Panel * const parent,
	 const std::string & caption,
	 const std::string & text,
	 const MBoxType type,
	 Align align)
	:
	Window(parent, "message_box", 0, 0, 20, 20, caption.c_str()),
	d(new WLMessageBoxImpl)
{
	d->type = type;
	d->textarea = new MultilineTextarea
		(this,
		 5, 5, 30, 30,
		 text.c_str(), align);

	const int32_t outerwidth = parent ?
		parent->get_inner_w() : g_gr->get_xres();
	const int32_t outerheight = parent ?
		parent->get_inner_h() : g_gr->get_yres();
	assert(outerwidth >= 80);
	assert(outerheight >= 60);
	const int32_t maxwidth = outerwidth - 80;
	const int32_t maxheight = outerheight - 60;
	uint32_t width, height;
	std::string font = d->textarea->get_font_name();
	int32_t fontsize = d->textarea->get_font_size();

	UI::g_fh->get_size(font, fontsize, text, width, height, maxwidth);
	// stupid heuristic to avoid excessively long lines
	if (height < 2U * fontsize)
		UI::g_fh->get_size(font, fontsize, text, width, height, maxwidth / 2);

	width += 10 + 2 * d->textarea->scrollbar_w();
	if (width < 100)
		width = 100;
	if (width > static_cast<uint32_t>(maxwidth))
		width = maxwidth;
	height += 50;
	if (height > static_cast<uint32_t>(maxheight))
		height = maxheight;

	set_inner_size(width, height);
	set_pos(Point((outerwidth - get_w()) / 2, (outerheight - get_h()) / 2));

	d->textarea->set_size(width - 10, height - 50);

	if (type == MBoxType::kOk) {
		UI::Button * okbtn = new Button
			(this, "ok",
			 (get_inner_w() - 120) / 2, get_inner_h() - 30, 120, 20,
			 g_gr->images().get("pics/but0.png"),
			 _("OK"));
		okbtn->sigclicked.connect(boost::bind(&WLMessageBox::clicked_ok, boost::ref(*this)));
	} else if (type == MBoxType::kOkCancel) {
		UI::Button * okbtn = new Button
			(this, "ok",
			 (get_inner_w() / 2 - 120) / 2, get_inner_h() - 30, 120, 20,
			 g_gr->images().get("pics/but0.png"),
			 _("OK"));
		okbtn->sigclicked.connect(boost::bind(&WLMessageBox::clicked_ok, boost::ref(*this)));
		UI::Button * cancelbtn = new Button
			(this, "no",
			 (get_inner_w() / 2 - 120) / 2 + get_inner_w() / 2, get_inner_h() - 30,
			 120, 20,
			 g_gr->images().get("pics/but1.png"),
			 _("Cancel"));
		cancelbtn->sigclicked.connect(boost::bind(&WLMessageBox::clicked_back, boost::ref(*this)));
	}
	focus();
}

WLMessageBox::~WLMessageBox()
{
}

/**
 * Handle mouseclick.
 *
 * Clicking the right mouse button inside the window acts like pressing
 * Ok or No, depending on the message box type.
 */
bool WLMessageBox::handle_mousepress(const uint8_t btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		if (d->type == MBoxType::kOk) {
			clicked_ok();
		} else {
			clicked_back();
		}
	}
	return true;
}

bool WLMessageBox::handle_mouserelease(const uint8_t, int32_t, int32_t)
{
	return true;
}

bool WLMessageBox::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		switch (code.sym) {
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				clicked_ok();
				return true;
			case SDLK_ESCAPE:
				clicked_back();
				return true;
			default:
				break; // not handled
		}
	}
	return UI::Panel::handle_key(down, code);
}


void WLMessageBox::clicked_ok()
{
	ok();
	if (is_modal())
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}

void WLMessageBox::clicked_back()
{
	cancel();
	if (is_modal())
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}


}
