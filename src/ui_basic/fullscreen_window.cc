/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <cstdio>

#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"

/*
==============================================================================

FullscreenWindow

==============================================================================
*/
namespace UI {

FullscreenWindow::FullscreenWindow()
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     background_image_("images/ui_fsmenu/background.png") {
	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.width, message.height);
		   layout();
		});
	set_frame_image(FullscreenWindow::Frames::kCornerTopLeft, "images/ui_fsmenu/top_left.png");
	set_frame_image(FullscreenWindow::Frames::kCornerTopRight, "images/ui_fsmenu/top_right.png");
	set_frame_image(FullscreenWindow::Frames::kCornerBottomLeft, "images/ui_fsmenu/bottom_left.png");
	set_frame_image(
	   FullscreenWindow::Frames::kCornerBottomRight, "images/ui_fsmenu/bottom_right.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeLeftTile, "images/ui_fsmenu/left.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeRightTile, "images/ui_fsmenu/right.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeTopTile, "images/ui_fsmenu/top.png");
	set_frame_image(FullscreenWindow::Frames::kEdgeBottomTile, "images/ui_fsmenu/bottom.png");
	add_overlay_image("images/ui_fsmenu/center.png", UI::Align::kCenter);
}

FullscreenWindow::~FullscreenWindow() {
	overlays_.clear();
	frame_overlays_.clear();
}

void FullscreenWindow::add_overlay_image(const std::string& filename, UI::Align align) {
	overlays_.push_back(std::make_pair(g_gr->images().get(filename), align));
}

void FullscreenWindow::clear_overlays() {
	overlays_.clear();
}

void FullscreenWindow::set_frame_image(FullscreenWindow::Frames id, const std::string& filename) {
	frame_overlays_.insert(std::make_pair(id, g_gr->images().get(filename)));
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
	dst.tile(Recti(0, 0, get_w(), get_h()), g_gr->images().get(background_image_), Vector2i(0, 0));

	// Optional overlays
	for (const auto& overlay : overlays_) {
		blit_image(dst, overlay.first, overlay.second);
	}

	// Frame edges
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeLeftTile),
				UI::Align::kTopLeft, kVertical);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeRightTile),
				UI::Align::kTopRight, kVertical);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeTopTile),
				UI::Align::kTopLeft, kHorizontal);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kEdgeBottomTile),
	           UI::Align::kBottomLeft, kHorizontal);

	// Frame corners
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerTopLeft),     UI::Align::kTopLeft);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerTopRight),    UI::Align::kTopRight);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerBottomLeft),  UI::Align::kBottomLeft);
	blit_image(dst, get_frame_image(FullscreenWindow::Frames::kCornerBottomRight), UI::Align::kBottomRight);
}

void FullscreenWindow::blit_image(RenderTarget& dst,
                                  const Image* image,
                                  UI::Align align,
                                  Tiling tiling) {
	if (image) {
		int x = 0;
		int y = 0;
		// Check HAlign
		if (align & UI::HAlign::kRight) {
			x = get_w() - image->width();
		} else if (align & UI::HAlign::kHCenter) {
			x = (get_w() - image->width()) / 2;
		}
		// Check VAlign
		if (align & UI::VAlign::kBottom) {
			y = get_h() - image->height();
		} else if (align & UI::VAlign::kVCenter) {
			y = (get_h() - image->height()) / 2;
		}

		if (tiling != kNone) {
			const int w = (tiling == kVertical)   ? image->width() : get_w();
			const int h = (tiling == kHorizontal) ? image->height() : get_h();
			dst.tile(Recti(x, y, w, h), image, Vector2i(0, 0));
		} else {
			dst.blit(Vector2f(x, y), image);
		}
	}
}

int FullscreenWindow::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

int FullscreenWindow::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

}  // namespace UI
