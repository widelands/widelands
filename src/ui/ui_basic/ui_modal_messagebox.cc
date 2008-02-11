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

#include "ui_modal_messagebox.h"

#include "constants.h"
#include "i18n.h"
#include "ui_window.h"
#include "ui_multilinetextarea.h"
#include "ui_button.h"
#include "graphic.h"
#include "wlapplication.h"

namespace UI {
Modal_Message_Box::Modal_Message_Box
	(Panel * const parent,
	 const std::string & caption,
	 const std::string & text,
	 const MB_Type type)
	:
	Window(parent, 0, 0, 20, 20, caption.c_str())
{

	set_inner_size(320, 160);
	set_pos
		(Point
		 ((parent->get_inner_w() - 320) / 2, (parent->get_inner_h() - 100) / 2));

	new Multiline_Textarea
		(this,
		 5, 5, get_inner_w() - 10, get_inner_h() - 70,
		 text.c_str(), Align_Center);

	if (type == OK) {
		new IDButton<Modal_Message_Box, int32_t>
			(this,
			 (get_inner_w() - 60) / 2, get_inner_h() - 30, 60, 20,
			 0,
			 &Modal_Message_Box::end_modal, this, 0,
			 _("OK"));
	} else if (type == YESNO) {
		new IDButton<Modal_Message_Box, int32_t>
			(this,
			 (get_inner_w() / 2 - 60) / 2, get_inner_h() - 30, 60, 20,
			 0,
			 &Modal_Message_Box::end_modal, this, 1,
			 _("Yes"));
		new IDButton<Modal_Message_Box, int32_t>
			(this,
			 (get_inner_w() / 2 - 60) / 2 + get_inner_w() / 2, get_inner_h() - 30,
			 60, 20,
			 1,
			 &Modal_Message_Box::end_modal, this, 0,
			 _("No"));
	}
}

Modal_Message_Box::~Modal_Message_Box() {}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we call end_modal() with NO (=0)
 * We are not draggable.
 */
bool Modal_Message_Box::handle_mousepress(const Uint8 btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_RIGHT) {play_click(); end_modal(0); return true;}
	return false;
}
bool Modal_Message_Box::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}
};
