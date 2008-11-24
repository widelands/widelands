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

#ifndef UI_TABPANEL_H
#define UI_TABPANEL_H

#include "ui_panel.h"

#include <vector>

namespace UI {
/**
 * Provides a tab view; every tab is a panel that can contain any number of
 * sub-panels (such as buttons, other Tab_Panels, etc..) and an associated
 * picture.
 * The picture is displayed as a button the user can click to bring the panel
 * to the top.
 *
 * The Panels you add() to the Tab_Panel must be children of the Tab_Panel.
*/
struct Tab_Panel : public Panel {
	Tab_Panel(Panel* parent, int32_t x, int32_t y, uint32_t background);

	void resize();

	void set_snapparent(bool snapparent);
	bool get_snapparent() const {return m_snapparent;}

	uint32_t add
		(uint32_t picid, Panel* panel, const std::string & tooltip = std::string());

	void activate(uint32_t idx);

private:
	// Drawing and event handlers
	void draw(RenderTarget &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	void handle_mousein(bool inside);

	struct Tab {
		uint32_t        picid;
		std::string tooltip;
		Panel     * panel;
	};

	std::vector<Tab> m_tabs;
	uint32_t             m_active;         ///< index of the currently active tab
	bool             m_snapparent; ///< if true, resize parent to fit this panel

	int32_t              m_highlight;      ///< index of the highlighted button

	uint32_t             m_pic_background; ///< picture used to draw background
};
};

#endif
