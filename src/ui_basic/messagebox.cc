/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "messagebox.h"

#include "constants.h"
#include "i18n.h"
#include "window.h"
#include "multilinetextarea.h"
#include "button.h"
#include "font_handler.h"
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
	 const MB_Type type)
	:
	Window(parent, 0, 0, 20, 20, caption.c_str()),
	d(new WLMessageBoxImpl)
{
	d->type = type;
	d->textarea = new Multiline_Textarea
		(this,
		 5, 5, 30, 30,
		 text.c_str(), Align_Center);

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
	height += 50;
	if (height > static_cast<uint32_t>(maxheight))
		height = maxheight;

	set_inner_size(width, height);
	set_pos(Point((outerwidth - get_w()) / 2, (outerheight - get_h()) / 2));

	d->textarea->set_size(width - 10, height - 50);

	if (type == OK) {
		new Callback_Button<WLMessageBox>
			(this,
			 (get_inner_w() - 60) / 2, get_inner_h() - 30, 60, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
			 &WLMessageBox::pressedOk, *this,
			 _("OK"));
	} else if (type == YESNO) {
		new Callback_Button<WLMessageBox>
			(this,
			 (get_inner_w() / 2 - 60) / 2, get_inner_h() - 30, 60, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
			 &WLMessageBox::pressedYes, *this,
			 _("Yes"));
		new Callback_Button<WLMessageBox>
			(this,
			 (get_inner_w() / 2 - 60) / 2 + get_inner_w() / 2, get_inner_h() - 30,
			 60, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 &WLMessageBox::pressedNo, *this,
			 _("No"));
	}
}

WLMessageBox::~WLMessageBox()
{
}


void WLMessageBox::set_align(Align align)
{
	d->textarea->set_align(align);
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

void WLMessageBox::pressedOk()
{
	ok.call();
	if (is_modal())
		end_modal(0);
}

void WLMessageBox::pressedYes()
{
	yes.call();
	if (is_modal())
		end_modal(1);
}

void WLMessageBox::pressedNo()
{
	no.call();
	if (is_modal())
		end_modal(0);
}


}
