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

#ifndef WL_WUI_ATTACK_BOX_H
#define WL_WUI_ATTACK_BOX_H

#include <list>

#include "graphic/image_catalog.h"
#include "graphic/font_handler1.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "logic/attackable.h"
#include "logic/bob.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"

using Widelands::Bob;
using Widelands::Building;
using Widelands::Soldier;

/**
 * Provides the attack settings that are part of a \ref FieldActionWindow
 * when clicking on an enemy building.
 */
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

	private:
		uint32_t get_max_attackers();
		UI::Slider & add_slider
			(UI::Box    & parent,
			 uint32_t      width,
			 uint32_t      height,
			 uint32_t      min, uint32_t max, uint32_t initial,
			 ImageCatalog::Key image_key,
			 char const  * hint);
		UI::Textarea & add_text
			(UI::Box           & parent,
			 std::string         str,
			 uint32_t            alignment = UI::Box::AlignTop,
			 const std::string & fontname = UI::g_fh1->fontset().serif(),
			 uint32_t            fontsize = UI_FONT_SIZE_SMALL);
		UI::Button & add_button
			(UI::Box           & parent,
			 char const * picname,
			 void (AttackBox::*fn)(),
			 const std::string & tooltip_text);

		void update_attack();
		void send_less_soldiers();
		void send_more_soldiers();

	private:
		Widelands::Player     * m_pl;
		Widelands::Map        * m_map;
		Widelands::FCoords    * m_node;

		UI::Slider            * m_slider_soldiers;
		UI::Textarea          * m_text_soldiers;

		UI::Button * m_less_soldiers;
		UI::Button * m_add_soldiers;
};

#endif  // end of include guard: WL_WUI_ATTACK_BOX_H
