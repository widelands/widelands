/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef RENDER_H
#define RENDER_H

#include <memory>

#include "graphic/text/rt_render.h"
#include "graphic/image_loader.h"

// A stand alone richtext renderer for tests and binaries.
class StandaloneRenderer : public RT::IRenderer {
	public:
		StandaloneRenderer();
		virtual ~StandaloneRenderer() override;

		// Ownership is maintained.
		IImageLoader* image_loader();

		// Implements RT::IRenderer.
	   virtual Surface*
	   render(const std::string& text, uint16_t w, const RT::TagSet& tagset = RT::TagSet()) override;
	   virtual RT::IRefMap* make_reference_map
			(const std::string& text, uint16_t w, const RT::TagSet & tagset = RT::TagSet()) override;


	private:
		std::unique_ptr<IImageLoader> image_loader_;
		std::unique_ptr<SurfaceCache> surface_cache_;
		std::unique_ptr<ImageCache> image_cache_;
		std::unique_ptr<RT::IRenderer> renderer_;
};

StandaloneRenderer* setup_standalone_renderer();

#endif /* end of include guard: RENDER_H */
