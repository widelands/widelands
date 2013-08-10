/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef _SOLDIERLIST_H_
#define _SOLDIERLIST_H_

#include "ui_basic/panel.h"
#include "logic/instances.h"

namespace Widelands {
class Soldier;
class Editor_Game_Base;
class Garrison;
}

class Interactive_GameBase;

namespace UI {
struct Panel;

struct SoldierPanel : Panel {
	typedef boost::function<void (const Widelands::Soldier *)> SoldierFn;

	SoldierPanel(UI::Panel & parent, Widelands::Editor_Game_Base & egbase, Widelands::Garrison & garrison);

	Widelands::Editor_Game_Base & egbase() const {return m_egbase;}

	virtual void think();
	virtual void draw(RenderTarget &);

	void set_mouseover(const SoldierFn & fn);
	void set_click(const SoldierFn & fn);

protected:
	virtual void handle_mousein(bool inside);
	virtual bool handle_mousemove(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_mousepress(Uint8 btn, int32_t x, int32_t y);

private:
	Point calc_pos(uint32_t row, uint32_t col) const;
	const Widelands::Soldier * find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Widelands::Soldier> soldier;
		uint32_t row;
		uint32_t col;
		Point pos;

		/**
		 * Keep track of how we last rendered this soldier,
		 * so that we can update when its status changes.
		 */
		/*@{*/
		uint32_t cache_level;
		uint32_t cache_health;
		/*@}*/
	};

	Widelands::Editor_Game_Base & m_egbase;
	Widelands::Garrison & m_garrison;

	SoldierFn m_mouseover_fn;
	SoldierFn m_click_fn;

	std::vector<Icon> m_icons;

	uint32_t m_rows;
	uint32_t m_cols;

	uint32_t m_icon_width;
	uint32_t m_icon_height;

	int32_t m_last_animate_time;

	static const uint32_t MaxColumns = 6;
	static const uint32_t AnimateSpeed = 300; ///< in pixels per second
	static const uint32_t IconBorder = 2;
};

}

#endif // _SOLDIERLIST_H_
