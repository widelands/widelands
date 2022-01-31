/*
 * Copyright 2010-2011 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "graphic/screen.h"

#include <memory>

#include "base/wexception.h"
#include "graphic/gl/utils.h"
#include "graphic/render_queue.h"
#include "graphic/texture.h"

Screen::Screen(int w, int h) : w_(w), h_(h) {
}

int Screen::width() const {
	return w_;
}

int Screen::height() const {
	return h_;
}

std::unique_ptr<Texture> Screen::to_texture() const {
	std::unique_ptr<uint8_t[]> pixels(new uint8_t[w_ * h_ * 4]);
	glReadPixels(0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

	Gl::swap_rows(w_, h_, w_ * 4, 4, pixels.get());

	// Ownership of pixels is not taken here. But the Texture() transfers it to
	// the GPU, frees the SDL surface and after that we are free to free
	// 'pixels'.
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
	   pixels.get(), w_, h_, 32, w_ * 4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	return std::unique_ptr<Texture>(new Texture(surface));
}

void Screen::do_blit(const Rectf& dst_rect,
                     const BlitData& texture,
                     float opacity,
                     BlendMode blend_mode) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kBlit;
	i.blend_mode = blend_mode;
	i.blit_arguments.blend = RGBAColor(0, 0, 0, 255 * opacity);
	i.blit_arguments.destination_rect = dst_rect;
	i.blit_arguments.mask.texture_id = 0;
	i.blit_arguments.mode = BlitMode::kDirect;
	i.blit_arguments.texture = texture;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_blit_blended(const Rectf& dst_rect,
                             const BlitData& texture,
                             const BlitData& mask,
                             const RGBColor& blend) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kBlit;
	i.blend_mode = BlendMode::UseAlpha;
	i.blit_arguments.blend = blend;
	i.blit_arguments.destination_rect = dst_rect;
	i.blit_arguments.mask = mask;
	i.blit_arguments.mode = BlitMode::kBlendedWithMask;
	i.blit_arguments.texture = texture;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_blit_monochrome(const Rectf& dst_rect,
                                const BlitData& texture,
                                const RGBAColor& blend) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kBlit;
	i.blend_mode = BlendMode::UseAlpha;
	i.blit_arguments.blend = blend;
	i.blit_arguments.destination_rect = dst_rect;
	i.blit_arguments.mask.texture_id = 0;
	i.blit_arguments.mode = BlitMode::kMonochrome;
	i.blit_arguments.texture = texture;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_draw_line_strip(std::vector<DrawLineProgram::PerVertexData> vertices) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kLine;
	i.blend_mode = BlendMode::UseAlpha;
	i.line_arguments.vertices = std::move(vertices);
	RenderQueue::instance().enqueue(i);
}

void Screen::do_fill_rect(const Rectf& dst_rect, const RGBAColor& color, BlendMode blend_mode) {
	RenderQueue::Item i;
	i.blend_mode = blend_mode;
	i.program_id = RenderQueue::Program::kRect;
	i.rect_arguments.color = color;
	i.rect_arguments.destination_rect = dst_rect;
	RenderQueue::instance().enqueue(i);
}
