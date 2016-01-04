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
	i.program_id = RenderQueue::Program::BLIT;
	i.blend_mode = blend_mode;
	i.destination_rect = dst_rect;
	i.vanilla_blit_arguments.texture = texture;
	i.vanilla_blit_arguments.opacity = opacity;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_blit_blended(const FloatRect& dst_rect,
                             const BlitData& texture,
                             const BlitData& mask,
                             const RGBColor& blend) {
	RenderQueue::Item i;
	i.destination_rect = dst_rect;
	i.program_id = RenderQueue::Program::BLIT_BLENDED;
	i.blend_mode = BlendMode::UseAlpha;
	i.blended_blit_arguments.texture = texture;
	i.blended_blit_arguments.mask = mask;
	i.blended_blit_arguments.blend = blend;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_blit_monochrome(const FloatRect& dst_rect,
                                const BlitData& texture,
                                const RGBAColor& blend) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::BLIT_MONOCHROME;
	i.blend_mode = BlendMode::UseAlpha;
	i.destination_rect = dst_rect;
	i.monochrome_blit_arguments.texture = texture;
	i.monochrome_blit_arguments.blend = blend;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_draw_line(const FloatPoint& start,
                          const FloatPoint& end,
                          const RGBColor& color,
								  const int line_width) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::LINE;
	i.blend_mode = BlendMode::Copy;
	i.destination_rect = FloatRect(start.x, start.y, end.x - start.x, end.y - start.y);
	i.line_arguments.color = color;
	i.line_arguments.line_width = line_width;
	RenderQueue::instance().enqueue(i);
}

void Screen::do_fill_rect(const FloatRect& dst_rect, const RGBAColor& color, BlendMode blend_mode) {
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::RECT;
	i.blend_mode = blend_mode;
	i.destination_rect = dst_rect;
	i.rect_arguments.color = color;
	RenderQueue::instance().enqueue(i);
}
