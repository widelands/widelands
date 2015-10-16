/*
 * Copyright (C) 2007-2008, 2010, 2013 by the Widelands Development Team
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

#include "ui_basic/progresswindow.h"

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"

#define PROGRESS_FONT_COLOR_FG        RGBColor(128, 128, 255)
#define PROGRESS_FONT_COLOR_BG        RGBColor(64, 64, 0)
#define PROGRESS_STATUS_RECT_PADDING  2
#define PROGRESS_STATUS_BORDER_X      2
#define PROGRESS_STATUS_BORDER_Y      2
#define PROGRESS_LABEL_POSITION_Y     90 /* in percents, from top */

namespace UI {

ProgressWindow::ProgressWindow(const std::string& background) {
	set_background(background);
	step(_("Preparing..."));
}

ProgressWindow::~ProgressWindow() {
	for (IProgressVisualization * visualization : m_visualizations) {
		visualization->stop(); //  inform visualizations
	}
}

void ProgressWindow::draw_background
	(RenderTarget & rt, const uint32_t xres, const uint32_t yres)
{
	m_label_center.x = xres / 2;
	m_label_center.y = yres * PROGRESS_LABEL_POSITION_Y / 100;
	Rect wnd_rect(Point(0, 0), xres, yres);

	const uint32_t h = g_fh->get_fontheight(UI::g_fh1->fontset()->serif(),
														 UI_FONT_SIZE_SMALL);
	m_label_rectangle.x = xres / 4;
	m_label_rectangle.w = xres / 2;
	m_label_rectangle.y =
	m_label_center.y - h / 2 - PROGRESS_STATUS_RECT_PADDING;
	m_label_rectangle.h = h + 2 * PROGRESS_STATUS_RECT_PADDING;

	const Image* bg = g_gr->images().get(m_background);
	rt.blitrect_scale(
	   Rect(0, 0, xres, yres), bg, Rect(0, 0, bg->width(), bg->height()), 1., BlendMode::UseAlpha);

	Rect border_rect = m_label_rectangle;
	border_rect.x -= PROGRESS_STATUS_BORDER_X;
	border_rect.y -= PROGRESS_STATUS_BORDER_Y;
	border_rect.w += 2 * PROGRESS_STATUS_BORDER_X;
	border_rect.h += 2 * PROGRESS_STATUS_BORDER_Y;

	rt.draw_rect(border_rect, PROGRESS_FONT_COLOR_FG);
}

/// Set a picture to render in the background
void ProgressWindow::set_background(const std::string & file_name) {
	RenderTarget & rt = *g_gr->get_render_target();
	if (!file_name.empty() && g_fs->file_exists(file_name)) {
		m_background = file_name;
	} else {
		m_background = "pics/progress.png";
	}
	draw_background(rt, g_gr->get_xres(), g_gr->get_yres());
	update(true);
}

void ProgressWindow::step(const std::string & description) {
	RenderTarget & rt = *g_gr->get_render_target();

	const uint32_t xres = g_gr->get_xres();
	const uint32_t yres = g_gr->get_yres();

	// always repaint the background first
	draw_background(rt, xres, yres);

	rt.fill_rect(m_label_rectangle, PROGRESS_FONT_COLOR_BG);

	UI::TextStyle ts(UI::TextStyle::ui_small());
	ts.fg = PROGRESS_FONT_COLOR_FG;
	UI::g_fh->draw_text(rt, ts, m_label_center, description, Align_Center);
	g_gr->update();

#ifdef _WIN32
		// Pump events to prevent "not responding" on windows
		SDL_PumpEvents();
#endif

	update(true);
}

void ProgressWindow::update(bool const repaint) {
	for (IProgressVisualization * visualization : m_visualizations) {
		visualization->update(repaint); //  let visualizations do their work
	}
	g_gr->refresh();
}

/**
 * Display a loader step description
 * std:string style format broke format argument list
 * on windows visual studio.
 */
// TODO(sirver): this should just take a string.
void ProgressWindow::stepf(const char * format, ...) {
	char buffer[1024];
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, sizeof(buffer), format, va);
	va_end(va);
	step (buffer);
}

/// Register additional visualization (tips/hints, animation, etc)
void ProgressWindow::add_visualization(IProgressVisualization * const instance)
{
	// just add to collection
	m_visualizations.push_back(instance);
}

void ProgressWindow::remove_visualization(IProgressVisualization * instance) {
	VisualizationArray & visualizations = m_visualizations;

	for (VisualizationArray::iterator vis_iter = visualizations.begin();
		  vis_iter != visualizations.end();
		  ++vis_iter) {

		if (*vis_iter == instance) {
			m_visualizations.erase (vis_iter);
			break;
		}
	}
}

}
