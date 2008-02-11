/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef STOCK_MENU_H
#define STOCK_MENU_H

#include "waresdisplay.h"

#include "ui_button.h"
#include "ui_unique_window.h"

class WaresDisplay;
class Interactive_Player;
namespace Widelands {struct Economy;};

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
struct Stock_Menu : public UI::UniqueWindow {
	Stock_Menu(Interactive_Player &, UI::UniqueWindow::Registry &);

	virtual void think();

private:
	Interactive_Player &  m_player;
	WaresDisplay          waresdisplay;
	UI::Button<Stock_Menu> help;
	UI::Button<Stock_Menu> switchpage;
	enum {Wares, Workers} current_page;
	void fill_waredisplay_with_wares  ();
	void fill_waredisplay_with_workers();

	/** Returns the horizontal/vertical spacing between buttons. */
	uint32_t hspacing() const {return 5;};
	uint32_t vspacing() const {return 5;};

	/** Returns the horizontal/vertical margin between edge and buttons. */
	uint32_t hmargin() const {return 2 * hspacing();}
	uint32_t vmargin() const {return 2 * vspacing();}

	/** Returns the width of a button in a row with nr_buttons buttons. */
	uint32_t buttonw(const uint32_t nr_buttons) const {
		return
			(waresdisplay.get_w() - (nr_buttons + 3) * hspacing()) / nr_buttons;
	}

	/**
	 * Returns the x coordinate of the (left edge of) button number nr in a row
	 * with nr_buttons buttons.
	 */
	uint32_t posx(const uint32_t nr, const uint32_t nr_buttons) const
	{return hmargin() + nr * (buttonw(nr_buttons) + hspacing());}

	void clicked_help       ();
	void clicked_switch_page();
};

#endif
