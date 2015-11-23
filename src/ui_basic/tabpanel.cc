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

#include "base/log.h" // NOCOM
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"

namespace UI {

// Button size of tab buttons in pixels.
constexpr int kTabPanelButtonSize = 34;

// Margin around image. The image will be scaled down to fit into this rectangle with preserving size.
constexpr int kTabPanelImageMargin = 2;

constexpr int kTabPanelTextMargin = 4;

//  height of the bar separating buttons and tab contents
constexpr int kTabPanelSeparatorHeight = 4;

constexpr uint32_t kNotFound = std::numeric_limits<uint32_t>::max();

/*
 * =================
 * class Tab
 * =================
 */
Tab::Tab
	(TabPanel         * const parent,
	 size_t            const id,
	 int32_t x,
	 int32_t w,
	 const std::string &       name,
	 const std::string &       gtitle,
	 const Image* gpic,
	 const std::string &       gtooltip,
	 Panel             * const gpanel)
	:
	NamedPanel(parent, name, x, 0, w, kTabPanelButtonSize, gtooltip),
	m_parent(parent),
	m_id(id),
	pic(gpic),
	title(gtitle),
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
 * class TabPanel
 * =================
 */
/**
 * Initialize an empty TabPanel
*/
TabPanel::TabPanel
	(Panel * const parent,
	 int32_t const x, int32_t const y,
	 const Image* background)
	:
	Panel            (parent, x, y, 0, 0),
	m_active         (0),
	m_highlight      (kNotFound),
	m_pic_background (background)
{}
TabPanel::TabPanel
	(Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 const Image* background)
	:
	Panel            (parent, x, y, w, h),
	m_active         (0),
	m_highlight      (kNotFound),
	m_pic_background (background)
{}

/**
 * Resize the visible tab based on our actual size.
 */
void TabPanel::layout()
{
	if (m_active < m_tabs.size()) {
		Panel * const panel = m_tabs[m_active]->panel;
		uint32_t h = get_h();

		// avoid excessive craziness in case there is a wraparound
		h = std::min(h, h - (kTabPanelButtonSize + kTabPanelSeparatorHeight));
		panel->set_size(get_w(), h);
	}
}

/**
 * Compute our desired size based on the currently selected tab.
 */
void TabPanel::update_desired_size()
{
	uint32_t w;
	uint32_t h;

	// size of button row
	w = kTabPanelButtonSize * m_tabs.size();
	h = kTabPanelButtonSize + kTabPanelSeparatorHeight;

	// size of contents
	if (m_active < m_tabs.size()) {
		Panel * const panel = m_tabs[m_active]->panel;
		uint32_t panelw, panelh;

		panel->get_desired_size(panelw, panelh);
		// TODO(unknown):  the panel might be bigger -> add a scrollbar in that case
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
 * Add a new textual tab
*/
uint32_t TabPanel::add
	(const std::string & name,
	 const std::string & title,
	 Panel             * const panel,
	 const std::string &       tooltip_text)
{
	assert(panel);
	assert(panel->get_parent() == this);

	size_t id = m_tabs.size();
	int32_t x = id > 0 ? m_tabs[id - 1]->get_x() + m_tabs[id - 1]->get_w() : 0;
	const Image* pic = UI::g_fh1->render(as_uifont(title));
	m_tabs.push_back(new Tab(this, id, x, std::max(kTabPanelButtonSize, pic->width() + 2 * kTabPanelTextMargin), name, title, pic, tooltip_text, panel));
	log("NOCOM new textual tab %s at %d - %d\n", tooltip_text.c_str(), m_tabs[id]->get_x(), m_tabs[id]->get_w());
	panel->set_pos(Point(0, kTabPanelButtonSize + kTabPanelSeparatorHeight));
	panel->set_visible(id == m_active);
	update_desired_size();

	return id;
}

/**
 * Add a new pictorial tab
*/
uint32_t TabPanel::add
	(const std::string & name,
	 const Image* pic,
	 Panel             * const panel,
	 const std::string &       tooltip_text)
{
	assert(panel);
	assert(panel->get_parent() == this);

	size_t id = m_tabs.size();
	int32_t x = id > 0 ? m_tabs[id - 1]->get_x() + m_tabs[id - 1]->get_w() : 0;
	m_tabs.push_back(new Tab(this, id, x, kTabPanelButtonSize, name, "", pic, tooltip_text, panel));
	log("NOCOM new pictorial tab %s at %d - %d\n", tooltip_text.c_str(), m_tabs[id]->get_x(), m_tabs[id]->get_w());
	panel->set_pos(Point(0, kTabPanelButtonSize + kTabPanelSeparatorHeight));
	panel->set_visible(id == m_active);
	update_desired_size();

	return id;
}


/**
 * Make a different tab the currently active tab.
*/
void TabPanel::activate(uint32_t idx)
{
	if (m_active < m_tabs.size())
		m_tabs[m_active]->panel->set_visible(false);
	if (idx < m_tabs.size())
		m_tabs[idx]->panel->set_visible(true);

	m_active = idx;

	update_desired_size();
}

void TabPanel::activate(const std::string & name)
{
	for (uint32_t t = 0; t < m_tabs.size(); ++t)
		if (m_tabs[t]->get_name() == name)
			activate(t);
}

/**
 * Return the tab names in order
 */
const TabPanel::TabList & TabPanel::tabs() {
	return m_tabs;
}

/**
 * Draw the buttons and the tab
*/
void TabPanel::draw(RenderTarget & dst)
{
	// draw the background
	static_assert(2 < kTabPanelButtonSize, "assert(2 < kTabPanelButtonSize) failed.");
	static_assert(4 < kTabPanelButtonSize, "assert(4 < kTabPanelButtonSize) failed.");

	if (m_pic_background) {
		dst.tile
			(Rect(Point(0, 0), m_tabs.size() * kTabPanelButtonSize, kTabPanelButtonSize - 2),
			 m_pic_background, Point(get_x(), get_y()));
		assert(kTabPanelButtonSize - 2 <= get_h());
		dst.tile
			(Rect
			 (Point(0, kTabPanelButtonSize - 2),
			  get_w(), get_h() - kTabPanelButtonSize + 2),
			 m_pic_background,
			 Point(get_x(), get_y() + kTabPanelButtonSize - 2));
	}

	// draw the buttons
	int tab_width;
	size_t idx;
	int32_t x;
	for (idx = 0; idx < m_tabs.size(); ++idx) {
		tab_width = m_tabs[idx]->get_w();
		x = m_tabs[idx]->get_x();

		if (m_highlight == idx) {
			dst.brighten_rect
				(Rect(Point(x, 0), tab_width, kTabPanelButtonSize),
				 MOUSE_OVER_BRIGHT_FACTOR);
		}

		assert(m_tabs[idx]->pic);

		// If the title is empty, we will assume a pictorial tab
		if (m_tabs[idx]->title.empty()) {
			//log("NOCOM drawing pictorial tab at %d\n", x);
			// Scale the image down if needed, but keep the ratio.
			constexpr int kMaxImageSize = kTabPanelButtonSize - 2 * kTabPanelImageMargin;
			double image_scale =
				std::min(1.,
							std::min(static_cast<double>(kMaxImageSize) / m_tabs[idx]->pic->width(),
										static_cast<double>(kMaxImageSize) / m_tabs[idx]->pic->height()));

			uint16_t picture_width = image_scale * m_tabs[idx]->pic->width();
			uint16_t picture_height = image_scale * m_tabs[idx]->pic->height();
			dst.blitrect_scale(Rect(x + (kTabPanelButtonSize - picture_width) / 2,
											(kTabPanelButtonSize - picture_height) / 2,
											picture_width,
											picture_height),
									 m_tabs[idx]->pic,
									 Rect(0, 0, m_tabs[idx]->pic->width(), m_tabs[idx]->pic->height()),
									 1.,
									 BlendMode::UseAlpha);
		} else {
			//log("NOCOM drawing textual tab %s at %d\n", m_tabs[idx]->title.c_str(), x);

			dst.blit(Point(x + kTabPanelTextMargin, (kTabPanelButtonSize - m_tabs[idx]->pic->height()) / 2),
						m_tabs[idx]->pic,
						BlendMode::UseAlpha,
						UI::Align_Left);
		}

		// Draw top part of border
		RGBColor black(0, 0, 0);

		dst.brighten_rect
			(Rect(Point(x, 0), tab_width, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect
			(Rect(Point(x, 2), 2, kTabPanelButtonSize - 4),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		dst.fill_rect
			(Rect(Point(x + tab_width - 2, 2), 1, kTabPanelButtonSize - 4),
			 black);
		dst.fill_rect
			(Rect(Point(x + tab_width - 1, 1), 1, kTabPanelButtonSize - 3),
			 black);

		// Draw bottom part
		if (m_active != idx)
			dst.brighten_rect
				(Rect(Point(x, kTabPanelButtonSize - 2), tab_width, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
		else {
			dst.brighten_rect
				(Rect(Point(x, kTabPanelButtonSize - 2), 2, 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);

			dst.brighten_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonSize - 2), 2, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
			dst.fill_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonSize - 1), 1, 1),
				 black);
			dst.fill_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonSize - 2), 2, 1),
				 black);
		}
	}

	// draw the remaining separator
	assert(x <= get_w());
	dst.brighten_rect
		(Rect(Point(x, kTabPanelButtonSize - 2), get_w() - x, 2),
		 2 * BUTTON_EDGE_BRIGHT_FACTOR);
}


/**
 * Cancel all highlights when the mouse leaves the panel
*/
void TabPanel::handle_mousein(bool inside)
{
	if (!inside && m_highlight != kNotFound) {
		update
			(m_highlight * kTabPanelButtonSize, 0, kTabPanelButtonSize, kTabPanelButtonSize);

		m_highlight = kNotFound;
	}
}


/**
 * Update highlighting
*/
bool TabPanel::handle_mousemove
	(uint8_t, int32_t const x, int32_t const y, int32_t, int32_t)
{
	size_t hl;

	if (y < 0 || y >= kTabPanelButtonSize)
		hl = kNotFound;
	else {
		size_t id = find_tab(x, y);
		if (id != kNotFound) {
			hl = id;
		}

		// NOCOM can this line go?
		if (m_tabs.size() <= hl)
			hl = kNotFound;
	}

	if (hl != m_highlight) {
		{
			if (hl != kNotFound)
				set_tooltip(m_tabs[hl]->tooltip);
		}
		// NOCOM update the correct range
		if (m_highlight != kNotFound)
			update
				(m_highlight * kTabPanelButtonSize, 0,
				 kTabPanelButtonSize, kTabPanelButtonSize);
		if (hl != kNotFound)
			update
				(hl * kTabPanelButtonSize, 0, kTabPanelButtonSize, kTabPanelButtonSize);

		m_highlight = hl;
	}
	return true;
}


/**
 * Change the active tab if a tab button has been clicked
*/
bool TabPanel::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		size_t id = find_tab(x, y);
		if (id != kNotFound) {
			activate(id);
			return true;
		}
	}
	return false;
}

bool TabPanel::handle_mouserelease(uint8_t, int32_t, int32_t)
{
	return false;
}


/**
 * Find the tab at the coordinates x, y
 * Returns kNotFound if no tab was found
 */
size_t TabPanel::find_tab(int32_t x, int32_t y) const {
	if (y < 0 || y >= kTabPanelButtonSize) {
		return kNotFound;
	}

	int32_t width = 0;
	for (size_t id = 0; id < m_tabs.size(); ++id) {
		width += m_tabs[id]->get_w();
		if (width > x) {
			if (id < m_tabs.size()) {
				return id;
			}
			break;
		}
	}
	return kNotFound;
}


}
