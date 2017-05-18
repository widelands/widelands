/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_RENDERED_TEXT_H
#define WL_GRAPHIC_TEXT_RENDERED_TEXT_H

#include <memory>
#include <vector>

#include "base/rect.h"
#include "base/vector.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"

namespace UI {

/// A rectangle that contains blitting information for rendered text.
class RenderedRect {
public:
	/// Whether the RenderedRect's image should be blitted once or tiled
	enum class DrawMode { kBlit, kTile };

private:
	RenderedRect(const Recti& init_rect,
	             const Image* init_image,
	             bool visited,
	             const RGBColor& color,
	             bool is_background_color_set,
	             DrawMode init_mode);

public:
	/// RenderedRect will contain a background image that should be tiled
	RenderedRect(const Recti& init_rect, const Image* init_image);

	/// RenderedRect will contain a background color that should be tiled
	RenderedRect(const Recti& init_rect, const RGBColor& color);

	/// RenderedRect will contain a normal image
	RenderedRect(const Image* init_image);
	~RenderedRect() {
	}

	/// An image to be blitted. Can be nullptr.
	const Image* image() const;

	/// The x position of the rectangle
	int x() const;
	/// The y position of the rectangle
	int y() const;
	/// The width of the rectangle
	int width() const;
	/// The height of the rectangle
	int height() const;

	/// Change x and y position of the rectangle.
	void set_origin(const Vector2i& new_origin);

	/// Set that this rectangle was already visited by the font renderer. Needed by the font renderer
	/// for correct positioning.
	void set_visited();
	/// Whether this rectangle was already visited by the font renderer
	bool was_visited() const;

	/// Whether this rectangle contains a background color rather than an image
	bool has_background_color() const;
	/// This rectangle's background color
	const RGBColor& background_color() const;

	/// Whether the RenderedRect's image should be blitted once or tiled
	DrawMode mode() const;

private:
	Recti rect_;
	const Image* image_;  // Not owned
	bool visited_;
	const RGBColor background_color_;
	const bool is_background_color_set_;
	const DrawMode mode_;
};

struct RenderedText {
	/// RenderedRects that can be drawn on screen
	std::vector<std::unique_ptr<RenderedRect>> rects;

	/// The width occupied  by all rects in pixels.
	int width() const;
	/// The height occupied  by all rects in pixels.
	int height() const;

	enum class CropMode {
		kRenderTarget,  // The RenderTarget will handle the cropping
		kHorizontal     // The draw() method will handle horizontal cropping only
	};

	/// Draw the rects. 'position', 'region' and 'align' are used to control the overall drawing
	/// position and cropping
	/// For 'cropmode', use kRenderTarget if you wish the text to fill the whole RenderTarget, e.g.
	/// for scrolling panels. Use kHorizontal for horizontal croping in smaller elements, e.g. table
	/// cells.
	void draw(RenderTarget& dst,
	          const Vector2i& position,
	          const Recti& region,
	          UI::Align align = UI::Align::kLeft,
	          CropMode cropmode = CropMode::kRenderTarget) const;

	/// Draw the rects without cropping. 'position' and 'align' are used to control the overall
	/// drawing position
	void draw(RenderTarget& dst, const Vector2i& position, UI::Align align = UI::Align::kLeft) const;

	/// Blit everything into a single texture. Use this only for testing purposes.
	std::unique_ptr<Texture> as_texture() const;

private:
	/// Helper function for horizontal positioning & cropping
	void blit_cropped(RenderTarget& dst,
	                  int offset_x,
	                  const Vector2i& position,
	                  const Vector2i& blit_point,
	                  const RenderedRect& rect,
	                  const Recti& region,
	                  Align align) const;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_TEXT_RENDERED_TEXT_H
