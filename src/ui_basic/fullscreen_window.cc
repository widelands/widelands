/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_basic/fullscreen_window.h"

#include <memory>

#include <boost/format.hpp>

#include "base/log.h"
#include "graphic/rendertarget.h"

/*
==============================================================================

FullscreenWindow

==============================================================================
*/
namespace UI {

FullscreenWindow::FullscreenWindow()
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     background_image_((boost::format("%1%fsmenu/background.png") % kTemplateDir).str()) {
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
		   layout();
	   });
	set_frame_image(FullscreenWindow::Frames::kCornerTopLeft, "fsmenu/top_left.png");
	set_frame_image(FullscreenWindow::Frames::kCornerTopRight, "fsmenu/top_right.png");
	set_frame_image(FullscreenWindow::Frames::kCornerBottomLeft, "fsmenu/bottom_left.png");
	set_frame_image(FullscreenWindow::Frames::kCornerBottomRight, "fsmenu/bottom_right.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeLeftTile, "fsmenu/left.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeRightTile, "fsmenu/right.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeTopTile, "fsmenu/top.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeBottomTile, "fsmenu/bottom.png");
	set_frame_image(FullscreenWindow::Frames::kCenter, "fsmenu/center.png");
}

FullscreenWindow::~FullscreenWindow() {
	overlays_.clear();
	frame_overlays_.clear();
}

void FullscreenWindow::add_overlay_image(const std::string& filename, Alignment align) {
	overlays_.push_back(
	   std::unique_ptr<const Overlay>(new Overlay(g_gr->images().get(filename), align)));
}

void FullscreenWindow::clear_overlays() {
	overlays_.clear();
}

void FullscreenWindow::set_frame_image(FullscreenWindow::Frames id, const std::string& filename) {
	frame_overlays_.insert(std::make_pair(id, g_gr->images().get(kTemplateDir + filename)));
}

const Image* FullscreenWindow::get_frame_image(FullscreenWindow::Frames id) const {
	if (frame_overlays_.count(id) == 1) {
		return frame_overlays_.find(id)->second;
	} else {
		return nullptr;
	}
}

/**
 * Draw the background / splash screen
 */
void FullscreenWindow::draw(RenderTarget& dst) {
	// Overall background
	dst.tile(Recti(0, 0, get_w(), get_h()), g_gr->images().get(background_image_), Vector2i::zero());

	// Center background
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCenter),
	           FullscreenWindow::Alignment(UI::Align::kCenter, UI::Align::kCenter));

	// Optional overlays
	for (const auto& overlay : overlays_) {
		blit_image(dst, overlay->image, overlay->align);
	}

	// Frame edges
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeLeftTile),
	           Alignment(UI::Align::kLeft, UI::Align::kTop), Tiling::kVertical);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeRightTile),
	           Alignment(UI::Align::kRight, UI::Align::kTop), Tiling::kVertical);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeTopTile),
	           Alignment(UI::Align::kLeft, UI::Align::kTop), Tiling::kHorizontal);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeBottomTile),
	           Alignment(UI::Align::kLeft, UI::Align::kBottom), Tiling::kHorizontal);

	// Frame corners
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerTopLeft),
	           FullscreenWindow::Alignment(UI::Align::kLeft, UI::Align::kTop));
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerTopRight),
	           FullscreenWindow::Alignment(UI::Align::kRight, UI::Align::kTop));
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerBottomLeft),
	           FullscreenWindow::Alignment(UI::Align::kLeft, UI::Align::kBottom));
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerBottomRight),
	           FullscreenWindow::Alignment(UI::Align::kRight, UI::Align::kBottom));
}

Recti FullscreenWindow::calculate_rect(const Image* image, Alignment align, Tiling tiling) {
	int x = 0;
	int y = 0;
	int w = image->width();
	int h = image->height();
	const int available_width = g_gr->get_xres();
	const int available_height = g_gr->get_yres();

	if (tiling != Tiling::kNone) {
		w = (tiling == Tiling::kVertical) ? w : available_width;
		h = (tiling == Tiling::kHorizontal) ? h : available_height;
	} else {
		const float scale =
		   std::min(1.f, std::max(static_cast<float>(available_width) / image->width(),
		                          static_cast<float>(available_height) / image->height()));
		w = scale * image->width();
		h = scale * image->height();
	}

	// Adjust horizontal alignment
	switch (align.halign) {
	case UI::Align::kRight:
		x = available_width - w;
		break;
	case UI::Align::kCenter:
		x = (available_width - w) / 2;
		break;
	case UI::Align::kLeft:
		break;
	}

	// Adjust vertical alignment
	switch (align.valign) {
	case UI::Align::kBottom:
		y = available_height - h;
		break;
	case UI::Align::kCenter:
		y = (available_height - h) / 2;
		break;
	case UI::Align::kTop:
		break;
	}
	return Recti(x, y, w, h);
}

void FullscreenWindow::blit_image(RenderTarget& dst,
                                  const Image* image,
                                  Alignment align,
                                  Tiling tiling) {
	if (image) {
		const Recti dest = FullscreenWindow::calculate_rect(image, align, tiling);
		if (tiling != Tiling::kNone) {
			dst.tile(dest, image, Vector2i::zero());
		} else {
			dst.blitrect_scale(dest.cast<float>(), image, Recti(0, 0, image->width(), image->height()),
			                   1., BlendMode::UseAlpha);
		}
	}
}

float FullscreenWindow::scale_factor() const {
	return std::max(1.0f, get_h() / 600.0f);
}

}  // namespace UI
