/*
 * Copyright (C) 2003, 2006-2008 by the Widelands Development Team
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

#include "waresdisplay.h"

#include "editor_game_base.h"
#include "font_handler.h"
#include "graphic.h"
#include "i18n.h"
#include "player.h"
#include "rendertarget.h"
#include "tribe.h"
#include "worker.h"

#include <stdio.h>

WaresDisplay::WaresDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y,
	 Widelands::Tribe_Descr const & tribe)
:
UI::Panel(parent, x, y, Width, 0),
m_tribe (tribe),

m_curware
	(this, 0, get_inner_h() - 25, get_inner_w(), 20, _("Stock"), Align_Center)

{set_size(Width, 100);}


WaresDisplay::~WaresDisplay()
{
	remove_all_warelists();
}


bool WaresDisplay::handle_mousemove(const Uint8, int32_t x, int32_t y, int32_t, int32_t) {
	assert(m_warelists.size());

	Widelands::Ware_Index const index =
		x < 0 or y < 0 ?
		Widelands::Ware_Index::Null()
		:
		Widelands::Ware_Index
			(static_cast<Widelands::Ware_Index::value_t>
			 	(y / (WARE_MENU_PIC_HEIGHT + 8 + 3) * WaresPerRow
			 	 +
			 	 x / (WARE_MENU_PIC_WIDTH + 4)));

	m_curware.set_text
		(index < m_warelists[0]->get_nrwareids() ?
		 (m_type == WORKER ?
		  m_tribe.get_worker_descr(index)->descname()
		  :
		  m_tribe.get_ware_descr  (index)->descname())
		 .c_str()
		 :
		 "");
	return true;
}

/*
===============
WaresDisplay::set_wares

add a ware list to be displayed in this WaresDisplay
===============
*/
void WaresDisplay::add_warelist
	(Widelands::WareList const * const wares, wdType const type)
{
	//  If you register something twice, it is counted twice. Not my problem.
	m_warelists.push_back(wares);

	int32_t rows, height;

	rows = (wares->get_nrwareids().value() + WaresPerRow - 1) / WaresPerRow;
	height = rows * (WARE_MENU_PIC_HEIGHT + 8 + 3) + 1;

	set_size(get_inner_w(), height+30);
	m_curware.set_pos(Point(0, get_inner_h() - 25));
	m_curware.set_size(get_inner_w(), 20);

	m_type = type;


	update(0, 0, get_w(), get_h());
}


void WaresDisplay::remove_all_warelists() {
	m_warelists.clear();
}

/*
===============
WaresDisplay::draw

Draw the wares.
===============
*/
void WaresDisplay::draw(RenderTarget* dst)
{
	Point p(2, 2);

	Widelands::Ware_Index number = m_tribe.get_nrwares();
	bool is_worker = false;

	if (m_type == WORKER) {
		number = m_tribe.get_nrworkers();
		is_worker = true;
	}
	uint8_t totid = 0;
	for
		(Widelands::Ware_Index id = Widelands::Ware_Index::First();
		 id < number;
		 ++id, ++totid)
	{
		uint32_t totalstock = 0;
		for
			(Widelands::Ware_Index i = Widelands::Ware_Index::First();
			 i.value() < m_warelists.size();
			 ++i)
			totalstock += m_warelists[i]->stock(id);

		draw_ware(*dst, p, id, totalstock, is_worker);

		if ((totid + 1) % WaresPerRow) {p.x += WARE_MENU_PIC_WIDTH + 3;}
		else {p.x = 2; p.y += WARE_MENU_PIC_HEIGHT + 8 + 3;}
	}
}


/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void WaresDisplay::draw_ware
	(RenderTarget        &       dst,
	 Point                 const p,
	 Widelands::Ware_Index const id,
	 uint32_t              const stock,
	 bool                  const is_worker)
{
	//  draw a background
	const uint32_t picid = g_gr->get_picture(PicMod_Game, "pics/ware_list_bg.png");
	uint32_t w, h;
	g_gr->get_picture_size(picid, w, h);

	dst.blit(p, picid);

	const Point pos = p + Point((w - WARE_MENU_PIC_WIDTH) / 2, 1);
	// Draw it
	dst.blit
		(pos,
		 is_worker ?
		 m_tribe.get_worker_descr(id)->get_menu_pic()
		 :
		 m_tribe.get_ware_descr(id)->get_icon());
	dst.fill_rect
		(Rect(pos + Point(0, WARE_MENU_PIC_HEIGHT), WARE_MENU_PIC_WIDTH, 8),
		 RGBColor(0, 0, 0));

	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%i", stock);

	g_fh->draw_string
		(dst,
		 UI_FONT_ULTRASMALL,
		 UI_FONT_SMALL_CLR,
		 p + Point(WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT - 4),
		 buffer,
		 Align_Right);
}
