/*
 * Copyright (C) 2003, 2006-2009 by the Widelands Development Team
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

#include "logic/editor_game_base.h"
#include "graphic/font_handler.h"
#include "i18n.h"
#include "logic/player.h"
#include "graphic/rendertarget.h"
#include "logic/tribe.h"
#include "logic/worker.h"

#include <cstdio>
#include <boost/lexical_cast.hpp>

AbstractWaresDisplay::AbstractWaresDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y,
	 Widelands::Tribe_Descr const & tribe,
	 wdType type,
	 bool selectable)
	:
	// Size is set when add_warelist is called, as it depends on the m_type.
	UI::Panel(parent, x, y, 0, 0),
	m_tribe (tribe),

	m_curware
		(this,
		 0, get_inner_h() - 25, get_inner_w(), 20,
		 _("Stock"), UI::Align_Center),
	m_type (type),
	m_selected(m_type == WORKER ? m_tribe.get_nrworkers()
	                            : m_tribe.get_nrwares(), false),
	m_hidden  (m_type == WORKER ? m_tribe.get_nrworkers()
	                            : m_tribe.get_nrwares(), false),
	m_selectable(selectable)				    
{
	// Find out geometry from icons_order 
	unsigned int columns = icons_order().size();	
	unsigned int rows = 0;
	for (unsigned int i = 0; i < icons_order().size(); i++)
		if (icons_order()[i].size() > rows) 
			rows = icons_order()[i].size();

	// 25 is height of m_curware text
	set_desired_size(columns * (WARE_MENU_PIC_WIDTH  +     3) + 1,
	                 rows    * (WARE_MENU_PIC_HEIGHT + 8 + 3) + 1 + 25);
}


bool AbstractWaresDisplay::handle_mousemove
	(Uint8, int32_t const x, int32_t const y, int32_t, int32_t)
{
	Widelands::Ware_Index const index = ware_at_point(x, y);

	m_curware.set_text
		(index ?
		 (m_type == WORKER ?
		  m_tribe.get_worker_descr(index)->descname()
		  :
		  m_tribe.get_ware_descr  (index)->descname())
		 .c_str()
		 :
		 "");
	return true;
}

bool AbstractWaresDisplay::handle_mousepress
	(Uint8 btn, int32_t const x, int32_t const y)
{
	if (btn == SDL_BUTTON_LEFT) {
		Widelands::Ware_Index ware = ware_at_point(x, y);
		if (!ware)
			return false;

		if (m_selectable) {
			toggle_ware(ware);
		}
		return true;
	}

	return UI::Panel::handle_mousepress(btn, x, y);
}

/**
 * Returns the index of the ware under the given coordinates, or
 * WareIndex::Null() if the given point is outside the range.
 */
Widelands::Ware_Index AbstractWaresDisplay::ware_at_point(int32_t x, int32_t y) const
{
	if (x < 0 || y < 0)
		return Widelands::Ware_Index::Null();


	unsigned int i = x / (WARE_MENU_PIC_WIDTH + 4);
	unsigned int j = y / (WARE_MENU_PIC_HEIGHT + 8 + 3);
	if (i < icons_order().size() && j < icons_order()[i].size()) {
		Widelands::Ware_Index ware = icons_order()[i][j];
		if (not m_hidden[ware]) {
			return ware;
		}
	}

	return Widelands::Ware_Index::Null();
}


void AbstractWaresDisplay::layout()
{
	m_curware.set_pos(Point(0, get_inner_h() - 25));
	m_curware.set_size(get_inner_w(), 20);
}

void WaresDisplay::remove_all_warelists() {
	m_warelists.clear();
}


void AbstractWaresDisplay::draw(RenderTarget & dst)
{
	Widelands::Ware_Index number = 
		m_type == WORKER ? 
		m_tribe.get_nrworkers() :
		m_tribe.get_nrwares();

	uint8_t totid = 0;
	for
		(Widelands::Ware_Index id = Widelands::Ware_Index::First();
		 id < number;
		 ++id, ++totid)
	{
		if (m_hidden[id]) continue;

		draw_ware(dst, id);
	}
}

Widelands::Tribe_Descr::WaresOrder const & AbstractWaresDisplay::icons_order() const
{
	switch(m_type) {
		case WARE:
			return m_tribe.wares_order();
			break;
		case WORKER:
			return m_tribe.workers_order();
			break;
	}
}

Widelands::Tribe_Descr::WaresOrderCoords const & AbstractWaresDisplay::icons_order_coords() const
{
	switch(m_type) {
		case WARE:
			return m_tribe.wares_order_coords();
			break;
		case WORKER:
			return m_tribe.workers_order_coords();
			break;
	}
}


Point AbstractWaresDisplay::ware_position(Widelands::Ware_Index const id) const
{
	Point p(2,2);
	p.x += icons_order_coords()[id].first  * (WARE_MENU_PIC_WIDTH + 3);
	p.y += icons_order_coords()[id].second * (WARE_MENU_PIC_HEIGHT + 3 + 8);
	return p;
}

/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void AbstractWaresDisplay::draw_ware
	(RenderTarget        &       dst,
	 Widelands::Ware_Index const id)
{
	Point p = ware_position(id);

	//  draw a background
	const PictureID picid =
		g_gr->get_picture(PicMod_Game,
			ware_selected(id) ?  "pics/ware_list_bg_selected.png"
			                  :  "pics/ware_list_bg.png"
		);
	uint32_t w, h;
	g_gr->get_picture_size(picid, w, h);

	dst.blit(p, picid);

	const Point pos = p + Point((w - WARE_MENU_PIC_WIDTH) / 2, 1);
	// Draw it
	dst.blit
		(pos,
		 m_type == WORKER ?
		 m_tribe.get_worker_descr(id)->icon()
		 :
		 m_tribe.get_ware_descr  (id)->icon());
	dst.fill_rect
		(Rect(pos + Point(0, WARE_MENU_PIC_HEIGHT), WARE_MENU_PIC_WIDTH, 8),
		 RGBColor(0, 0, 0));

	UI::g_fh->draw_string
		(dst,
		 UI_FONT_ULTRASMALL,
		 UI_FONT_SMALL_CLR,
		 p + Point(WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT - 4),
		 info_for_ware(id),
		 UI::Align_Right);
}

// Wares highlighting/selecting
void AbstractWaresDisplay::select_ware(Widelands::Ware_Index ware) {
	m_selected[ware] = true;
}
void AbstractWaresDisplay::unselect_ware(Widelands::Ware_Index ware) {
	m_selected[ware] = false;
}
bool AbstractWaresDisplay::ware_selected(Widelands::Ware_Index ware) {
	return	m_selected[ware];
}

// Wares hiding
void AbstractWaresDisplay::hide_ware(Widelands::Ware_Index ware) {
	m_hidden[ware] = true;
}
void AbstractWaresDisplay::unhide_ware(Widelands::Ware_Index ware) {
	m_hidden[ware] = false;
}
bool AbstractWaresDisplay::ware_hidden(Widelands::Ware_Index ware) {
	return	m_hidden[ware];
}

WaresDisplay::WaresDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y,
	 Widelands::Tribe_Descr const & tribe,
	 wdType type,
	 bool selectable) :
	 AbstractWaresDisplay(parent, x, y, tribe, type, selectable)
{}

WaresDisplay::~WaresDisplay()
{
	remove_all_warelists();
}

std::string WaresDisplay::info_for_ware(Widelands::Ware_Index ware) {
	uint32_t totalstock = 0;
	for
		(Widelands::Ware_Index i = Widelands::Ware_Index::First();
		 i.value() < m_warelists.size();
		 ++i)
		totalstock += m_warelists[i]->stock(ware);
	return boost::lexical_cast<std::string>(totalstock);
}

/*
===============
add a ware list to be displayed in this WaresDisplay
===============
*/
void WaresDisplay::add_warelist
	(Widelands::WareList const & wares)
{
	//  If you register something twice, it is counted twice. Not my problem.
	m_warelists.push_back(&wares);
}
