/*
 * Copyright (C) 2003-2022 by the Widelands Development Team
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

#include <functional>
#include <memory>

#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "ui_basic/textarea.h"

using WaresOrderCoords = std::map<Widelands::DescriptionIndex, Widelands::Coords>;

/**
 * Display wares or workers together with some string (typically a number)
 * in the style of the @ref WarehouseWindow.
 *
 * For practical purposes, use one of the derived classes, e.g. @ref WaresDisplay.
 */
class AbstractWaresDisplay : public UI::Panel {
public:
	AbstractWaresDisplay(
	   UI::Panel* const parent,
	   int32_t x,
	   int32_t y,
	   const Widelands::TribeDescr&,
	   Widelands::WareWorker type,
	   bool selectable,
	   CLANG_DIAG_OFF("-Wunknown-pragmas") CLANG_DIAG_OFF("-Wzero-as-null-pointer-constant")
	      std::function<void(Widelands::DescriptionIndex, bool)> callback_function = 0,
	   CLANG_DIAG_ON("-Wzero-as-null-pointer-constant")
	      CLANG_DIAG_ON("-Wunknown-pragmas") bool horizontal = false,
	   int32_t hgap = 3,
	   int32_t vgap = 4);

	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	void handle_mousein(bool inside) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;

	// Wares may be selected (highlighted)
	void select_ware(Widelands::DescriptionIndex);
	void unselect_ware(Widelands::DescriptionIndex);
	bool ware_selected(Widelands::DescriptionIndex);

	// Wares may be hidden
	void hide_ware(Widelands::DescriptionIndex);
	bool is_ware_hidden(Widelands::DescriptionIndex) const;

	Widelands::DescriptionIndex ware_at_point(int32_t x, int32_t y) const;
	Widelands::WareWorker get_type() const {
		return type_;
	}

	int32_t get_hgap() {
		return hgap_;
	}
	int32_t get_vgap() {
		return vgap_;
	}
	void set_hgap(int32_t, bool = true);
	void set_vgap(int32_t, bool = true);

	Widelands::Extent get_extent() const;

	const WaresOrderCoords& icons_order_coords() const;
	Widelands::DescriptionIndex ware_at_coords(int16_t x, int16_t y) const;
	uint16_t column_length(int16_t) const;

	void set_min_free_vertical_space(int32_t s) {
		min_free_vertical_space_ = s;
	}
	int32_t get_min_free_vertical_space() const {
		return min_free_vertical_space_;
	}

	static inline int32_t calc_hgap(int32_t columns, int32_t total_w, int32_t min = 3) {
		return std::max(min, (total_w - columns * kWareMenuPicWidth) / (columns - 1));
	}

protected:
	void layout() override;

	virtual std::string info_for_ware(Widelands::DescriptionIndex) = 0;

	virtual RGBColor info_color_for_ware(Widelands::DescriptionIndex);

	const Widelands::TribeDescr::WaresOrder& icons_order() const;
	virtual Vector2i ware_position(Widelands::DescriptionIndex) const;
	void draw(RenderTarget&) override;
	virtual void draw_ware(RenderTarget&, Widelands::DescriptionIndex);
	virtual RGBAColor draw_ware_background_overlay(Widelands::DescriptionIndex) {
		return RGBAColor(0, 0, 0, 0);
	}

private:
	using WareListVector = std::vector<const Widelands::WareList*>;
	using WareListSelectionType = std::map<const Widelands::DescriptionIndex, bool>;

	/**
	 * Update the anchored selection. When first mouse button is pressed on a
	 * ware, it is stored in @ref selection_anchor_. Mouse moves trigger this
	 * function to select all wares in the rectangle between the anchor and the
	 * mouse position. They are temporary stored in @ref in_selection_.
	 * Releasing the mouse button will performs the selection. This allows
	 * selection of multiple wares by dragging.
	 */
	void update_anchor_selection(int32_t x, int32_t y);
	void finalize_anchor_selection();

	const Widelands::TribeDescr& tribe_;
	Widelands::WareWorker type_;
	const std::set<Widelands::DescriptionIndex> indices_;
	UI::Textarea curware_;
	WareListSelectionType selected_;
	WareListSelectionType hidden_;
	WareListSelectionType in_selection_;  // Wares in temporary anchored selection
	bool selectable_;
	bool horizontal_;
	int32_t hgap_;
	int32_t vgap_;

	WaresOrderCoords order_coords_;

	void relayout_icons_order_coords();
	void recalc_desired_size(bool);

	/**
	 * The ware on which the mouse press has been performed.
	 * It is not selected directly, but will be on mouse release.
	 */
	Widelands::DescriptionIndex selection_anchor_;
	std::function<void(Widelands::DescriptionIndex, bool)> callback_function_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
	int32_t min_free_vertical_space_;
};

/*
class WaresDisplay
------------------
Panel that displays the contents of many WareLists. The ware_lists
must be valid while they are registered with this class.
*/
class WaresDisplay : public AbstractWaresDisplay {
public:
	WaresDisplay(UI::Panel* const parent,
	             int32_t x,
	             int32_t y,
	             const Widelands::TribeDescr&,
	             Widelands::WareWorker type,
	             bool selectable);

	~WaresDisplay() override;

	void add_warelist(const Widelands::WareList&);
	void remove_all_warelists();

protected:
	uint32_t amount_of(Widelands::DescriptionIndex);
	std::string info_for_ware(Widelands::DescriptionIndex) override;

private:
	using WareListVector = std::vector<const Widelands::WareList*>;
	WareListVector warelists_;
};

class StockMenuWaresDisplay : public WaresDisplay {
public:
	StockMenuWaresDisplay(UI::Panel* const parent,
	                      int32_t x,
	                      int32_t y,
	                      const Widelands::Player&,
	                      Widelands::WareWorker type);

	void set_solid_icon_backgrounds(const bool s) {
		solid_icon_backgrounds_ = s;
	}

protected:
	RGBAColor draw_ware_background_overlay(Widelands::DescriptionIndex) override;
	std::string info_for_ware(Widelands::DescriptionIndex) override;

	const Widelands::Player& player_;
	bool solid_icon_backgrounds_;
};

std::string waremap_to_richtext(const Widelands::TribeDescr& tribe,
                                const std::map<Widelands::DescriptionIndex, uint8_t>& map);
std::string get_amount_string(uint32_t, bool cutoff1k = false);

#endif  // end of include guard: WL_WUI_WARESDISPLAY_H
