/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef _ATTACK_BOX_H_
#define _ATTACK_BOX_H_

#include "editor/ui_menus/editor_event_menu_new_event.h"
#include "graphic/picture_id.h"
#include "logic/attackable.h"
#include "logic/bob.h"
#include "logic/soldier.h"
#include "logic/player.h"

#include "ui_basic/box.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include <list>

namespace UI {

using Widelands::Bob;
using Widelands::Building;
using Widelands::Soldier;

struct AttackBox : public UI::Box {
	AttackBox
		(UI::Panel              * parent,
		 Widelands::Player      * player,
		 Widelands::FCoords     * target,
		 uint32_t  const              x,
		 uint32_t  const              y);
	~AttackBox();

	void init();

	uint32_t soldiers() const;
	uint32_t retreat() const;

	private:
		uint32_t get_max_attackers();
		Slider * add_slider
			(Box         * parent,
			 uint32_t      width,
			 uint32_t      height,
			 uint32_t      min, uint32_t max, uint32_t initial,
			 char const  * picname,
			 char const  * hint);
		Textarea * add_text
			(Box               * parent,
			 std::string         str,
			 uint32_t            alignment = UI::Box::AlignTop,
			 std::string const & fontname = UI_FONT_NAME,
			 uint32_t            fontsize = UI_FONT_SIZE_SMALL);
		void add_button
			(Box *,
			 char const * picname,
			 void (AttackBox::*fn)(),
			 std::string const & tooltip_text);

		void update_attack();
		void send_less_soldiers();
		void send_more_soldiers();

	private:
		Widelands::Player     * m_pl;
		Widelands::Map        * m_map;
		Widelands::FCoords    * m_node;

		UI::Slider            * m_slider_retreat;
		UI::Slider            * m_slider_soldiers;
		UI::Textarea          * m_text_soldiers;
		UI::Textarea          * m_text_retreat;
};

}
#endif
