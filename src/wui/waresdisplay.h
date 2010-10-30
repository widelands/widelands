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

#ifndef WARESDISPLAY_H
#define WARESDISPLAY_H

#include "logic/warelist.h"
#include "logic/tribe.h"

#include "ui_basic/textarea.h"

#include <vector>

namespace UI {struct Textarea;}

namespace Widelands {
struct Tribe_Descr;
struct WareList;
}

/*
class WaresDisplay
------------------
Panel that displays the contents of a WareList.
*/
struct WaresDisplay : public UI::Panel {
	enum wdType {
		WORKER,
		WARE
	};

	WaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y, Widelands::Tribe_Descr const &);
	virtual ~WaresDisplay();

	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);

	void add_warelist(Widelands::WareList const &, wdType);
	void remove_all_warelists();

	// Wares may be selected (highlighted)
	void select_ware(Widelands::Ware_Index);
	void unselect_ware(Widelands::Ware_Index);
	bool ware_selected(Widelands::Ware_Index);
	void toggle_ware(Widelands::Ware_Index ware) {
		if (ware_selected(ware))
			unselect_ware(ware);
		else
			select_ware(ware);
	}

	// Wares may be hidden
	void hide_ware(Widelands::Ware_Index);
	void unhide_ware(Widelands::Ware_Index);
	bool ware_hidden(Widelands::Ware_Index);

	Widelands::Ware_Index ware_at_point(int32_t x, int32_t y) const;
	wdType get_type() const {return m_type;}

protected:
	virtual void layout();
	virtual void update_desired_size();

	Widelands::Tribe_Descr::WaresOrder const & icons_order() const;
	Widelands::Tribe_Descr::WaresOrderCoords const & icons_order_coords() const;
	virtual Point ware_position(Widelands::Ware_Index const) const;
	virtual void draw(RenderTarget &);
	virtual void draw_ware
		(RenderTarget &,
		 Widelands::Ware_Index,
		 uint32_t stock);

private:
	typedef std::vector<Widelands::WareList const *> vector_type;
	typedef std::vector<bool> selection_type;

	Widelands::Tribe_Descr const & m_tribe;
	UI::Textarea        m_curware;
	wdType              m_type;
	vector_type         m_warelists;
	selection_type      m_selected;
	selection_type      m_hidden;
};

#endif
