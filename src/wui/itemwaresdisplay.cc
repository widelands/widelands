/*
 * Copyright (C) 2011, 2013 by the Widelands Development Team
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

#include "wui/itemwaresdisplay.h"

#include <logic/player.h>
#include <logic/tribe.h>

#include "graphic/rendertarget.h"

namespace {

static const uint32_t IWD_HBorder = 10;
static const uint32_t IWD_VBorder = 10;
static const uint32_t IWD_DefaultItemsPerRow = 9;
static const uint32_t IWD_ItemWidth = 14;
static const uint32_t IWD_ItemHeight = 26;
static const uint32_t IWD_WorkerBaseline = -2; ///< Offset of anim center from bottom border of item rect
static const uint32_t IWD_WareBaseLine = -6;

} // anonymous namespace

/**
 * Create an ItemWaresDisplay with no items and zero capacity.
 */
ItemWaresDisplay::ItemWaresDisplay(Panel * parent, const Widelands::Player & gplayer) :
	Panel(parent, 0, 0, 0, 0),
	m_player(gplayer),
	m_capacity(0),
	m_itemsperrow(IWD_DefaultItemsPerRow)
{
	set_desired_size(2 * IWD_HBorder, 2 * IWD_VBorder);
}

/**
 * Remove all items from the internal list of items.
 */
void ItemWaresDisplay::clear()
{
	if (!m_items.empty()) {
		m_items.clear();
		update();
	}
}

/**
 * Set the nominal capacity of the display.
 *
 * This does not actually affect the number of items that can be added to the internal list,
 * but it does affect the desired size for this panel.
 */
void ItemWaresDisplay::set_capacity(uint32_t cap)
{
	if (cap != m_capacity) {
		m_capacity = cap;
		recalc_desired_size();
	}
}

/**
 * Set the items shown per row of the panel.
 */
void ItemWaresDisplay::set_itemsperrow(uint32_t nr)
{
	if (nr != m_itemsperrow) {
		m_itemsperrow = nr;
		recalc_desired_size();
	}
}

void ItemWaresDisplay::recalc_desired_size()
{
	uint32_t nrrows = (m_capacity + m_itemsperrow - 1) / m_itemsperrow;
	uint32_t rowitems = m_capacity >= m_itemsperrow ? m_itemsperrow : m_capacity;

	set_desired_size(2 * IWD_HBorder + rowitems * IWD_ItemWidth, 2 * IWD_VBorder + nrrows * IWD_ItemHeight);
}

/**
 * Add an item to the end of the internal list.
 */
void ItemWaresDisplay::add(bool worker, Widelands::Ware_Index index)
{
	Item it;
	it.worker = worker;
	it.index = index;
	m_items.push_back(it);
	update();
}

void ItemWaresDisplay::draw(RenderTarget & dst)
{
	const Widelands::Tribe_Descr & tribe(player().tribe());

	dst.fill_rect(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(0, 0, 0, 0));

	for (uint32_t idx = 0; idx < m_items.size(); ++idx) {
		const Item & it = m_items[idx];
		uint32_t row = idx / m_itemsperrow;
		uint32_t col = idx % m_itemsperrow;

		uint32_t x = IWD_HBorder / 2 + col * IWD_ItemWidth;
		uint32_t y = IWD_VBorder + row * IWD_ItemHeight;

		if (it.worker) {
			y += IWD_WorkerBaseline;
			dst.drawanim
				(Point(x + (IWD_ItemWidth / 2), y + (IWD_ItemHeight / 2)),
				 tribe.get_worker_descr(it.index)->main_animation(), 0, &player());
		} else {
			y += IWD_WareBaseLine;
			if (tribe.get_ware_descr(it.index)->icon())
				dst.blit(Point(x, y), tribe.get_ware_descr(it.index)->icon());
		}
	}
}
