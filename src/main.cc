/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <iostream>
#include <stdexcept>
#include "wlapplication.h"

using std::cerr;
using std::endl;
using std::flush;

/**
 * Cross-platform entry point for SDL applications.
 */
extern "C" 
int main(int argc, char* argv[])
{
	try {
		WLApplication * const g_app=WLApplication::get(argc, const_cast<const char**>(argv));
		//TODO: handle exceptions from the constructor

		g_app->run();

		return 0;
	}
	catch(...) {
		cerr<<"Caught unknown exception in outermost handler!"<<endl<<
		"This should not happen"<<endl<<endl<<
		"Please file a bug report."<<endl<<
		flush;
	}
}
