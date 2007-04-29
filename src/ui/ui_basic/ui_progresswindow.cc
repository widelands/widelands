/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "ui_progresswindow.h"

#include "font_handler.h"
#include "i18n.h"

#define PROGRESS_FONT_COLOR_FG        RGBColor(128,128,255)
#define PROGRESS_FONT_COLOR_BG        RGBColor(64,64,0)
#define PROGRESS_FONT_COLOR PROGRESS_FONT_COLOR_FG, PROGRESS_FONT_COLOR_BG
#define PROGRESS_STATUS_RECT_PADDING  2
#define PROGRESS_STATUS_BORDER_X      2
#define PROGRESS_STATUS_BORDER_Y      2
#define PROGRESS_LABEL_POSITION_Y     90 /* in percents, from top */

namespace UI {

ProgressWindow::ProgressWindow() {
	const uint xres = g_gr->get_xres();
	m_label_center.x = xres / 2;
	m_label_center.y = g_gr->get_yres() * PROGRESS_LABEL_POSITION_Y / 100;

	// Load background graphics
	const uint pic_background =
		g_gr->get_picture(PicMod_Menu, "pics/mainmenu.jpg");
	RenderTarget & rt = *g_gr->get_render_target();
	rt.blit(Point(0, 0), pic_background);

	const uint h = g_fh->get_fontheight (UI_FONT_SMALL);
	m_label_rectangle.x = xres / 4;
	m_label_rectangle.w = xres / 2;
	m_label_rectangle.y =
		m_label_center.y - h / 2 - PROGRESS_STATUS_RECT_PADDING;
	m_label_rectangle.h = h + 2 * PROGRESS_STATUS_RECT_PADDING;

	Rect border_rect = m_label_rectangle;
	border_rect.x -= PROGRESS_STATUS_BORDER_X;
	border_rect.y -= PROGRESS_STATUS_BORDER_Y;
	border_rect.w += 2 * PROGRESS_STATUS_BORDER_X;
	border_rect.h += 2 * PROGRESS_STATUS_BORDER_Y;

	rt.draw_rect(border_rect, PROGRESS_FONT_COLOR_FG);
	step(_("Preparing..."));
}


void ProgressWindow::step(const std::string & description) const {
	RenderTarget & rt = *g_gr->get_render_target();

	rt.fill_rect(m_label_rectangle, PROGRESS_FONT_COLOR_BG);
	g_fh->draw_string
		(rt, UI_FONT_SMALL, PROGRESS_FONT_COLOR, m_label_center, description, Align_Center);
	g_gr->update_rectangle(m_label_rectangle);
	g_gr->refresh();
}

/**
 * Display a loader step description
 */
void ProgressWindow::stepf(const std::string & format, ...) const {
	char buffer[1024];
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, sizeof(buffer), format.c_str(), va);
	va_end(va);
	step (buffer);
}
};
