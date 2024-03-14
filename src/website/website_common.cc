/*
 * Copyright (C) 2018-2024 by the Widelands Development Team
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
 *
 */

#include "website/website_common.h"

#include <SDL.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/multithreading.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	set_initializer_thread();

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		// We sometimes run into a missing video driver in our CI environment, so we exit 0 to prevent
		// too frequent failures
		log_err("Failed to initialize SDL, no valid video driver: %s", SDL_GetError());
		exit(2);
	}

	g_fs = new LayeredFileSystem();
	FileSystem& fs = FileSystem::create(INSTALL_DATADIR);
	g_fs->add_file_system(&fs);

	i18n::set_localedir(fs.canonicalize_name("i18n/translations"));
	i18n::init_locale();
	i18n::set_locale("en");
	i18n::grab_textdomain("widelands", i18n::get_localedir());

	// We don't really need graphics here, but we will get error messages
	// when they aren't initialized
	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false, false);
}

// Cleanup before program end
void cleanup() {
	if (g_gr != nullptr) {
		delete g_gr;
		g_gr = nullptr;
	}

	i18n::release_textdomain();

	if (g_fs != nullptr) {
		delete g_fs;
		g_fs = nullptr;
	}

	SDL_Quit();
}
