// Widelands server for GGZ
// Copyright (C) 2004 Josef Spillner <josef@ggzgamingzone.org>
// Copyright (C) 2009 The Widelands Development Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Widelands includes
#include "widelands_server.h"

// System includes
#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <ctime>

// Version information
#define WIDELANDS_SERVER_VERSION "0.3"

// Main function: parse arguments and start server
int main(int argc, char** argv)
{
	int option;
	struct option opt [] =
	{
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	int optindex;

	while((option = getopt_long(argc, argv, "hvg", opt, &optindex)) != EOF)
		switch(option)
		{
			case 'h':
				std::cout << "The GGZ Gaming Zone Widelands Server" << std::endl
					<< "Copyright (C) 2004 Josef Spillner <josef@ggzgamingzone.org>" << std::endl
					<< "Copyright (C) 2009 The Widelands Development Team" << std::endl
					<< "Published under GNU GPL v.2 conditions " << std::endl << std::endl
					<< "Options: " << std::endl
					<< "[-h | --help]    This help screen" << std::endl
					<< "[-v | --version] Version information" << std::endl;
				exit(EXIT_SUCCESS);
			case 'v':
				std::cout << "GGZ Widelands Server version " WIDELANDS_SERVER_VERSION << std::endl;
				exit(EXIT_SUCCESS);
		}

	srand(time(NULL));

	WidelandsServer *wls = new WidelandsServer();
	wls->connect(false);
	delete wls;

	return 0;
}

