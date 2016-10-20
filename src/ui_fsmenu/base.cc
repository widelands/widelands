/*
 * Copyright (C) 2002, 2007-2010 by the Widelands Development Team
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

#include "ui_fsmenu/base.h"

#include <cstdio>

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "io/filesystem/filesystem.h"
#include "wlapplication.h"

/*
==============================================================================

FullscreenMenuBase

==============================================================================
*/

/**
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
FullscreenMenuBase::FullscreenMenuBase()
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     background_image_("images/ui_fsmenu/ui_fsmenu.png") {
	set_frame_image(FullscreenMenuBase::Frames::kCornerTopLeft, "images/ui_fsmenu/top_left.png");
	set_frame_image(FullscreenMenuBase::Frames::kCornerTopRight, "images/ui_fsmenu/top_right.png");
	set_frame_image(
	   FullscreenMenuBase::Frames::kCornerBottomLeft, "images/ui_fsmenu/bottom_left.png");
	set_frame_image(
	   FullscreenMenuBase::Frames::kCornerBottomRight, "images/ui_fsmenu/bottom_right.png");
	set_frame_image(FullscreenMenuBase::Frames::kEdgeLeftTile, "images/ui_fsmenu/left.png");
	set_frame_image(FullscreenMenuBase::Frames::kEdgeRightTile, "images/ui_fsmenu/right.png");
	set_frame_image(FullscreenMenuBase::Frames::kEdgeTopTile, "images/ui_fsmenu/top.png");
	set_frame_image(FullscreenMenuBase::Frames::kEdgeBottomTile, "images/ui_fsmenu/bottom.png");
}

FullscreenMenuBase::~FullscreenMenuBase() {
	overlays_.clear();
	frame_overlays_.clear();
}

void FullscreenMenuBase::add_overlay_image(const std::string& filename, UI::Align align) {
	overlays_.push_back(std::make_pair(g_gr->images().get(filename), align));
}

void FullscreenMenuBase::set_frame_image(FullscreenMenuBase::Frames id,
                                         const std::__cxx11::string& filename) {
	frame_overlays_.insert(std::make_pair(id, g_gr->images().get(filename)));
}

const Image* FullscreenMenuBase::get_frame_image(FullscreenMenuBase::Frames id) const {
	if (frame_overlays_.count(id) == 1) {
		return frame_overlays_.find(id)->second;
	} else {
		return nullptr;
	}
}

/**
 * Draw the background / splash screen
*/
void FullscreenMenuBase::draw(RenderTarget& dst) {
	// Overall background
	dst.tile(
	   Rect(Point(0, 0), get_w(), get_h()), g_gr->images().get(background_image_), Point(0, 0));

	// Optional overlays
	for (const auto& overlay : overlays_) {
		blit_image(dst, overlay.first, overlay.second);
	}

	// Frame edges
	blit_image(dst, get_frame_image(FullscreenMenuBase::Frames::kEdgeLeftTile), UI::Align::kTopLeft,
	           UI::Align::kVertical);
	blit_image(dst, get_frame_image(FullscreenMenuBase::Frames::kEdgeRightTile),
	           UI::Align::kTopRight, UI::Align::kVertical);
	blit_image(dst, get_frame_image(FullscreenMenuBase::Frames::kEdgeTopTile), UI::Align::kTopLeft,
	           UI::Align::kHorizontal);
	blit_image(dst, get_frame_image(FullscreenMenuBase::Frames::kEdgeBottomTile),
	           UI::Align::kBottomLeft, UI::Align::kHorizontal);

	// Frame corners
	blit_image(
	   dst, get_frame_image(FullscreenMenuBase::Frames::kCornerTopLeft), UI::Align::kTopLeft);
	blit_image(
	   dst, get_frame_image(FullscreenMenuBase::Frames::kCornerTopRight), UI::Align::kTopRight);
	blit_image(
	   dst, get_frame_image(FullscreenMenuBase::Frames::kCornerBottomLeft), UI::Align::kBottomLeft);
	blit_image(dst, get_frame_image(FullscreenMenuBase::Frames::kCornerBottomRight),
	           UI::Align::kBottomRight);
}

void FullscreenMenuBase::blit_image(RenderTarget& dst,
                                    const Image* image,
                                    UI::Align align,
                                    UI::Align tiling) {
	if (image) {
		int x = 0;
		int y = 0;
		if (static_cast<int>(align & UI::Align::kRight)) {
			x = get_w() - image->width();
		} else if (static_cast<int>(align & UI::Align::kHCenter)) {
			x = (get_w() - image->width()) / 2;
		}
		if (static_cast<int>(align & UI::Align::kBottom)) {
			y = get_h() - image->height();
		} else if (static_cast<int>(align & UI::Align::kVCenter)) {
			y = (get_h() - image->height()) / 2;
		}
		if (static_cast<int>(tiling & (UI::Align::kVertical | UI::Align::kHorizontal))) {
			const int w = (static_cast<int>(tiling & UI::Align::kVertical)) ? image->width() : get_w();
			const int h =
			   (static_cast<int>(tiling & UI::Align::kHorizontal)) ? image->height() : get_h();
			dst.tile(Rect(Point(x, y), w, h), image, Point(0, 0));
		} else {
			dst.blit(Point(x, y), image);
		}
	}
}

int FullscreenMenuBase::fs_small() {
	return UI_FONT_SIZE_SMALL * get_h() / 600;
}

int FullscreenMenuBase::fs_big() {
	return UI_FONT_SIZE_BIG * get_h() / 600;
}

bool FullscreenMenuBase::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			clicked_back();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::Panel::handle_key(down, code);
}

void FullscreenMenuBase::clicked_back() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
}
void FullscreenMenuBase::clicked_ok() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
}
