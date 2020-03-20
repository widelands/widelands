/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_FULLSCREEN_WINDOW_H
#define WL_UI_BASIC_FULLSCREEN_WINDOW_H

#include <memory>

#include "graphic/align.h"
#include "graphic/graphic.h"
#include "ui_basic/panel.h"

namespace UI {

/**
 * This class is the base class for a fullscreen window.
 * A fullscreen window takes up the full screen; it has the size
 * g_gr->get_xres(), g_gr->get_yres()
 */
class FullscreenWindow : public UI::Panel {
public:
	/// Access keys for frame overlay images
	enum class Frames {
		kCornerTopLeft,
		kCornerTopRight,
		kCornerBottomLeft,
		kCornerBottomRight,
		kEdgeLeftTile,
		kEdgeRightTile,
		kEdgeTopTile,
		kEdgeBottomTile,
		kCenter
	};
	struct FramesHash {
		template <typename T> int operator()(T t) const {
			return static_cast<int>(t);
		}
	};

	struct Alignment {
		Alignment(UI::Align h, UI::Align v) : halign(h), valign(v) {
		}
		Alignment() : Alignment(UI::Align::kLeft, UI::Align::kTop) {
		}

		UI::Align halign;
		UI::Align valign;
	};

	/// A full screen main menu outside of the game/editor itself.
	FullscreenWindow();
	~FullscreenWindow() override;

	/// \return the font size scale factor for fitting texts to current resolution
	float scale_factor() const;

protected:
	void draw(RenderTarget&) override;

	/// Sets the image for the given frame position.
	void set_frame_image(FullscreenWindow::Frames id, const std::string& filename);
	/// Add an overlay images to be blitted according to 'align'.
	void add_overlay_image(const std::string& filename, Alignment align);
	/// Remove all overlay images
	void clear_overlays();

private:
	/// Returns the image for the given frame position.
	const Image* get_frame_image(FullscreenWindow::Frames id) const;

	enum class Tiling { kNone, kHorizontal, kVertical };

	/// Calculate the rect that the image will be blitted to.
	static Recti calculate_rect(const Image* image, Alignment align, Tiling tiling);

	/**
	 * Blit an image according to the given 'align'.
	 * If 'tiling' is set to 'UI::Align::kVertical' or 'UI::Align::kHorizontal', the image will be
	 * tiled. If the image is bigger than the panel, it will get scaled down.
	 */
	void blit_image(RenderTarget& dst,
	                const Image* image,
	                Alignment align,
	                Tiling tiling = Tiling::kNone);

	const std::string background_image_;

	/// These overlay images will be blitted in the order they were added and according to the given
	/// align.
	struct Overlay {
		Overlay(const Image* init_image, const FullscreenWindow::Alignment& init_align)
		   : image(init_image), align(init_align) {
		}
		const Image* image;
		const FullscreenWindow::Alignment align;
	};
	std::vector<std::unique_ptr<const Overlay>> overlays_;

	/// Images for the edges and the center. Except for the center one, they will be blitted on top
	/// of the overlays_.
	std::unordered_map<FullscreenWindow::Frames, const Image*, FullscreenWindow::FramesHash>
	   frame_overlays_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};
}  // namespace UI
#endif  // end of include guard: WL_UI_BASIC_FULLSCREEN_WINDOW_H
