/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__UI_MODAL_MESSAGE_BOX_H
#define __S__UI_MODAL_MESSAGE_BOX_H

#include "ui_window.h"

namespace UI {
/**
 * Shows a modal messagebox, user must click something
*/
struct Modal_Message_Box : public Window {
	enum MB_Type {
         OK,
         YESNO
	};
	Modal_Message_Box
		(Panel * const parent,
		 const std::string & caption,
		 const std::string & text,
		 const MB_Type);
      ~Modal_Message_Box();

	bool handle_mousepress  (const Uint8 btn, int mx, int my);
	bool handle_mouserelease(const Uint8 btn, int mx, int my);
};
};

#endif
