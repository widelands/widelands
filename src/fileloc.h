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

#ifndef __S__FILE_LOCATOR
#define __S__FILE_LOCATOR

#include <dirent.h>

#include "singleton.h"

/** class File_Locator 
 *
 * This class is responsible for locating files in several directorys
 * and to list them in a easy to use manner.
 */
class File_Locator : public Singleton<File_Locator> {
		  // Forbidden functions
		  File_Locator(const File_Locator&);
		  File_Locator& operator=(const File_Locator&);

		  public:
#ifdef MAX_PATH
					 static const unsigned int MAX_PATHL=MAX_PATH;
#else 
					 static const unsigned int MAX_PATHL=256;
#endif
					 static const unsigned int MAX_DIRS=5; 
					 static const unsigned int MAX_SUBDIRL=20;
					 static const unsigned int MAX_SUBDIRS=10;
		  
#ifdef WINDOWS
					 static const unsigned char SEP[]="\\";
					 static const unsigned char CSEP='\\';
#else
					 static const unsigned char SEP[]="/";
					 static const unsigned char CSEP='/';
#endif

					 enum Last_action {
								LA_SUCCESS,
								LA_NOTEXISTING,
								LA_NOTALLOWED,
								LA_TOOMUCH,
								LA_NOTSET,
								LA_NOMOREFILES
					 };
					 
					 File_Locator(void);
					 ~File_Locator(void);

					 void  add_searchdir(const char*, const unsigned int);
					 const char* locate_file(const char*, const int=-1);
					 Last_action get_state(void) const;
					 void register_subdir(const unsigned int, const char*);
					
					 const char* get_new_filename(const char*, const int=-1);
					 void set_def_writedir(const unsigned int id); 

					 // Listings of all available files
					 void init_filelisting( const int, const char[5] );
					 const char* get_next_file(void);
					 void end_filelisting(void);
					 
		  private:
					 char dirs[MAX_DIRS][MAX_PATHL];
					 char subdirs[MAX_SUBDIRL][MAX_SUBDIRS];
					 char retval[MAX_PATHL+MAX_SUBDIRL];
					 char suf[5];
					 int def_writedir;
					 Last_action la;
					 
					 DIR* curdir;
					 int ncurdir;
					 int nlisttype;
					 int ndirstrlen;

					 void open_next_dir(void);

};

#define g_fileloc File_Locator::get_singleton()

#define TYPE_TEXT 0
#define TYPE_PIC  1
#define TYPE_BOB  2
#define TYPE_TRIBE 3
#define TYPE_WORLD 4
#define TYPE_CAMPAIGN 5
#define TYPE_MAP 6
#define TYPE_MUSIC 7
#define TYPE_EFFECT 8

#endif /* __S__FILE_LOCATOR */
