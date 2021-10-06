/*
 * Copyright (C) 2003-2021 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_TABPANEL_H
#define WL_UI_BASIC_TABPANEL_H

#include <memory>

#include "graphic/styles/font_style.h"
#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"

namespace UI {

// Button height of tab buttons in pixels. Is also used for width with pictorial buttons.
constexpr int kTabPanelButtonHeight = 34;

/**
 * This represents a Tab of the TabPanel. Note that this does no work
 * of drawing itself or handling anything really, it is only here to
 * offer the Panel interface for tabs so that the scripting interface
 * stays the same for all elements
 */
struct TabPanel;
struct Tab : public NamedPanel {
	friend struct TabPanel;
	friend class FileViewPanel;

	/** If title is not empty, this will be a textual tab.
	 *  In that case, pic will need to be the rendered title
	 *
	 * Text conventions: Title Case for the 'title', Sentence case for the 'gtooltip'
	 */
	Tab(TabPanel* parent,
	    PanelStyle,
	    size_t id,
	    int32_t x,
	    FontStyle,
	    const std::string& name,
	    const std::string& title,
	    const Image* pic,
	    const std::string& gtooltip,
	    Panel* gpanel);

	bool active();
	void activate();

	void set_title(const std::string&);
	void update_template() override;

private:
	// Leave handling the mouse move to the TabPanel.
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override {
		return false;
	}
	// Play click
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;

	TabPanel* parent;
	uint32_t id;

	const Image* pic;
	FontStyle font_style_;
	std::string title_;
	std::shared_ptr<const UI::RenderedText> rendered_title;
	std::string tooltip;
	Panel* panel;
};

/**
 * Provides a tab view; every tab is a panel that can contain any number of
 * sub-panels (such as buttons, other TabPanels, etc..) and an associated
 * picture.
 * The picture is displayed as a button the user can click to bring the panel
 * to the top.
 *
 * The Panels you add() to the TabPanel must be children of the TabPanel.
 *
 */
struct TabPanel : public Panel {

	friend struct Tab;

	TabPanel(Panel* parent, UI::TabPanelStyle style);

	/** Add textual tab
	 *
	 * Text conventions: Title Case for the 'title', Sentence case for the 'tooltip'
	 */
	uint32_t add(const std::string& name,
	             const std::string& title,
	             Panel* panel,
	             const std::string& tooltip = std::string());

	/** Add pictorial tab
	 * Text conventions: Sentence case for the 'tooltip'
	 */
	uint32_t add(const std::string& name,
	             const Image* pic,
	             Panel* panel,
	             const std::string& tooltip = std::string());

	using TabList = std::vector<Tab*>;

	const TabList& tabs() const;
	void activate(uint32_t idx);
	void activate(const std::string&);
	uint32_t active() const {
		return active_;
	}
	// Removes the last tab if the 'tabname' matches. Returns whether a tab was removed.
	// We use the tabname as a safety precaution to prevent acidentally removing the wrong tab.
	bool remove_last_tab(const std::string& tabname);

	boost::signals2::signal<void()> sigclicked;

	bool handle_key(bool, SDL_Keysym) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

protected:
	void layout() override;
	void update_desired_size() override;

	UI::TabPanelStyle tab_style_;

	std::vector<Recti> focus_overlay_rects() override;

private:
	// Common adding function for textual and pictorial tabs
	uint32_t add_tab(const std::string& name,
	                 const std::string& title,
	                 const Image* pic,
	                 const std::string& tooltip,
	                 Panel* contents);

	// Drawing and event handlers
	void draw(RenderTarget&) override;

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	void handle_mousein(bool inside) override;

	size_t find_tab(int32_t x, int32_t y) const;

	TabList tabs_;
	size_t active_;     ///< index of the currently active tab
	size_t highlight_;  ///< index of the highlighted button

	const UI::TabPanelStyle background_style_;  // Background color and texture. Not owned.
	const UI::PanelStyleInfo& background_style() const;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_TABPANEL_H
