/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/test/paths.h"
#include "graphic/text/texture_cache.h"
#include "io/filesystem/layered_filesystem.h"

StandaloneRenderer::StandaloneRenderer() {
	i18n::set_locale("en");
	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(WIDELANDS_DATA_DIR));
	g_fs->add_file_system(&FileSystem::create(RICHTEXT_DATA_DIR));
	UI::FontSets fontsets;

	texture_cache_.reset(new TextureCache(500 << 20));  // 500 MB
	image_cache_.reset(new ImageCache());
	renderer_.reset(new RT::Renderer(image_cache_.get(), texture_cache_.get(), fontsets));
}

StandaloneRenderer::~StandaloneRenderer() {
	delete g_fs;
	g_fs = nullptr;
}

RT::Renderer* StandaloneRenderer::renderer() {
	return renderer_.get();
}
