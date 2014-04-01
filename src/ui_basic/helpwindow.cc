/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "ui_basic/helpwindow.h"

#include <boost/format.hpp>

#include "constants.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "wlapplication.h"


using boost::format;

namespace UI {

HelpWindow::HelpWindow
	(Panel * const parent,
	 const std::string & caption,
	 uint32_t fontsize,
	 uint32_t width, uint32_t height)
	:
	Window(parent, "help_window", 0, 0, 20, 20, (boost::format(_("Help: %s")) % caption).str().c_str()),
	textarea(new Multiline_Textarea(this, 5, 5, 30, 30, std::string(), Align_Left)),
	m_h1((format("%u") % (fontsize < 12 ? 18 : fontsize * 3 / 2)).str()),
	m_h2((format("%u") % (fontsize < 12 ? 12 : fontsize)).str()),
	m_p ((format("%u") % (fontsize < 12 ? 10  : fontsize * 5 / 6)).str()),
	m_fn(ui_fn().substr(0, ui_fn().size() - 4)) // Font file - .ttf
{
	// Begin the text with the caption
	m_text  = "<rt text-align=center><p font-color=#AAFFAA font-face=";
	m_text += m_fn;
	m_text += " font-size=";
	m_text += m_h1;
	m_text += ">";
	m_text += caption;
	m_text += "</p></rt>";
	textarea->set_text(m_text);
	lastentry = HEADING;

	// Calculate sizes
	int32_t const out_width  = (width  == 0) ? g_gr->get_xres() * 3 / 5 : width;
	int32_t const out_height = (height == 0) ? g_gr->get_yres() * 4 / 5 : height;
	int32_t const but_height  = g_gr->get_yres() * 9 / 200;

	assert(out_width  >= 80);
	assert(out_height >= 60);
	int32_t in_width  = out_width  - 80;
	int32_t in_height = out_height - 60;

	set_inner_size(in_width, in_height);
	set_pos(Point((g_gr->get_xres() - out_width) / 2, (g_gr->get_yres() - out_height) / 2));

	Button * btn = new Button
		(this, "ok",
		 in_width / 3, in_height - but_height * 3 / 2,
		 in_width / 3, but_height,
		 g_gr->images().get("pics/but0.png"),
		 _("OK"), std::string(), true, false);
	btn->sigclicked.connect(boost::bind(&HelpWindow::pressedOk, boost::ref(*this)));
	btn->set_font(Font::get(UI_FONT_NAME, (fontsize < 12 ? 12 : fontsize)));

	textarea->set_size(in_width - 10, in_height - 10 - (2 * but_height));
}


HelpWindow::~HelpWindow()
{
	delete textarea;
}

/// Adds a new heading.
void HelpWindow::add_heading(std::string heading) {
	m_text += "<rt text-align=left><p font-color=#AAAAFF font-face=";
	m_text += m_fn;
	m_text += " font-size=";
	m_text += m_h2;
	m_text += "><br><br>";
	m_text += heading;
	m_text += "</p></rt>";
	textarea->set_text(m_text);
	lastentry = HEADING;
}

/// Adds a new paragraph.
void HelpWindow::add_paragraph(std::string block) {
	m_text += "<rt><p font-face=";
	m_text += m_fn;
	m_text += " font-size=";
	m_text += m_p;
	if (lastentry == HEADING)
		m_text += ">";
	else
		m_text += "><br>";
	lastentry = BLOCK;
	return add_block(block);
}

/// Behaves the same as add_paragraph, just it adds only one < br> if last
/// written entry was already a block text.
void HelpWindow::add_block(std::string block) {
	if (lastentry == HEADING)
		return add_paragraph(block);
	m_text += "<br>";
	m_text += block;
	m_text += "</p></rt>";
	textarea->set_text(m_text);
	lastentry = BLOCK;
}

void HelpWindow::add_picture_li(std::string block, std::string picpath) {
	m_text += "<rt image=";
	m_text += picpath;
	m_text += " image-align=left><p font-face=";
	m_text += m_fn;
	m_text += " font-size=";
	m_text += m_p;
	m_text += ">";
	lastentry = BLOCK;
	return add_block(block);
}


/**
 * Handle mouseclick.
 *
 * Clicking the right mouse button inside the window acts like pressing Ok.
 */
bool HelpWindow::handle_mousepress(const uint8_t btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		pressedOk();
	}
	return true;
}

bool HelpWindow::handle_mouserelease(const uint8_t, int32_t, int32_t)
{
	return true;
}

void HelpWindow::pressedOk()
{
	if (is_modal())
		end_modal(0);
	else {
		// do not call die() here - could lead to broken pointers.
		// the window should get deleted with the parent anyways.
		set_visible(false);
	}
}

/*
===================
LuaTextHelpWindow
===================
*/

LuaTextHelpWindow::LuaTextHelpWindow
	(Panel * const parent,
	 UI::UniqueWindow::Registry & reg,
	 const std::string & caption,
	 std::string path_to_script,
	 uint32_t width, uint32_t height)
	:
	UI::UniqueWindow(parent, "help_window", &reg, width, height,
		(boost::format(_("Help: %s")) % caption).str().c_str()
	),
	textarea(new Multiline_Textarea(this, 5, 5, width - 10, height -10, std::string(), Align_Left))
{
	LuaInterface lua;

	try {
		std::unique_ptr<LuaTable> t = lua.run_script(path_to_script);
		textarea->set_text(t->get_string("text"));
	} catch (LuaError & err) {
		textarea->set_text(err.what());
	}
}

LuaTextHelpWindow::~LuaTextHelpWindow()
{
	delete textarea;
}

}
