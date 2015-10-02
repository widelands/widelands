/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_HELPWINDOW_H
#define WL_UI_BASIC_HELPWINDOW_H

#include <memory>

#include "graphic/align.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

class LuaInterface;

namespace Widelands {
struct BuildingDescr;
}

namespace UI {

/**
 * Shows a help window.
 *
 * Using it is quite straightforward. To ensure, that all help windows have the
 * same formations, all richtext formating will be done via add_* functions:
 *     HelpWindow help(parent, "Caption", fontsize);
 *     help.add_heading("A minor heading");
 *     help.add_block("Some lines of text!");
 *     help.add_block("More text, just one linebreak between");
 *     help.add_paragraph("Even more text, now drawn in a new paragraph");
 *     help.add_block("More text, same paragraph, but a linebreak between!");
 *     help.add_heading("Another minor heading");
 *     ...
 *     help.run();
*/
struct HelpWindow : public Window {
	HelpWindow
		(Panel * parent,
		 const std::string & caption,
		 uint32_t fontsize,
		 uint32_t width = 0, uint32_t height = 0);
	~HelpWindow();

	bool handle_mousepress  (uint8_t btn, int32_t mx, int32_t my) override;
	bool handle_mouserelease(uint8_t btn, int32_t mx, int32_t my) override;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	void add_heading   (std::string text);
	void add_paragraph (std::string text);
	void add_block     (std::string text);
	void add_picture_li(std::string text, std::string picpath);

protected:
	void clicked_ok();

private:
	enum State {
		BLOCK = 0,
		HEADING = 1
	} lastentry;

	std::unique_ptr<MultilineTextarea> textarea;
	std::string const m_h1, m_h2, m_p; // font sizes
	std::string const m_fn; // font name
	std::string m_text;
};

/*
 * This is a totally different beast than HelpWindow. It takes
 * a Lua script, runs it and displays it's formatted content
 * as a static text
 */
struct LuaTextHelpWindow : public UI::UniqueWindow {
	LuaTextHelpWindow
		(Panel * parent, UI::UniqueWindow::Registry& reg,
		 const Widelands::BuildingDescr& building_description,
		 LuaInterface * const lua,
		 uint32_t width = 300, uint32_t height = 400);
	~LuaTextHelpWindow();

private:
	std::unique_ptr<MultilineTextarea> textarea;
};

}

#endif  // end of include guard: WL_UI_BASIC_HELPWINDOW_H
