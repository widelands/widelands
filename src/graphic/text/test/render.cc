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

#include "graphic/image_cache.h"
#include "graphic/surface_cache.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/test/paths.h"
#include "io/filesystem/layered_filesystem.h"

StandaloneRenderer::StandaloneRenderer() {
	g_fs = new LayeredFileSystem();
	g_fs->AddFileSystem(FileSystem::Create(WIDELANDS_DATA_DIR));
	g_fs->AddFileSystem(FileSystem::Create(RICHTEXT_DATA_DIR));

	surface_cache_.reset(create_surface_cache(500 << 20));  // 500 MB
	image_cache_.reset(create_image_cache(surface_cache_.get()));
	renderer_.reset(RT::setup_renderer(image_cache_.get(), surface_cache_.get()));
}

StandaloneRenderer::~StandaloneRenderer() {
	delete g_fs;
	g_fs = nullptr;
}

RT::IRenderer* StandaloneRenderer::renderer() {
	return renderer_.get();
}
