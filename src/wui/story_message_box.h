/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef STORY_MESSAGE_BOX_H
#define STORY_MESSAGE_BOX_H

#include <vector>

#include "ui_basic/window.h"

struct Story_Message_Box : public UI::Window {
	Story_Message_Box
		(UI::Panel *,
		 const std::string &, const std::string &, const std::string &,
		 int32_t gposx, int32_t gposy, uint32_t w, uint32_t h);

	bool handle_mousepress(Uint8 btn, int32_t mx, int32_t my) override;

private:
	void clicked_ok();
};

#endif
