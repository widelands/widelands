/*
 * Copyright (C) 2001 by Holger Rapp 
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

#include "os.h"
#include "options.h"
#include "errors.h"
#include "output.h"
#include "fileloc.h"
#include "input.h"
#include "cursor.h"
#include "intro.h"
#include "mainmenue.h"
#include "setup.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>

/** g_main function 
 * This is the OS Independant main function.
 * 
 * It makes sure, Options and Commandline is parsed,
 * initializes Graphics and Resource Handler
 *
 * return Exitcode of App
 */
int g_main(int argn, char** argc) {
		  
		  // Setup default searchpaths
		  setup_searchpaths();
		  
		  // Handle options
		  handle_options(argn, argc);

		  // run intro
		  run_intro();
		 
		  // run main_menue
		  main_menue();
		  
		  return RET_OK;
}
