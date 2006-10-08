/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__STOCK_MENU_H
#define __S__STOCK_MENU_H

#include "ui_button.h"
#include "ui_unique_window.h"
#include "waresdisplay.h"

class WaresDisplay;
class Interactive_Player;
class Economy;

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
class Stock_Menu : public UIUniqueWindow {
public:
	Stock_Menu(Interactive_Player &, UIUniqueWindowRegistry &);

	virtual void think();

private:
	Interactive_Player &  m_player;
	WaresDisplay          waresdisplay;
	UIButton              help;
	UIButton              switchpage;
	enum {Wares, Workers} current_page;
   void fill_waredisplay_with_wares(void);
   void fill_waredisplay_with_workers(void);

	/** Returns the horizontal/vertical spacing between buttons. */
	uint hspacing() const {return 5;};
	uint vspacing() const {return 5;};

	/** Returns the horizontal/vertical margin between edge and buttons. */
	uint hmargin() const {return 2 * hspacing();}
	uint vmargin() const {return 2 * vspacing();}

	/** Returns the width of a button in a row with nr_buttons buttons. */
	uint buttonw(const uint nr_buttons) const {
		return
			(waresdisplay.get_w() - (nr_buttons + 3) * hspacing()) / nr_buttons;
	}

	/**
	 * Returns the x coordinate of the (left edge of) button number nr in a row
	 * with nr_buttons buttons.
	 */
	uint posx(const uint nr, const uint nr_buttons) const
	{return hmargin() + nr * (buttonw(nr_buttons) + hspacing());}

	void clicked_help       (int);
	void clicked_switch_page(int);
};

#endif
