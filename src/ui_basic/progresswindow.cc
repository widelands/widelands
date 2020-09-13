/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#include <cstdlib>
#include <memory>
#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include <SDL_events.h>

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"

constexpr int16_t kProgressStatusRectPadding = 2;
constexpr int16_t kProgressStatusBorderX = 2;
constexpr int16_t kProgressStatusBorderY = 2;
constexpr int16_t kProgressStatusPositionY = 90; /* in percents, from top */

namespace UI {

ProgressWindow::ProgressWindow(const std::string& background)
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     label_center_(Vector2i::zero()),
     style_(g_style_manager->progressbar_style(UI::PanelStyle::kFsMenu)) {

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
	   });

	set_background(background);
	step(_("Loading…"));
}

ProgressWindow::~ProgressWindow() {
	for (IProgressVisualization* visualization : visualizations_) {
		visualization->stop();  //  inform visualizations
	}
}

void ProgressWindow::draw(RenderTarget& rt) {
	{  // Center and downscale background image
		const Image& bg = *g_image_cache->get(background_);
		const float w = bg.width();
		const float h = bg.height();
		rt.blitrect_scale(
		   fit_image(w, h, get_w(), get_h()), &bg, Recti(0, 0, w, h), 1.f, BlendMode::UseAlpha);
	}

	// No float division to avoid Texture subsampling.
	label_center_.x = get_w() / 2;
	label_center_.y = get_h() * kProgressStatusPositionY / 100;

	const uint32_t h = text_height(style_.font());

	label_rectangle_.x = get_w() / 6;
	label_rectangle_.w = get_w() * 2 / 3;
	label_rectangle_.y = label_center_.y - h / 2 - kProgressStatusRectPadding;
	label_rectangle_.h = h + 2 * kProgressStatusRectPadding;

	Recti border_rect = label_rectangle_;
	border_rect.x -= kProgressStatusBorderX;
	border_rect.y -= kProgressStatusBorderY;
	border_rect.w += 2 * kProgressStatusBorderX;
	border_rect.h += 2 * kProgressStatusBorderY;

	rt.draw_rect(border_rect, style_.font().color());
	// TODO(GunChleoc): this should depend on actual progress. Add a total steps variable and reuse
	// the Progressbar class.
	rt.fill_rect(label_rectangle_, style_.medium_color());
}

/// Set a picture to render in the background
void ProgressWindow::set_background(const std::string& file_name) {
	if (file_name.empty() || !g_fs->file_exists(file_name)) {
		const std::set<std::string> images =
		   g_fs->list_directory(std::string(kTemplateDir) + "loadscreens/gameloading");
		auto it = images.begin();
		std::advance(it, std::rand() % images.size());  // NOLINT
		background_ = *it;
	} else {
		background_ = file_name;
	}
	draw(*g_gr->get_render_target());
}

void ProgressWindow::step(const std::string& description) {
	RenderTarget& rt = *g_gr->get_render_target();
	// always repaint the background first
	draw(rt);

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_richtext_paragraph(description, style_.font()));
	UI::center_vertically(rendered_text->height(), &label_center_);
	rendered_text->draw(rt, label_center_, UI::Align::kCenter);

	// Pump events to prevent "not responding" on windows & "beach ball" on macOS
	SDL_PumpEvents();
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
}  // namespace UI
