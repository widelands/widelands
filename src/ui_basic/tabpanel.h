/*
 * Copyright (C) 2003, 2006-2009 by the Widelands Development Team
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

#ifndef UI_TABPANEL_H
#define UI_TABPANEL_H

#include <vector>

#include "ui_basic/panel.h"

namespace UI {
/**
 * This represents a Tab of the TabPanel. Note that this does no work
 * of drawing itself or handling anything really, it is only here to
 * offer the Panel interface for tabs so that the scripting interface
 * stays the same for all elements
 */
struct Tab_Panel;
struct Tab : public NamedPanel {
	friend struct Tab_Panel;

	Tab
		(Tab_Panel * parent,
		 uint32_t,
		 const std::string & name,
		 const Image*,
		 const std::string & gtooltip,
		 Panel             * gpanel);

	bool active();
	void activate();

private:
	Tab_Panel * m_parent;
	uint32_t    m_id;

	const Image* pic;
	std::string tooltip;
	Panel     * panel;
};

/**
 * Provides a tab view; every tab is a panel that can contain any number of
 * sub-panels (such as buttons, other Tab_Panels, etc..) and an associated
 * picture.
 * The picture is displayed as a button the user can click to bring the panel
 * to the top.
 *
 * The Panels you add() to the Tab_Panel must be children of the Tab_Panel.
 *
 */
struct Tab_Panel : public Panel {
	friend struct Tab;

	Tab_Panel(Panel * parent, int32_t x, int32_t y, const Image* background);
	// For Fullscreen menus
	Tab_Panel
		(Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 const Image* background);

	uint32_t add
		(const std::string & name,
		 const Image* pic,
		 Panel             * panel,
		 const std::string & tooltip = std::string());

	typedef std::vector<Tab *> TabList;

	const TabList & tabs();
	void activate(uint32_t idx);
	void activate(const std::string &);
	uint32_t active() {return m_active;}

protected:
	virtual void layout() override;
	virtual void update_desired_size() override;

private:
	// Drawing and event handlers
	void draw(RenderTarget &) override;

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	void handle_mousein(bool inside) override;


	TabList          m_tabs;
	uint32_t         m_active;         ///< index of the currently active tab
	int32_t          m_highlight;      ///< index of the highlighted button

	const Image* m_pic_background; ///< picture used to draw background
};
};

#endif
