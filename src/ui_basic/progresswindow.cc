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

#include "constants.h"
#include "container_iterate.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/image_transformations.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"

#define PROGRESS_FONT_COLOR_FG        RGBColor(128, 128, 255)
#define PROGRESS_FONT_COLOR_BG        RGBColor(64, 64, 0)
//#define PROGRESS_FONT_COLOR PROGRESS_FONT_COLOR_FG, PROGRESS_FONT_COLOR_BG // not used
#define PROGRESS_STATUS_RECT_PADDING  2
#define PROGRESS_STATUS_BORDER_X      2
#define PROGRESS_STATUS_BORDER_Y      2
#define PROGRESS_LABEL_POSITION_Y     90 /* in percents, from top */

namespace UI {

ProgressWindow::ProgressWindow(const std::string & background)
	: m_xres(0), m_yres(0),
	m_background_pic(nullptr)
{
	set_background(background);
	step(_("Preparing..."));
}

ProgressWindow::~ProgressWindow() {
	const VisualizationArray & visualizations = m_visualizations;
	container_iterate_const(VisualizationArray, visualizations, i)
		(*i.current)->stop(); //  inform visualizations
}

void ProgressWindow::draw_background
	(RenderTarget & rt, const uint32_t xres, const uint32_t yres)
{
	m_label_center.x = xres / 2;
	m_label_center.y = yres * PROGRESS_LABEL_POSITION_Y / 100;
	Rect wnd_rect(Point(0, 0), xres, yres);

	if (!m_background_pic or xres != m_xres or yres != m_yres) {
		// (Re-)Load background graphics
		m_background_pic = ImageTransformations::resize(g_gr->images().get(m_background), xres, yres);

		const uint32_t h = g_fh->get_fontheight (UI_FONT_SMALL);
		m_label_rectangle.x = xres / 4;
		m_label_rectangle.w = xres / 2;
		m_label_rectangle.y =
		m_label_center.y - h / 2 - PROGRESS_STATUS_RECT_PADDING;
		m_label_rectangle.h = h + 2 * PROGRESS_STATUS_RECT_PADDING;
		// remember last resolution
		m_xres = xres;
		m_yres = yres;
	}

	rt.blit(Point(0, 0), m_background_pic);

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
	if (file_name.size() > 0) {
		if (g_fs->FileExists(file_name))
			m_background = file_name;
		else {
			m_background = "pics/progress.png";
		}
	} else
		m_background = "pics/progress.png";
	m_background_pic = nullptr;
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
	g_gr->update_rectangle(m_label_rectangle);

#ifdef _WIN32
		// Pump events to prevent "not responding" on windows
		SDL_PumpEvents();
#endif

	update(true);
}

void ProgressWindow::update(bool const repaint) {
	VisualizationArray & visualizations = m_visualizations;
	container_iterate_const(VisualizationArray, visualizations, i)
		(*i.current)->update(repaint); //  let visualizations do their work

	g_gr->refresh(false);
}

/**
 * Display a loader step description
 * std:string style format broke format argument list
 * on windows visual studio.
 */
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
	container_iterate(VisualizationArray, visualizations, i)
		if (*i.current == instance) {
			m_visualizations.erase (i.current);
			break;
		}
}

}
