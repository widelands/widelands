/*
 * Copyright (C) 2002, 2007-2008 by the Widelands Development Team
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

#include "fullscreen_menu_base.h"

#include "graphic.h"
#include "profile.h"
#include "rendertarget.h"
#include "wlapplication.h"

#include <stdio.h>

/*
==============================================================================

Fullscreen_Menu_Base

==============================================================================
*/

/** Fullscreen_Menu_Base::Fullscreen_Menu_Base(const char *bgpic)
 *
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
Fullscreen_Menu_Base::Fullscreen_Menu_Base(const char *bgpic)
	: UI::Panel(0, 0, 0, gr_x(), gr_y())
{
	// Switch graphics mode if necessary
	m_xres = gr_x();
	m_yres = gr_y();

	Section *s = g_options.pull_section("global");

	WLApplication::get()->init_graphics
			(m_xres, m_yres, s->get_int("depth", 16), s->get_bool("fullscreen", false));

	// Load background graphics
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "pics/%s", bgpic);
	m_pic_background = g_gr->get_picture(PicMod_Menu, buffer);
	m_res_background = g_gr->get_resized_picture
			(m_pic_background, m_xres, m_yres, Graphic::ResizeMode_Loose);
}

Fullscreen_Menu_Base::~Fullscreen_Menu_Base() {
	if (m_res_background != m_pic_background)
		g_gr->free_surface(m_res_background);
}


/**
 * Draw the background / splash screen
*/
void Fullscreen_Menu_Base::draw(RenderTarget & dst) {
	dst.blit(Point(0, 0), m_res_background);
}


uint32_t Fullscreen_Menu_Base::gr_x() {
	Section *s = g_options.pull_section("global");
	return s->get_int("xres", MENU_XRES);
}

uint32_t Fullscreen_Menu_Base::gr_y() {
	Section *s = g_options.pull_section("global");
	return s->get_int("yres", MENU_XRES);
}


uint32_t Fullscreen_Menu_Base::fs_small() {
	return UI_FONT_SIZE_SMALL * gr_y() / 600;
}

uint32_t Fullscreen_Menu_Base::fs_big() {
	return UI_FONT_SIZE_BIG * gr_y() / 600;
}

std::string Fullscreen_Menu_Base::ui_fn() {
	Section *s = g_options.pull_section("global");
	std::string style(s->get_string("ui_font_style", UI_FONT_NAME));
	return (style == "sans") ? UI_FONT_NAME_SANS : UI_FONT_NAME_SERIF;
}

