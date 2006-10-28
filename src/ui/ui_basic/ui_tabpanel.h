/*
 * Copyright (C) 2003, 2006 by the Widelands Development Team
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

#ifndef included_ui_tabpanel_h
#define included_ui_tabpanel_h

#include <vector>
#include "ui_panel.h"

#include <string>

/**
Provides a tab view; every tab is a panel that can contain any number of
sub-panels (such as buttons, other UITab_Panels, etc..) and an associated
picture.
The picture is displayed as a button the user can click to bring the panel
to the top.

The UIPanels you add() to the UITab_Panel must be children of the UITab_Panel.
*/
class UITab_Panel : public UIPanel {
public:
	UITab_Panel(UIPanel* parent, int x, int y, uint background);

	void resize();

	void set_snapparent(bool snapparent);
	bool get_snapparent() const { return m_snapparent; }

	uint add
		(uint picid, UIPanel* panel, const std::string & tooltip = std::string());

	void activate(uint idx);

private:
	// Drawing and event handlers
	void draw(RenderTarget* dst);

	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);
	void handle_mousemove(int x, int y, int xdiff, int ydiff);
	void handle_mousein(bool inside);

	struct Tab {
		uint		picid;
		std::string tooltip;
		UIPanel*	panel;
	};

	std::vector<Tab>	m_tabs;
	uint					m_active; ///< index of the currently active tab
	bool					m_snapparent; ///< if true, resize parent to fit this panel

	int					m_highlight; ///< index of the highlighted button

	uint					m_pic_background; ///< picture used to draw background
};


#endif // included_ui_tabpanel_h
