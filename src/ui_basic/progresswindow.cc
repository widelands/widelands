/*
 * Copyright (C) 2007-2016 by the Widelands Development Team
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
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"

namespace {

#define PROGRESS_FONT_COLOR_FG RGBColor(128, 128, 255)
#define PROGRESS_FONT_COLOR_BG RGBColor(64, 64, 0)
#define PROGRESS_STATUS_RECT_PADDING 2
#define PROGRESS_STATUS_BORDER_X 2
#define PROGRESS_STATUS_BORDER_Y 2
#define PROGRESS_LABEL_POSITION_Y 90 /* in percents, from top */

}  // namespace

namespace UI {

ProgressWindow::ProgressWindow(const std::string& background) {
	set_background(background);
	step(_("Loading…"));
}

ProgressWindow::~ProgressWindow() {
	for (IProgressVisualization* visualization : visualizations_) {
		visualization->stop();  //  inform visualizations
	}
}

void ProgressWindow::draw_background(RenderTarget& rt, const uint32_t xres, const uint32_t yres) {
	label_center_.x = xres / 2;
	label_center_.y = yres * PROGRESS_LABEL_POSITION_Y / 100;
	Recti wnd_rect(Vector2i(0, 0), xres, yres);

	const uint32_t h = text_height();

	label_rectangle_.x = xres / 4.f;
	label_rectangle_.w = xres / 2.f;
	label_rectangle_.y = label_center_.y - h / 2.f - PROGRESS_STATUS_RECT_PADDING;
	label_rectangle_.h = h + 2.f * PROGRESS_STATUS_RECT_PADDING;

	const Image* bg = g_gr->images().get(background_);
	rt.blitrect_scale(Rectf(0.f, 0.f, xres, yres), bg, Recti(0, 0, bg->width(), bg->height()), 1.,
	                  BlendMode::UseAlpha);

	Rectf border_rect = label_rectangle_;
	border_rect.x -= PROGRESS_STATUS_BORDER_X;
	border_rect.y -= PROGRESS_STATUS_BORDER_Y;
	border_rect.w += 2 * PROGRESS_STATUS_BORDER_X;
	border_rect.h += 2 * PROGRESS_STATUS_BORDER_Y;

	rt.draw_rect(border_rect, PROGRESS_FONT_COLOR_FG);
}

/// Set a picture to render in the background
void ProgressWindow::set_background(const std::string& file_name) {
	RenderTarget& rt = *g_gr->get_render_target();
	if (!file_name.empty() && g_fs->file_exists(file_name)) {
		background_ = file_name;
	} else {
		background_ = "images/loadscreens/progress.png";
	}
	draw_background(rt, g_gr->get_xres(), g_gr->get_yres());
}

void ProgressWindow::step(const std::string& description) {
	RenderTarget& rt = *g_gr->get_render_target();

	const uint32_t xres = g_gr->get_xres();
	const uint32_t yres = g_gr->get_yres();

	// always repaint the background first
	draw_background(rt, xres, yres);

	rt.fill_rect(label_rectangle_, PROGRESS_FONT_COLOR_BG);
	// NOCOM
	const Image* rendered_text = UI::g_fh1->render_multi(as_uifont(description, UI_FONT_SIZE_SMALL, PROGRESS_FONT_COLOR_FG))->texts[0]->image;
	UI::correct_for_align(UI::Align::kCenter, rendered_text->width(), rendered_text->height(), &label_center_);
	rt.blit(label_center_.cast<float>(), rendered_text);

#ifdef _WIN32
	// Pump events to prevent "not responding" on windows
	SDL_PumpEvents();
#endif
	update(true);
}

void ProgressWindow::update(bool const repaint) {
	for (IProgressVisualization* visualization : visualizations_) {
		visualization->update(repaint);  //  let visualizations do their work
	}
	g_gr->refresh();
}

/// Register additional visualization (tips/hints, animation, etc)
void ProgressWindow::add_visualization(IProgressVisualization* const instance) {
	// just add to collection
	visualizations_.push_back(instance);
}

void ProgressWindow::remove_visualization(IProgressVisualization* instance) {
	VisualizationArray& visualizations = visualizations_;

	for (VisualizationArray::iterator vis_iter = visualizations.begin();
	     vis_iter != visualizations.end(); ++vis_iter) {

		if (*vis_iter == instance) {
			visualizations_.erase(vis_iter);
			break;
		}
	}
}
}
