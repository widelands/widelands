/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef STOCK_MENU_H
#define STOCK_MENU_H

#include "ui_basic/unique_window.h"
#include "wui/waresdisplay.h"

struct Interactive_Player;

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
struct Stock_Menu : public UI::UniqueWindow {
	Stock_Menu(Interactive_Player &, UI::UniqueWindow::Registry &);

	virtual void think() override;

private:
	Interactive_Player &  m_player;
	WaresDisplay * m_all_wares;
	WaresDisplay * m_all_workers;
	WaresDisplay * m_warehouse_wares;
	WaresDisplay * m_warehouse_workers;

	void fill_total_waresdisplay(WaresDisplay * waresdisplay, Widelands::WareWorker type);
	void fill_warehouse_waresdisplay(WaresDisplay * waresdisplay, Widelands::WareWorker type);
};

#endif
