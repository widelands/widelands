/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include <iostream>
#include <typeinfo>

#ifdef _MSC_VER
// Needed to resolve entry point
#include <SDL.h>
#else
#include <unistd.h>
#endif

#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "wlapplication.h"
#include "wlapplication_messages.h"

/**
 * Cross-platform entry point for SDL applications.
 */
int main(int argc, char* argv[]) {
	std::cout << "This is Widelands version " << build_ver_details() << std::endl;

	WLApplication* g_app = nullptr;
	try {
		g_app = WLApplication::get(argc, const_cast<char const**>(argv));
		// TODO(unknown): handle exceptions from the constructor
		g_app->run();

		delete g_app;

		return 0;
	} catch (const ParameterError& e) {
		//  handle wrong commandline parameters
		show_usage(build_ver_details(), e.level_);
		if (e.what()[0] != 0) {
			std::cerr << std::string(60, '=') << std::endl << std::endl << e.what() << std::endl;
		}
		delete g_app;

		return 0;
	}
#ifdef NDEBUG
	catch (const WException& e) {
		std::cerr << "\nCaught exception (of type '" << typeid(e).name()
		          << "') in outermost handler!\nThe exception said: " << e.what()
		          << "\n\nThis should not happen. Please file a bug report on version "
		          << build_ver_details() << ".\n"
		          << "and remember to specify your operating system.\n\n"
		          << std::flush;
		delete g_app;

		return 1;
	} catch (const std::exception& e) {
		std::cerr << "\nCaught exception (of type '" << typeid(e).name()
		          << "') in outermost handler!\nThe exception said: " << e.what()
		          << "\n\nThis should not happen. Please file a bug report on version "
		          << build_ver_details() << ".\n"
		          << "and remember to specify your operating system.\n\n"
		          << std::flush;
		delete g_app;

		return 1;
	}
#endif
}
