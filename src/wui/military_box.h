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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _MILITARY_BOX_H_
#define _MILITARY_BOX_H_

#include <list>

#include "logic/attackable.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"

/**
 * Military settings tab that is part of the \ref FieldActionWindow
 *
 * Used to configure the player's global military settings.
 */
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
			 const std::string & fontname = UI_FONT_NAME,
			 uint32_t            fontsize = UI_FONT_SIZE_SMALL);
		UI::Button & add_button
			(UI::Box           & parent,
			 char const *,
			 char const *,
			 void (MilitaryBox::*fn)(),
			 const std::string & tooltip_text);

		void update();

	private:
		Widelands::Player     * m_pl;
		Widelands::Map        * m_map;

		bool                    m_allowed_change;

		UI::Slider            * m_slider_retreat;
		UI::Textarea          * m_text_retreat;

};

#endif
