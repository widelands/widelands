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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include <SDL_main.h>
#include <config.h>
#ifndef _WIN32
#include <fcntl.h>
#include <syslog.h>
#endif
#include <unistd.h>

#include "build_info.h"
#include "wexception.h"
#include "wlapplication.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

/**
 * Cross-platform entry point for SDL applications.
 */
int main(int argc, char * argv[])
{

#ifndef _WIN32
	// if Widelands is called as dedicated server, Widelands should be forked and started as daemon
	bool dedicated = false;
	bool daemon    = false;

	for (int i = 1; i < argc && !(daemon && dedicated); ++i) {
		std::string opt = argv[i];

		// At least a size of 8 is needed for --daemon, --dedicated is even longer
		if (opt.size() < 8)
			continue;

		if (opt == "--version") {
			cout << "Widelands " << build_id() << '(' << build_type() << ')' << "\n";
			return 0;
		}

		std::string::size_type const pos = opt.find('=');
		if (pos == std::string::npos) { //  if no equals sign found
			if (opt == "--daemon")
				daemon    = true;
		} else {
			opt.erase(pos, opt.size() - pos);
			if (opt == "--dedicated")
				dedicated = true;
		}
	}
	if (daemon && dedicated) {
		pid_t pid;
		if ((pid = fork()) < 0) {
			perror("fork() failed");
			exit(2);
		}
		if (pid == 0) {
			setsid();

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			open("/dev/null", O_RDWR);
			dup(STDIN_FILENO);
			dup(STDIN_FILENO);
			// from now on, it's a daemon
			openlog("FREELINE", LOG_PID, LOG_DAEMON);
		} else {
			printf("Child has PID %i.\n", pid);
			return 0;
		}
	}
#endif

	WLApplication * g_app = nullptr;
	try {
		g_app = WLApplication::get(argc, const_cast<char const * *>(argv));
		//TODO: handle exceptions from the constructor
		g_app->run();

		delete g_app;

		return 0;
	} catch (const Parameter_error & e) {
		//  handle wrong commandline parameters
		cerr<<endl<<e.what()<<endl<<endl;
		WLApplication::show_usage();
		delete g_app;

		return 0;
	}
#ifdef NDEBUG
	catch (const _wexception & e) {
		cerr
			<< "\nCaught exception (of type '" << typeid(e).name()
			<< "') in outermost handler!\nThe exception said: " << e.what()
			<< "\n\nThis should not happen. Please file a bug report on version "
			<< build_id() << '(' << build_type() << ')' << ".\n"
			<< "and remember to specify your operating system.\n\n" << flush;
		delete g_app;

		return 1;
	} catch (const std::exception & e) {
		cerr
			<< "\nCaught exception (of type '" << typeid(e).name()
			<< "') in outermost handler!\nThe exception said: " << e.what()
			<< "\n\nThis should not happen. Please file a bug report on version "
			<< build_id() << '(' << build_type() << ')' <<".\n"
			<< "and remember to specify your operating system.\n\n" << flush;
		delete g_app;

		return 1;
	}
#endif
}
