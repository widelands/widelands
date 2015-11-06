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

#ifndef WL_WUI_WARESDISPLAY_H
#define WL_WUI_WARESDISPLAY_H

#include <vector>

#include <boost/signals2.hpp>

#include "logic/tribes/tribe_descr.h"
#include "logic/warelist.h"
#include "logic/wareworker.h"
#include "ui_basic/textarea.h"

namespace UI {struct Textarea;}

namespace Widelands {
class TribeDescr;
struct WareList;
}

/**
 * Display wares or workers together with some string (typically a number)
 * in the style of the @ref WarehouseWindow.
 *
 * For practical purposes, use one of the derived classes, e.g. @ref WaresDisplay.
 */
class AbstractWaresDisplay : public UI::Panel {
public:
	AbstractWaresDisplay
		(UI::Panel * const parent,
		 int32_t x, int32_t y,
		 const Widelands::TribeDescr &,
		 Widelands::WareWorker type,
		 bool selectable,
		 boost::function<void(Widelands::WareIndex, bool)> callback_function = 0,
		 bool horizontal = false);

	bool handle_mousemove
		(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;

	// Wares may be selected (highlighted)
	void select_ware(Widelands::WareIndex);
	void unselect_ware(Widelands::WareIndex);
	bool ware_selected(Widelands::WareIndex);

	// Wares may be hidden
	void hide_ware(Widelands::WareIndex);
	void unhide_ware(Widelands::WareIndex);
	bool ware_hidden(Widelands::WareIndex);

	Widelands::WareIndex ware_at_point(int32_t x, int32_t y) const;
	Widelands::WareWorker get_type() const {return m_type;}

protected:
	void layout() override;

	virtual std::string info_for_ware(Widelands::WareIndex) = 0;

	virtual RGBColor info_color_for_ware(Widelands::WareIndex);

	const Widelands::TribeDescr::WaresOrder & icons_order() const;
	const Widelands::TribeDescr::WaresOrderCoords & icons_order_coords() const;
	virtual Point ware_position(Widelands::WareIndex) const;
	void draw(RenderTarget &) override;
	virtual void draw_ware
		(RenderTarget &,
		 Widelands::WareIndex);

private:
	using WareListVector = std::vector<const Widelands::WareList *>;
	using WareListSelectionType = std::map<const Widelands::WareIndex, bool>;

	/**
	 * Update the anchored selection. When first mouse button is pressed on a
	 * ware, it is stored in @ref m_selection_anchor. Mouse moves trigger this
	 * function to select all wares in the rectangle between the anchor and the
	 * mouse position. They are temporary stored in @ref m_in_selection.
	 * Releasing the mouse button will performs the selection. This allows
	 * selection of multiple wares by dragging.
	 */
	void update_anchor_selection(int32_t x, int32_t y);

	const Widelands::TribeDescr & m_tribe;
	Widelands::WareWorker m_type;
	const std::set<Widelands::WareIndex> m_indices;
	UI::Textarea        m_curware;
	WareListSelectionType      m_selected;
	WareListSelectionType      m_hidden;
	WareListSelectionType      m_in_selection;  //Wares in temporary anchored selection
	bool                m_selectable;
	bool                m_horizontal;

	/**
	 * The ware on which the mouse press has been performed.
	 * It is not selected directly, but will be on mouse release.
	 */
	Widelands::WareIndex m_selection_anchor;
	boost::function<void(Widelands::WareIndex, bool)> m_callback_function;
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
		 const Widelands::TribeDescr &,
		 Widelands::WareWorker type,
		 bool selectable);

	virtual ~WaresDisplay();

	void add_warelist(const Widelands::WareList &);
	void remove_all_warelists();

protected:
	std::string info_for_ware(Widelands::WareIndex) override;

private:
	using WareListVector = std::vector<const Widelands::WareList *>;
	WareListVector         m_warelists;
	std::vector<boost::signals2::connection> connections_;
};

std::string waremap_to_richtext
		(const Widelands::TribeDescr & tribe,
		 const std::map<Widelands::WareIndex, uint8_t> & map);

#endif  // end of include guard: WL_WUI_WARESDISPLAY_H
