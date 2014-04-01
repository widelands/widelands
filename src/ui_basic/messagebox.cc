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

#include "constants.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"
#include "wlapplication.h"

namespace UI {


struct WLMessageBoxImpl {
	Multiline_Textarea * textarea;
	WLMessageBox::MB_Type type;
};


WLMessageBox::WLMessageBox
	(Panel * const parent,
	 const std::string & caption,
	 const std::string & text,
	 const MB_Type type,
	 Align align)
	:
	Window(parent, "message_box", 0, 0, 20, 20, caption.c_str()),
	d(new WLMessageBoxImpl)
{
	d->type = type;
	d->textarea = new Multiline_Textarea
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

	if (type == OK) {
		UI::Button * okbtn = new Button
			(this, "ok",
			 (get_inner_w() - 120) / 2, get_inner_h() - 30, 120, 20,
			 g_gr->images().get("pics/but0.png"),
			 _("OK"));
		okbtn->sigclicked.connect(boost::bind(&WLMessageBox::pressedOk, boost::ref(*this)));
	} else if (type == YESNO) {
		UI::Button * yesbtn = new Button
			(this, "yes",
			 (get_inner_w() / 2 - 120) / 2, get_inner_h() - 30, 120, 20,
			 g_gr->images().get("pics/but0.png"),
			 _("Yes"));
		yesbtn->sigclicked.connect(boost::bind(&WLMessageBox::pressedYes, boost::ref(*this)));
		UI::Button * nobtn = new Button
			(this, "no",
			 (get_inner_w() / 2 - 120) / 2 + get_inner_w() / 2, get_inner_h() - 30,
			 120, 20,
			 g_gr->images().get("pics/but1.png"),
			 _("No"));
		nobtn->sigclicked.connect(boost::bind(&WLMessageBox::pressedNo, boost::ref(*this)));
	}
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
bool WLMessageBox::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		if (d->type == OK)
			pressedOk();
		else
			pressedNo();
	}
	return true;
}

bool WLMessageBox::handle_mouserelease(const Uint8, int32_t, int32_t)
{
	return true;
}

bool WLMessageBox::handle_key(bool down, SDL_keysym code)
{
	if (!down) {
		return false;
	}

	switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			pressedYes();
			pressedOk();
			return true;
		case SDLK_ESCAPE:
			pressedNo();
			pressedOk();
			return true;
		default:
			return false;
	}

	return UI::Panel::handle_key(down, code);
}

void WLMessageBox::pressedOk()
{
	ok();
	if (is_modal())
		end_modal(0);
}

void WLMessageBox::pressedYes()
{
	yes();
	if (is_modal())
		end_modal(1);
}

void WLMessageBox::pressedNo()
{
	no();
	if (is_modal())
		end_modal(0);
}


}
