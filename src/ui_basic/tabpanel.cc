/*
 * Copyright (C) 2003, 2006-2010 by the Widelands Development Team
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

#include "ui_basic/tabpanel.h"

#include "graphic/rendertarget.h"
#include "ui_basic/mouse_constants.h"

namespace UI {
#define TP_BUTTON_WIDTH     34
#define TP_BUTTON_HEIGHT    34

//  height of the bar separating buttons and tab contents
#define TP_SEPARATOR_HEIGHT  4

/*
 * =================
 * class Tab
 * =================
 */
Tab::Tab
	(Tab_Panel         * const parent,
	 uint32_t            const id,
	 const std::string &       name,
	 const Image* gpic,
	 const std::string &       gtooltip,
	 Panel             * const gpanel)
	:
	NamedPanel
		(parent, name, id * TP_BUTTON_WIDTH, 0, TP_BUTTON_WIDTH,
		 TP_BUTTON_HEIGHT, gtooltip),
	m_parent(parent),
	m_id(id),
	pic(gpic),
	tooltip(gtooltip),
	panel(gpanel)
{
}

/**
 * Currently active tab
 */
bool Tab::active() {
	return m_parent->m_active == m_id;
}
void Tab::activate() {
	return m_parent->activate(m_id);
}

/*
 * =================
 * class Tab_Panel
 * =================
 */
/**
 * Initialize an empty Tab_Panel
*/
Tab_Panel::Tab_Panel
	(Panel * const parent,
	 int32_t const x, int32_t const y,
	 const Image* background)
	:
	Panel            (parent, x, y, 0, 0),
	m_active         (0),
	m_highlight      (-1),
	m_pic_background (background)
{}
Tab_Panel::Tab_Panel
	(Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 const Image* background)
	:
	Panel            (parent, x, y, w, h),
	m_active         (0),
	m_highlight      (-1),
	m_pic_background (background)
{}

/**
 * Resize the visible tab based on our actual size.
 */
void Tab_Panel::layout()
{
	if (m_active < m_tabs.size()) {
		Panel * const panel = m_tabs[m_active]->panel;
		uint32_t h = get_h();

		// avoid excessive craziness in case there is a wraparound
		h = std::min(h, h - (TP_BUTTON_HEIGHT + TP_SEPARATOR_HEIGHT));
		panel->set_size(get_w(), h);
	}
}

/**
 * Compute our desired size based on the currently selected tab.
 */
void Tab_Panel::update_desired_size()
{
	uint32_t w;
	uint32_t h;

	// size of button row
	w = TP_BUTTON_WIDTH * m_tabs.size();
	h = TP_BUTTON_HEIGHT + TP_SEPARATOR_HEIGHT;

	// size of contents
	if (m_active < m_tabs.size()) {
		Panel * const panel = m_tabs[m_active]->panel;
		uint32_t panelw, panelh;

		panel->get_desired_size(panelw, panelh);
		// TODO  the panel might be bigger -> add a scrollbar in that case
		//panel->set_size(panelw, panelh);

		if (panelw > w)
			w = panelw;
		h += panelh;
	}

	set_desired_size(w, h);

	// This is not redundant, because even if all this doesn't change our
	// desired size, we were typically called because of a child window that
	// changed, and we need to relayout that.
	layout();
}

/**
 * Add a new tab
*/
uint32_t Tab_Panel::add
	(const std::string & name,
	 const Image* pic,
	 Panel             * const panel,
	 const std::string &       tooltip_text)
{
	assert(panel);
	assert(panel->get_parent() == this);

	uint32_t id = m_tabs.size();
	m_tabs.push_back(new Tab(this, id, name, pic, tooltip_text, panel));

	panel->set_pos(Point(0, TP_BUTTON_HEIGHT + TP_SEPARATOR_HEIGHT));
	panel->set_visible(id == m_active);
	update_desired_size();

	return id;
}


/**
 * Make a different tab the currently active tab.
*/
void Tab_Panel::activate(uint32_t idx)
{
	if (m_active < m_tabs.size())
		m_tabs[m_active]->panel->set_visible(false);
	if (idx < m_tabs.size())
		m_tabs[idx]->panel->set_visible(true);

	m_active = idx;

	update_desired_size();
}

void Tab_Panel::activate(const std::string & name)
{
	for (uint32_t t = 0; t < m_tabs.size(); ++t)
		if (m_tabs[t]->get_name() == name)
			activate(t);
}

/**
 * Return the tab names in order
 */
const Tab_Panel::TabList & Tab_Panel::tabs() {
	return m_tabs;
}

/**
 * Draw the buttons and the tab
*/
void Tab_Panel::draw(RenderTarget & dst)
{
	uint32_t idx;
	uint32_t x;

	// draw the background
	static_assert(2 < TP_BUTTON_WIDTH, "assert(2 < TP_BUTTON_WIDTH) failed.");
	static_assert(4 < TP_BUTTON_HEIGHT, "assert(4 < TP_BUTTON_HEIGHT) failed.");

	if (m_pic_background) {
		dst.tile
			(Rect(Point(0, 0), m_tabs.size() * TP_BUTTON_WIDTH, TP_BUTTON_HEIGHT - 2),
			 m_pic_background, Point(get_x(), get_y()));
		assert(TP_BUTTON_HEIGHT - 2 <= get_h());
		dst.tile
			(Rect
			 (Point(0, TP_BUTTON_HEIGHT - 2),
			  get_w(), get_h() - TP_BUTTON_HEIGHT + 2),
			 m_pic_background,
			 Point(get_x(), get_y() + TP_BUTTON_HEIGHT - 2));
	}

	// draw the buttons
	for (idx = 0, x = 0; idx < m_tabs.size(); idx++, x += TP_BUTTON_WIDTH) {
		if (m_highlight == static_cast<int32_t>(idx))
			dst.brighten_rect
				(Rect(Point(x, 0), TP_BUTTON_WIDTH, TP_BUTTON_HEIGHT),
				 MOUSE_OVER_BRIGHT_FACTOR);

		// Draw the icon
		assert(m_tabs[idx]->pic);
		uint16_t cpw = m_tabs[idx]->pic->width();
		uint16_t cph = m_tabs[idx]->pic->height();
		dst.blit
			(Point(x + (TP_BUTTON_WIDTH - cpw) / 2, (TP_BUTTON_HEIGHT - cph) / 2),
			 m_tabs[idx]->pic);

		// Draw top part of border
		RGBColor black(0, 0, 0);

		dst.brighten_rect
			(Rect(Point(x, 0), TP_BUTTON_WIDTH, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect
			(Rect(Point(x, 2), 2, TP_BUTTON_HEIGHT - 4),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		dst.fill_rect
			(Rect(Point(x + TP_BUTTON_WIDTH - 2, 2), 1, TP_BUTTON_HEIGHT - 4),
			 black);
		dst.fill_rect
			(Rect(Point(x + TP_BUTTON_WIDTH - 1, 1), 1, TP_BUTTON_HEIGHT - 3),
			 black);

		// Draw bottom part
		if (m_active != idx)
			dst.brighten_rect
				(Rect(Point(x, TP_BUTTON_HEIGHT - 2), TP_BUTTON_WIDTH, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
		else {
			dst.brighten_rect
				(Rect(Point(x, TP_BUTTON_HEIGHT - 2), 2, 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);

			dst.brighten_rect
				(Rect(Point(x + TP_BUTTON_WIDTH - 2, TP_BUTTON_HEIGHT - 2), 2, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
			dst.fill_rect
				(Rect(Point(x + TP_BUTTON_WIDTH - 2, TP_BUTTON_HEIGHT - 1), 1, 1),
				 black);
			dst.fill_rect
				(Rect(Point(x + TP_BUTTON_WIDTH - 2, TP_BUTTON_HEIGHT - 2), 2, 1),
				 black);
		}
	}

	// draw the remaining separator
	assert(x <= static_cast<uint32_t>(get_w()));
	dst.brighten_rect
		(Rect(Point(x, TP_BUTTON_HEIGHT - 2), get_w() - x, 2),
		 2 * BUTTON_EDGE_BRIGHT_FACTOR);
}


/**
 * Cancel all highlights when the mouse leaves the panel
*/
void Tab_Panel::handle_mousein(bool inside)
{
	if (!inside && m_highlight >= 0) {
		update
			(m_highlight * TP_BUTTON_WIDTH, 0, TP_BUTTON_WIDTH, TP_BUTTON_HEIGHT);

		m_highlight = -1;
	}
}


/**
 * Update highlighting
*/
bool Tab_Panel::handle_mousemove
	(Uint8, int32_t const x, int32_t const y, int32_t, int32_t)
{
	int32_t hl;

	if (y < 0 || y >= TP_BUTTON_HEIGHT)
		hl = -1;
	else {
		hl = x / TP_BUTTON_WIDTH;

		if (m_tabs.size() <= static_cast<size_t>(hl))
			hl = -1;
	}

	if (hl != m_highlight) {
		{
			if (hl >= 0)
				set_tooltip(m_tabs[hl]->tooltip);
		}
		if (m_highlight >= 0)
			update
				(m_highlight * TP_BUTTON_WIDTH, 0,
				 TP_BUTTON_WIDTH, TP_BUTTON_HEIGHT);
		if (hl >= 0)
			update
				(hl * TP_BUTTON_WIDTH, 0, TP_BUTTON_WIDTH, TP_BUTTON_HEIGHT);

		m_highlight = hl;
	}
	return true;
}


/**
 * Change the active tab if a tab button has been clicked
*/
bool Tab_Panel::handle_mousepress(const Uint8 btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		int32_t id;

		if (y >= TP_BUTTON_HEIGHT)
			return false;

		id = x / TP_BUTTON_WIDTH;

		if (static_cast<size_t>(id) < m_tabs.size()) {
			activate(id);

			return true;
		}
	}

	return false;
}
bool Tab_Panel::handle_mouserelease(Uint8, int32_t, int32_t)
{
	return false;
}

}
