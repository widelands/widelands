/*
 * Copyright (C) 2002-2004, 2008-2009 by the Widelands Development Team
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

//config.h must be included first of all!
#include "build_info.h"

#include "wexception.h"
#include "wlapplication.h"

#include <config.h>

#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include <SDL_main.h>


using std::cerr;
using std::endl;
using std::flush;

/**
 * Cross-platform entry point for SDL applications.
 */
int main(int argc, char * argv[])
{
	WLApplication * g_app = 0;
	try {
		g_app = WLApplication::get(argc, const_cast<char const * *>(argv));
		//TODO: handle exceptions from the constructor
		g_app->run();

		delete g_app;

		return 0;
	} catch (Parameter_error const & e) {
		//  handle wrong commandline parameters
		cerr<<endl<<e.what()<<endl<<endl;
		WLApplication::show_usage();
		delete g_app;

		return 0;
	} catch (_wexception const & e) {
		cerr
			<< "\nCaught exception (of type '" << typeid(e).name()
			<< "') in outermost handler!\nThe exception said: " << e.what()
			<< "\n\nThis should not happen. Please file a bug report on version "
			<< build_id() << '(' << build_type() << ')' << ".\n"
			<< "and remember to specify your operating system.\n\n" << flush;
		delete g_app;

		return 1;
	} catch (std::exception const & e) {
		cerr
			<< "\nCaught exception (of type '" << typeid(e).name()
			<< "') in outermost handler!\nThe exception said: " << e.what()
			<< "\n\nThis should not happen. Please file a bug report on version "
			<< build_id() << '(' << build_type() << ')' <<".\n"
			<< "and remember to specify your operating system.\n\n" << flush;
		delete g_app;

		return 1;
	}
}
