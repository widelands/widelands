/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#endif

#include "fileloc.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** class File_Locator
 *
 * searches for files in various searchdirs and subdirs.
 * Reports filenames when files are found.
 *
 * Is also able to list files in a convenient way
 */

/** File_Locator::File_Locator(void)
 *
 * Some inits. Nothing special
 *
 * Args: none
 * Returns: Nothing
 */
File_Locator::File_Locator(void) {
		  la=LA_SUCCESS;

		  for(uint i=0; i<MAX_DIRS; i++) dirs[i][0]='\0';
		  for(uint j=0; j<MAX_SUBDIRS; j++) subdirs[j][0]='\0';
		  retval[0]='\0';
		  def_writedir=0;

		  curdir=NULL;
		  ncurdir=-1;
		  nlisttype=-1;
		  ndirstrlen=0;
}

/** File_Locator::~File_Locator(void)
 *
 * Cleanups.
 *
 * Args: None
 * Returns: Nothing
 */
File_Locator::~File_Locator(void) {
}

/** void File_Locator::add_searchdir(const char* dir, const uint prio)
 *
 * This adds a dir to the directorys to search. the priority
 * is where to add the dir (so MAX_DIRS-1 is lowest, 0 is highest)
 *
 * Args:	dir	Directory to add
 * 		prio	Priority of the dir
 * Returns: Nothing
 */
void File_Locator::add_searchdir(const char* dir, const uint prio) {
		  if(prio>= MAX_DIRS) {
					 la=LA_TOOMUCH;
					 return;
		  }

		  struct stat st;
		  if(stat(dir, &st) == -1) {
					 la=LA_NOTEXISTING;
					 return;
		  }

		  if(!(S_IFDIR & st.st_mode)) {
					 la=LA_NOTALLOWED;
					 return;
		  }
		  strcpy(dirs[prio], dir);
		  int len=strlen(dirs[prio]);
		  if(dirs[prio][len-1]!='/' && dirs[prio][len-1] != '\\') {
					 dirs[prio][len]='/';
					 dirs[prio][len+1]='\0';
		  }
		  la=LA_SUCCESS;
}

/* File_Locator::Last_action File_Locator::get_state(void) const
 *
 * This functions returns the error code of the last action
 *
 * Args: None
 * Returns: Success information on last action
 */
File_Locator::Last_action File_Locator::get_state(void) const {
		  return la;
}

/** void File_Locator::register_subdir(const int id, const char* subd)
 *
 * This function registers a subdir with a certain type of
 * file type (for ex: #define TYPE_TEXT 0  ; register_subdir(TYPE_TEXT, "txts"))
 *
 * Args:	id 	id to identify with
 * 	   subd	subdir to identify with
 *
 *	Returns: nothing
 */
void File_Locator::register_subdir(const uint id, const char* subd) {
		  if(id>= MAX_SUBDIRS) {
					 la=LA_TOOMUCH;
					 return;
		  }

		  strcpy(subdirs[id], subd);
		  int len=strlen(subdirs[id]);
		  if(subdirs[id][len-1] != '/' && subdirs[id][len-1] != '\\') {
					 subdirs[id][len]='/';
					 subdirs[id][len+1]='\0';
		  }
		  la=LA_SUCCESS;
}

/** void File_Locator::locate_file(const char* file, const int type=-1)
 *
 * This file locates a file in the registered dirs
 *
 * Args:	file	filename to find
 * 		type	subdir id to use/ if -1 == don't use subdirs
 *
 * Returns: a pathname to the first valid file found
 */
const char* File_Locator::locate_file(const char* file, const int type) {
		  if(type >= (int) MAX_SUBDIRS) {
					 la=LA_NOTALLOWED;
					 return NULL;
		  }

		  la=LA_NOTEXISTING;
		  
		  struct stat st;

		  for(uint n=0; n<MAX_DIRS; n++) {
					 if(dirs[n][0]=='\0') continue;

					 strcpy(retval, dirs[n]);
					 if(type != -1) strcat(retval, subdirs[type]);
					 strcat(retval, file);

					 // Stat this file!
					 if(stat(retval, &st) == -1) {
								// Not Found. Next one
								continue;
					 }

					 // Found
					 la=LA_SUCCESS;
					 break;
		  }
					 
		  if(la==LA_SUCCESS) {
					 return retval;
		  }
		  
		  // not found
		  return NULL;
}

/* const char* File_Locator::get_new_filename(const char* file, const int type=-1)
 *
 * This functions returns a valid filename in the default write dir
 *
 * Args: file	Filename to create
 * 		type	file type to create
 * Returns: 	Path to complete filename
 */
const char* File_Locator::get_new_filename(const char* file, const int type) {
		  if(type >= (int) MAX_SUBDIRS) {
					 la=LA_NOTALLOWED;
					 return NULL;
		  }
		  
		  la=LA_NOTALLOWED;

		  for(uint n=def_writedir; n<MAX_DIRS; n++) {
					 if(dirs[n][0]=='\0') continue;
								
					 strcpy(retval, dirs[n]);
					 if(type != -1) {
								strcat(retval, subdirs[type]);
								// Create the subdir, if it doesn't exists already
#ifndef WIN32
								mkdir(retval, 0x1FF);
#else
								mkdir(retval);
#endif
					 }
					 strcat(retval, file);

					 la=LA_SUCCESS;
					 break;
		  }
		  
		  if(la==LA_SUCCESS) {
					 return retval;
		  }

		  // Some error
		  return NULL;
}
					 
/* void File_Locator::set_def_writedir(const int id) 
 *
 * This functions sets the default write dir by the priority of
 * the registered searchdirs.
 *
 * Args: id		Directory of this priority should be used as default write dir
 * Returns: Nothing
 */
void File_Locator::set_def_writedir(const uint id) {
		  if(id >= (int) MAX_DIRS) {
					 la=LA_NOTALLOWED;
					 return;
		  }

		  if(dirs[id][0] == '\0') {
					 la=LA_NOTSET;
					 return;
		  }

		  def_writedir=id;
		  la=LA_SUCCESS;
}

/* void File_Locator::init_filelisting( const int type, const uchar postfix[5] );
 *
 * This functions inits the filelisting. it registeres the prefix and opens the first dir
 *
 * Args: type 		Which kinds of file to list
 * 		postfix	With what should the files end
 * Returns: Nothing
 */
void File_Locator::init_filelisting( const int type, const char postfix[5] ) {
		  if(type >= (int) MAX_SUBDIRS) {
					 la=LA_NOTALLOWED;
					 return;
		  }
		  
		  la=LA_NOTALLOWED;
		  
		  nlisttype=type;
		  ncurdir=0;

		  if(ncurdir<(int) MAX_DIRS) 
					 open_next_dir();

		  if(!curdir) {
					 la=LA_NOMOREFILES;
					 return;
		  }

		  if(postfix[0]!='\0') {
					 strncpy(suf, postfix, 5);
		  } else {
					 suf[0]='\0';
		  }

		  la=LA_SUCCESS;

		  return;
}

/** void File_Locator::open_next_dir(void) 
 *
 * PRIVATE FUNCTION
 *
 * this functions trys to open the next valid dir
 *
 * Args:	none
 * Returns: Nothing
 */
void File_Locator::open_next_dir(void)  {
		  if(ncurdir<0) return;

		  if(curdir) closedir(curdir);
		  curdir=NULL;

		  retval[0]='\0';

		  if (ncurdir == MAX_DIRS)
			  return;
		  
		  for(; ncurdir<(int)MAX_DIRS; ncurdir++) {
					 if(dirs[ncurdir][0]=='\0') continue;
								
					 strcpy(retval, dirs[ncurdir]);
					 if(nlisttype != -1) strcat(retval, subdirs[nlisttype]);
					 ncurdir++;

					 break;
		  }

		  // Try to open the dir
		  curdir=opendir(retval);
		  if(!curdir && ncurdir<(int)MAX_DIRS) {
					 // dir is invalid, but there are others. try them
					 open_next_dir();
					 return;
		  }
		  
		  ndirstrlen=strlen(retval);
}

/** void File_Locator::end_filelisting(void) 
 *
 * This ends the file listing process, does cleanup
 *
 * Args: none
 * Returns: noting
 */
void File_Locator::end_filelisting(void) {
		  if(curdir) closedir(curdir);
		  ncurdir=-1;
		  nlisttype=-1;
		  ndirstrlen=0;
		  
		  la=LA_SUCCESS;
}

/** const char* File_Locator::get_next_file(void);
 *
 * This functions returns the next file in the file listing
 * process
 *
 * Args: None
 * Returns: The next file
 */
const char* File_Locator::get_next_file(void) {
		  if(la==LA_NOMOREFILES) return NULL;
		  if(ncurdir<0) {
					 la=LA_NOTALLOWED;
					 return NULL;
		  }

		 
		  assert(curdir);
		  
		  struct dirent *file;

		  // reset the retval to just contain the dir
		  retval[ndirstrlen]='\0';
		  
		  file=readdir(curdir);
		  if(!file) {
					 // Last file in dir
					 open_next_dir();
					 if(!curdir) {
								// last file to list is done
								// return NULL, set la accordingly
								la=LA_NOMOREFILES;
								return NULL;
					 }
					 return get_next_file();
		  }

		  // strcat the file and return it
		  strcat(retval, file->d_name);


		  // Make sure the prefix is valid
		  /*if(suf[0]!='\0' && strlen(retval)>strlen(suf)) {
					 uint n=0;
					 for(n=strlen(suf)-1; n>0; --n) {
								if(suf[strlen(suf)-n]!=retval[strlen(retval)-n]) break;
					 }
					 if(n) return get_next_file();
		  }*/
		  if (suf[0])
		  {
			  int rlen = strlen(retval);
			  int slen = strlen(suf);
			  if (rlen < slen || strcasecmp(retval + rlen - slen, suf))
				  return get_next_file();
		  }
		  
		  la=LA_SUCCESS;
		  
		  return retval;
}
