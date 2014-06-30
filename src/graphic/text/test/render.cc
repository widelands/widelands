/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include "graphic/text/test/render.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "base/log.h" // NOCOM(#sirver): remove again
#include "graphic/image_cache.h"
#include "graphic/image_loader_impl.h"
#include "graphic/render/sdl_helper.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/surface_cache.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/sdl_ttf_font.h"
#include "graphic/text/test/paths.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "third_party/lodepng/lodepng.h"

// NOCOM(#sirver): delet lodepng.
using namespace std;

StandaloneRenderer::StandaloneRenderer() {
	g_fs = new LayeredFileSystem();
	g_fs->AddFileSystem(FileSystem::Create(RICHTEXT_DATA_DIR));
	g_fs->AddFileSystem(FileSystem::Create(WIDELANDS_DATA_DIR));

	image_loader_.reset(new ImageLoaderImpl());
	surface_cache_.reset(create_surface_cache(500 << 20));  // 500 MB
	image_cache_.reset(create_image_cache(image_loader_.get(), surface_cache_.get()));
	renderer_.reset(RT::setup_renderer(
	   image_cache_.get(), surface_cache_.get(), RT::ttf_fontloader_from_filesystem(g_fs)));
}

StandaloneRenderer::~StandaloneRenderer() {
	delete g_fs;
	g_fs = nullptr;
}

Surface* StandaloneRenderer::render(const std::string& text,
                                            uint16_t w,
                                            const RT::TagSet& tagset) {
	return renderer_->render(text, w, tagset);
}

RT::IRefMap* StandaloneRenderer::make_reference_map(
   const std::string& text, uint16_t w, const RT::TagSet& tagset) {
	return renderer_->make_reference_map(text, w, tagset);
}

IImageLoader* StandaloneRenderer::image_loader() {
	return image_loader_.get();
}

StandaloneRenderer* setup_standalone_renderer() {
	return new StandaloneRenderer();
}
