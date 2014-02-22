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

#include <vector>

#include <boost/signals2.hpp>

#include "logic/tribe.h"
#include "logic/warelist.h"
#include "logic/wareworker.h"
#include "ui_basic/textarea.h"

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
class AbstractWaresDisplay : public UI::Panel {
public:
	AbstractWaresDisplay
		(UI::Panel * const parent,
		 int32_t x, int32_t y,
		 const Widelands::Tribe_Descr &,
		 Widelands::WareWorker type,
		 bool selectable,
		 boost::function<void(Widelands::Ware_Index, bool)> callback_function = 0,
		 bool horizontal = false);

	bool handle_mousemove
		(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;

	// Wares may be selected (highlighted)
	void select_ware(Widelands::Ware_Index);
	void unselect_ware(Widelands::Ware_Index);
	bool ware_selected(Widelands::Ware_Index);

	// Wares may be hidden
	void hide_ware(Widelands::Ware_Index);
	void unhide_ware(Widelands::Ware_Index);
	bool ware_hidden(Widelands::Ware_Index);

	Widelands::Ware_Index ware_at_point(int32_t x, int32_t y) const;
	Widelands::WareWorker get_type() const {return m_type;}

protected:
	virtual void layout() override;

	virtual std::string info_for_ware(Widelands::Ware_Index) = 0;

	virtual RGBColor info_color_for_ware(Widelands::Ware_Index);

	const Widelands::Tribe_Descr::WaresOrder & icons_order() const;
	const Widelands::Tribe_Descr::WaresOrderCoords & icons_order_coords() const;
	virtual Point ware_position(Widelands::Ware_Index) const;
	virtual void draw(RenderTarget &) override;
	virtual void draw_ware
		(RenderTarget &,
		 Widelands::Ware_Index);

private:
	typedef std::vector<const Widelands::WareList *> vector_type;
	typedef std::vector<bool> selection_type;

	/**
	 * Update the anchored selection. When first mouse button is pressed on a
	 * ware, it is stored in @ref m_selection_anchor. Mouse moves trigger this
	 * function to select all wares in the rectangle between the anchor and the
	 * mouse position. They are temporary stored in @ref m_in_selection.
	 * Releasing the mouse button will performs the selection. This allows
	 * selection of multiple wares by dragging.
	 */
	void update_anchor_selection(int32_t x, int32_t y);

	const Widelands::Tribe_Descr & m_tribe;
	Widelands::WareWorker m_type;
	UI::Textarea        m_curware;
	selection_type      m_selected;
	selection_type      m_hidden;
	selection_type      m_in_selection;  //Wares in temporary anchored selection
	bool                m_selectable;
	bool                m_horizontal;

	/**
	 * The ware on which the mouse press has been performed.
	 * It is not selected directly, but will be on mouse release.
	 */
	Widelands::Ware_Index m_selection_anchor;
	boost::function<void(Widelands::Ware_Index, bool)> m_callback_function;
};

/*
class WaresDisplay
------------------
Panel that displays the contents of many WareLists. The ware_lists
must be valid while they are registered with this class.
*/
class WaresDisplay : public AbstractWaresDisplay {
public:
	WaresDisplay
		(UI::Panel * const parent,
		 int32_t x, int32_t y,
		 const Widelands::Tribe_Descr &,
		 Widelands::WareWorker type,
		 bool selectable);

	virtual ~WaresDisplay();

	void add_warelist(const Widelands::WareList &);
	void remove_all_warelists();

protected:
	virtual std::string info_for_ware(Widelands::Ware_Index) override;

private:
	typedef std::vector<const Widelands::WareList *> vector_type;
	vector_type         m_warelists;
	std::vector<boost::signals2::connection> connections_;
};

std::string waremap_to_richtext
		(const Widelands::Tribe_Descr & tribe,
		 const std::map<Widelands::Ware_Index, uint8_t> & map);

#endif
