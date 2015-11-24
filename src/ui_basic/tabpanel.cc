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

// Button height of tab buttons in pixels. Is also used for width with pictorial buttons.
constexpr int kTabPanelButtonHeight = 34;

// Margin around image. The image will be scaled down to fit into this rectangle with preserving size.
constexpr int kTabPanelImageMargin = 2;

// Left and right margin around text.
constexpr int kTabPanelTextMargin = 4;

//  height of the bar separating buttons and tab contents
constexpr int kTabPanelSeparatorHeight = 4;

// Constant to flag up when we're not at a tab.
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
	NamedPanel(parent, name, x, 0, w, kTabPanelButtonHeight, gtooltip),
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
	 const Image* background,
	 TabPanel::Type border_type)
	:
	Panel            (parent, x, y, 0, 0),
	m_active         (0),
	m_highlight      (kNotFound),
	m_pic_background (background),
	border_type_(border_type)
{}
TabPanel::TabPanel
	(Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 const Image* background,
	 TabPanel::Type border_type)
	:
	Panel            (parent, x, y, w, h),
	m_active         (0),
	m_highlight      (kNotFound),
	m_pic_background (background),
	border_type_(border_type)
{}

/**
 * Resize the visible tab based on our actual size.
 */
void TabPanel::layout()
{
	if (m_active < m_tabs.size()) {
		Panel * const panel = m_tabs[m_active]->panel;
		uint32_t h = get_h();

		// NOCOM fix for border
		// avoid excessive craziness in case there is a wraparound
		h = std::min(h, h - (kTabPanelButtonHeight + kTabPanelSeparatorHeight));
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
	w = kTabPanelButtonHeight * m_tabs.size();
	h = kTabPanelButtonHeight + kTabPanelSeparatorHeight;

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
	const Image* pic = UI::g_fh1->render(as_uifont(title));
	return add_tab(std::max(kTabPanelButtonHeight, pic->width() + 2 * kTabPanelTextMargin),
						name,
						title,
						pic,
						tooltip_text,
						panel);
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
	return add_tab(kTabPanelButtonHeight,
						name,
						"",
						pic,
						tooltip_text,
						panel);
}

/** Common adding function for textual and pictorial tabs. */
uint32_t TabPanel::add_tab(int32_t width,
									const std::string& name,
									const std::string& title,
									const Image* pic,
									const std::string& tooltip_text,
									Panel* panel) {
	assert(panel);
	assert(panel->get_parent() == this);

	size_t id = m_tabs.size();
	int32_t x = id > 0 ? m_tabs[id - 1]->get_x() + m_tabs[id - 1]->get_w() : 0;
	m_tabs.push_back(new Tab(this, id, x, width, name, title, pic, tooltip_text, panel));

	// Add a margin if there is a border
	// NOCOM make the margins uniform for all tab panels, with or without border
	if (border_type_ == TabPanel::Type::kBorder) {
		panel->set_border(kTabPanelSeparatorHeight + 1, kTabPanelSeparatorHeight + 1,
								kTabPanelSeparatorHeight, kTabPanelSeparatorHeight);
		panel->set_pos(Point(0, kTabPanelButtonHeight));
	} else {
		panel->set_pos(Point(0, kTabPanelButtonHeight + kTabPanelSeparatorHeight));
	}

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
	static_assert(2 < kTabPanelButtonHeight, "assert(2 < kTabPanelButtonSize) failed.");
	static_assert(4 < kTabPanelButtonHeight, "assert(4 < kTabPanelButtonSize) failed.");

	if (m_pic_background) {
		dst.tile
			(Rect(Point(0, 0), m_tabs.size() * kTabPanelButtonHeight, kTabPanelButtonHeight - 2),
			 m_pic_background, Point(get_x(), get_y()));
		assert(kTabPanelButtonHeight - 2 <= get_h());
		dst.tile
			(Rect
			 (Point(0, kTabPanelButtonHeight - 2),
			  get_w(), get_h() - kTabPanelButtonHeight + 2),
			 m_pic_background,
			 Point(get_x(), get_y() + kTabPanelButtonHeight - 2));
	}

	RGBColor black(0, 0, 0);

	// draw the buttons
	int32_t x;
	int tab_width;
	for (size_t idx = 0; idx < m_tabs.size(); ++idx) {
		x = m_tabs[idx]->get_x();
		tab_width = m_tabs[idx]->get_w();

		if (m_highlight == idx) {
			dst.brighten_rect(Rect(Point(x, 0), tab_width, kTabPanelButtonHeight), MOUSE_OVER_BRIGHT_FACTOR);
		}

		assert(m_tabs[idx]->pic);

		// If the title is empty, we will assume a pictorial tab
		if (m_tabs[idx]->title.empty()) {
			// Scale the image down if needed, but keep the ratio.
			constexpr int kMaxImageSize = kTabPanelButtonHeight - 2 * kTabPanelImageMargin;
			double image_scale =
				std::min(1.,
							std::min(static_cast<double>(kMaxImageSize) / m_tabs[idx]->pic->width(),
										static_cast<double>(kMaxImageSize) / m_tabs[idx]->pic->height()));

			uint16_t picture_width = image_scale * m_tabs[idx]->pic->width();
			uint16_t picture_height = image_scale * m_tabs[idx]->pic->height();
			dst.blitrect_scale(Rect(x + (kTabPanelButtonHeight - picture_width) / 2,
											(kTabPanelButtonHeight - picture_height) / 2,
											picture_width,
											picture_height),
									 m_tabs[idx]->pic,
									 Rect(0, 0, m_tabs[idx]->pic->width(), m_tabs[idx]->pic->height()),
									 1.,
									 BlendMode::UseAlpha);
		} else {
			dst.blit(Point(x + kTabPanelTextMargin, (kTabPanelButtonHeight - m_tabs[idx]->pic->height()) / 2),
						m_tabs[idx]->pic,
						BlendMode::UseAlpha,
						UI::Align_Left);
		}

		// Draw top part of border
		dst.brighten_rect
			(Rect(Point(x, 0), tab_width, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect
			(Rect(Point(x, 2), 2, kTabPanelButtonHeight - 4),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		dst.fill_rect
			(Rect(Point(x + tab_width - 2, 2), 1, kTabPanelButtonHeight - 4),
			 black);
		dst.fill_rect
			(Rect(Point(x + tab_width - 1, 1), 1, kTabPanelButtonHeight - 3),
			 black);

		// Draw bottom part
		if (m_active != idx)
			dst.brighten_rect
				(Rect(Point(x, kTabPanelButtonHeight - 2), tab_width, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
		else {
			dst.brighten_rect
				(Rect(Point(x, kTabPanelButtonHeight - 2), 2, 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);

			dst.brighten_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonHeight - 2), 2, 2),
				 2 * BUTTON_EDGE_BRIGHT_FACTOR);
			dst.fill_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonHeight - 1), 1, 1),
				 black);
			dst.fill_rect
				(Rect(Point(x + tab_width - 2, kTabPanelButtonHeight - 2), 2, 1),
				 black);
		}
	}

	// draw the remaining separator
	assert(x <= get_w());
	dst.brighten_rect
		(Rect(Point(x + tab_width, kTabPanelButtonHeight - 2), get_w() - x, 2),
		 2 * BUTTON_EDGE_BRIGHT_FACTOR);

	// Draw border around the main panel
	if (border_type_ == TabPanel::Type::kBorder) {
		//  left edge
		dst.brighten_rect
			(Rect(Point(0, kTabPanelButtonHeight), 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		//  bottom edge
		dst.fill_rect(Rect(Point(2, get_h() - 2), get_w() - 2, 1), black);
		dst.fill_rect(Rect(Point(1, get_h() - 1), get_w() - 1, 1), black);
		//  right edge
		dst.fill_rect(Rect(Point(get_w() - 2, kTabPanelButtonHeight - 1), 1, get_h() - 2), black);
		dst.fill_rect(Rect(Point(get_w() - 1, kTabPanelButtonHeight - 2), 1, get_h() - 1), black);
	}
}


/**
 * Cancel all highlights when the mouse leaves the panel
*/
void TabPanel::handle_mousein(bool inside)
{
	if (!inside && m_highlight != kNotFound) {
		update(m_tabs[m_highlight]->get_x(), 0, m_tabs[m_highlight]->get_w(), kTabPanelButtonHeight);
		m_highlight = kNotFound;
	}
}


/**
 * Update highlighting
*/
bool TabPanel::handle_mousemove
	(uint8_t, int32_t const x, int32_t const y, int32_t, int32_t)
{
	size_t hl = find_tab(x, y);

	if (hl != m_highlight) {
		if (hl != kNotFound) {
			update(m_tabs[hl]->get_x(), 0, m_tabs[hl]->get_w(), kTabPanelButtonHeight);
		}
		if (m_highlight != kNotFound) {
			update(m_tabs[m_highlight]->get_x(), 0, m_tabs[m_highlight]->get_w(), kTabPanelButtonHeight);
		}
		m_highlight = hl;
		set_tooltip(m_highlight != kNotFound ? m_tabs[m_highlight]->tooltip : "");
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
	if (y < 0 || y >= kTabPanelButtonHeight) {
		return kNotFound;
	}

	int32_t width = 0;
	for (size_t id = 0; id < m_tabs.size(); ++id) {
		width += m_tabs[id]->get_w();
		if (width > x) {
			return id;
		}
	}
	return kNotFound;
}


}
