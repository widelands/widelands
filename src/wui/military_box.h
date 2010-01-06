/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef _MILITARY_BOX_H_
#define _MILITARY_BOX_H_

#include "editor/ui_menus/editor_event_menu_new_event.h"
#include "graphic/picture_id.h"
#include "logic/attackable.h"
#include "logic/player.h"

#include "ui_basic/box.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include <list>

struct MilitaryBox : public UI::Box {
	MilitaryBox
		(UI::Panel              * parent,
		 Widelands::Player      * player,
		 uint32_t  const              x,
		 uint32_t  const              y);
	~MilitaryBox();

	void init();
	bool allowed_change() const {return m_allowed_change;}

	private:
		UI::Slider & add_slider
			(UI::Box    & parent,
			 uint32_t      width,
			 uint32_t      height,
			 uint32_t      min, uint32_t max, uint32_t initial,
			 char const  * picname,
			 char const  * hint);
		UI::Textarea & add_text
			(UI::Box           & parent,
			 std::string         str,
			 uint32_t            alignment = UI::Box::AlignTop,
			 std::string const & fontname = UI_FONT_NAME,
			 uint32_t            fontsize = UI_FONT_SIZE_SMALL);
		UI::Callback_Button<MilitaryBox> & add_button
			(UI::Box           & parent,
			 char const * picname,
			 void (MilitaryBox::*fn)(),
			 std::string const & tooltip_text);

		void update();

	private:
		Widelands::Player     * m_pl;
		Widelands::Map        * m_map;

		bool                    m_allowed_change;

		UI::Slider            * m_slider_retreat;
		UI::Textarea          * m_text_retreat;

};

#endif
