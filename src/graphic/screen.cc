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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "graphic/screen.h"

#include <algorithm>
#include <cassert>
#include <memory>

#include "base/wexception.h"
#include "graphic/gl/utils.h"
#include "graphic/render_queue.h"
#include "graphic/texture.h"

Screen::Screen(int w, int h) : m_w(w), m_h(h) {
}

int Screen::width() const {
	return m_w;
}

int Screen::height() const {
	return m_h;
}

std::unique_ptr<Texture> Screen::to_texture() const {
	std::unique_ptr<uint8_t[]> pixels(new uint8_t[m_w * m_h * 4]);
	glReadPixels(0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

	Gl::swap_rows(m_w, m_h, m_w * 4, 4, pixels.get());

	// Ownership of pixels is not taken here. But the Texture() transfers it to
	// the GPU, frees the SDL surface and after that we are free to free
	// 'pixels'.
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels.get(),
	                                                m_w,
	                                                m_h,
	                                                32,
	                                                m_w * 4,
	                                                0x000000ff,
	                                                0x0000ff00,
	                                                0x00ff0000,
	                                                0xff000000);

	return std::unique_ptr<Texture>(new Texture(surface));
}

void Screen::do_blit(const FloatRect& dst_rect,
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

void Screen::do_blit_blended(const FloatRect& dst_rect,
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

void Screen::do_blit_monochrome(const FloatRect& dst_rect,
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

void Screen::do_draw_line_strip(const std::vector<FloatPoint>& gl_points,
                                const RGBColor& color) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kLine;
	i.blend_mode = BlendMode::UseAlpha;
	i.line_arguments.points = gl_points;
	i.line_arguments.color = color;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_fill_rect(const FloatRect& dst_rect, const RGBAColor& color, BlendMode blend_mode) {
	RenderQueue::Item i;
	i.blend_mode = blend_mode;
	i.program_id = RenderQueue::Program::kRect;
	i.rect_arguments.color = color;
	i.rect_arguments.destination_rect = dst_rect;
	RenderQueue::instance().enqueue(i);
}
