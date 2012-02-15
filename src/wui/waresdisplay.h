/*
 * Copyright (C) 2003, 2006-2011 by the Widelands Development Team
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

#ifndef WARESDISPLAY_H
#define WARESDISPLAY_H

#include "logic/warelist.h"
#include "logic/wareworker.h"
#include "logic/tribe.h"

#include "graphic/graphic.h"

#include "ui_basic/textarea.h"

#include <vector>

namespace UI {struct Textarea;}

namespace Widelands {
struct Tribe_Descr;
struct WareList;
}

/**
 * Display wares or workers together with some string (typically a number)
 * in the style of the @ref Warehouse_Window.
 *
 * For practical purposes, use one of the derived classes, e.g. @ref WaresDisplay.
 */
struct AbstractWaresDisplay : public UI::Panel {
	AbstractWaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y,
		 Widelands::Tribe_Descr const &,
		 Widelands::WareWorker type,
		 bool selectable,
		 boost::function<void(Widelands::Ware_Index, bool)> callback_function = NULL,
		 bool horizontal = true);

	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);

	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y);


	// Wares may be selected (highlighted)
	void select_ware(Widelands::Ware_Index);
	void unselect_ware(Widelands::Ware_Index);
	bool ware_selected(Widelands::Ware_Index);
	void toggle_ware(Widelands::Ware_Index ware) {
		if (ware_selected(ware))
			unselect_ware(ware);
		else
			select_ware(ware);
		if (m_callback_function)
			m_callback_function(ware, ware_selected(ware));
	}

	// Wares may be hidden
	void hide_ware(Widelands::Ware_Index);
	void unhide_ware(Widelands::Ware_Index);
	bool ware_hidden(Widelands::Ware_Index);

	Widelands::Ware_Index ware_at_point(int32_t x, int32_t y) const;
	Widelands::WareWorker get_type() const {return m_type;}

protected:
	virtual void layout();

	virtual std::string info_for_ware(Widelands::Ware_Index const) = 0;

	virtual RGBColor info_color_for_ware(Widelands::Ware_Index);

	Widelands::Tribe_Descr::WaresOrder const & icons_order() const;
	Widelands::Tribe_Descr::WaresOrderCoords const & icons_order_coords() const;
	virtual Point ware_position(Widelands::Ware_Index) const;
	virtual void draw(RenderTarget &);
	virtual void draw_ware
		(RenderTarget &,
		 Widelands::Ware_Index);

private:
	typedef std::vector<Widelands::WareList const *> vector_type;
	typedef std::vector<bool> selection_type;

	Widelands::Tribe_Descr const & m_tribe;
	Widelands::WareWorker m_type;
	UI::Textarea        m_curware;
	selection_type      m_selected;
	selection_type      m_hidden;
	bool                m_selectable;
	bool                m_horizontal;
	boost::function<void(Widelands::Ware_Index, bool)> m_callback_function;
};

/*
struct WaresDisplay
------------------
Panel that displays the contents of a WareList.
*/
struct WaresDisplay : public AbstractWaresDisplay {
	WaresDisplay
		(UI::Panel * const parent,
		 int32_t const x, int32_t const y,
		 Widelands::Tribe_Descr const &,
		 Widelands::WareWorker type,
		 bool selectable);

	virtual ~WaresDisplay();

	void add_warelist(Widelands::WareList const &);
	void remove_all_warelists();

protected:
	virtual std::string info_for_ware(Widelands::Ware_Index);

private:
	typedef std::vector<Widelands::WareList const *> vector_type;
	vector_type         m_warelists;
};

#endif
