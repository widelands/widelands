/*
 * Copyright (C) 2002 by Holger Rapp 
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
#include "myfile.h"

/** class File
 *
 * This implements the basic for a file
 *
 * Some things inherit from this (ascii_file) 
 */


/** File::File()
 *
 * Default constructor. Puts things in a known state
 *
 * Args: None
 * returns: Nothing
 */
File::File(void) {
		  st=CLOSE;
}

/** File::~File() 
 *
 * Default destructor. Closes the file. 
 * declared virtual for inheritance
 *
 * Args: None
 * returns: Nothing
 */
File::~File(void) {
}

/** File::get_state(void) 
 *
 * Gets the current state of the file.
 *
 * Args: none
 * Returns: current state of file
 */
File::State File::get_state(void) {
		  return st;
}


			
/** File::set_state(State nst)
 *
 * Protected function for childs to set the state of the 
 * File
 *
 * Args: nst		New state
 * 
 * Returns: nothing
 */
void File::set_state(State nst) {
		  st=nst;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/** class Binary_file 
 *
 * Inherits from File
 *
 * Defines functions for binary files
 */
		  
/** Binary_file::Binary_file()
 *
 * Simple inits
 *
 * Args: none
 * returns: Nothing
 */
Binary_file::Binary_file(void) {
		  f=NULL;
}

/** Binary_file::~Binary_file(void) 
 *
 * Default Destructor. Closes file
 */
Binary_file::~Binary_file(void) {
		  close();
}

/** Binary_file::open(char* file, For forw) 
 *
 * Opens a file. 
 *
 * Args: file   == path to open
 * 		forw	 == READ / WRITE
 *
 * returns: Nothing
 */
void Binary_file::open(const char* file, For forw) {
		  if(get_state()!=CLOSE) return ;

		  char mode[4];

		  if(forw == READ) strcpy(mode, "rb");
		  if(forw == WRITE) strcpy(mode, "wb");

		  f=fopen(file, mode);
		  if(f) set_state(OPEN);
}

/** Binary_file::close(void) 
 *
 * Closes the file if it's open. Does nothing on closed files
 *
 * Args: none
 * returns: Nothing
 */
void Binary_file::close(void) {
		  if(get_state()!=CLOSE) { 
					 fclose(f); 
					 f=NULL; 
					 set_state(CLOSE); 
		  }
}

/** Binary_file::read(void* buf, int size) 
 *
 * Reads some bytes from the file into the buf
 *
 * Args: buf   buffer to store bytes in
 * 		size	bytes to read
 *
 * Returns: bytes read or -1
 */
int Binary_file::read(void* buf, int size) {
		  if(get_state() != OPEN) return -1;

		  set_state(READS);
		  
		  int nread;
		  nread=fread((char*)buf, 1, size, f);

		  set_state(OPEN);

		  if(nread<size) set_state(END_OF_FILE);

		  return nread;
}

/** Binary_file::write(void* buf, int size) 
 *
 * Writes some bytes into the file from buf
 *
 * Args: buf	data to write into buffer
 * 		size	number of bytes to write
 *
 * Returns: bytes written or -1
 */
int Binary_file::write(const void* buf, int size) {
		  if(get_state() != OPEN) return -1;

		  set_state(WRITES);

		  int nwritten;

		  nwritten=fwrite((char*)buf, 1, size, f);

		  set_state(OPEN);

		  return nwritten;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









/** class Ascii_file
 *
 * Inherits from File
 * Needs to overwrite some functioncs, but has lot in common
 *
 */

/** Ascii_file::Ascii_file()
 *
 * Simple inits
 *
 * Args: none
 * returns: Nothing
 */
Ascii_file::Ascii_file(void) {
		  f=NULL;
}

/** Ascii_file::~Ascii_file(void) 
 *
 * Default Destructor. Closes file
 */
Ascii_file::~Ascii_file(void) {
		  close();
}

/** Ascii_file::open(char* file, For forw) 
 *
 * Opens a file. 
 *
 * Args: file   == path to open
 * 		forw	 == READ / WRITE
 *
 * returns: Nothing
 */
void Ascii_file::open(const char* file, For forw) {
		  if(get_state()!=CLOSE) return ;

		  char mode[4];

		  if(forw == READ) strcpy(mode, "r");
		  if(forw == WRITE) strcpy(mode, "w");

		  f=fopen(file, mode);
		  if(f) set_state(OPEN);
}

/** Ascii_file::close(void) 
 *
 * Closes the file if it's open. Does nothing on closed files
 *
 * Args: none
 * returns: Nothing
 */
void Ascii_file::close(void) {
		  if(get_state()!=CLOSE) { 
					 fclose(f); 
					 f=NULL; 
					 set_state(CLOSE); 
		  }
}

/** Ascii_file::read(void* buf, int size) 
 *
 * Reads some bytes from the file into the buf
 *
 * Args: buf   buffer to store bytes in
 * 		size	bytes to read
 *
 * Returns: bytes read or -1
 */
int Ascii_file::read(void* buf, int size) {
		  if(get_state() != OPEN) return -1;

		  set_state(READS);
		  
		  int nread;
		  nread=fread((char*)buf, 1, size, f);

		  set_state(OPEN);

		  if(nread<size) set_state(END_OF_FILE);

		  return nread;
}

/** Ascii_file::write(void* buf, int size) 
 *
 * Writes some bytes into the file from buf
 *
 * Args: buf	data to write into buffer
 * 		size	number of bytes to write
 *
 * Returns: bytes written or -1
 */
int Ascii_file::write(const void* buf, int size) {
		  if(get_state() != OPEN) return -1;

		  set_state(WRITES);

		  int nwritten;

		  nwritten=fwrite((char*)buf, 1, size, f);

		  set_state(OPEN);

		  return nwritten;
}

/** Ascii_file::read_line(char* buf, int size)
 *
 * Reads a line from the file (or max size bytes )
 *
 * Args: buf	buffer to save line in
 * 		size	max numbers of bytes to read
 *
 * Returns: bytes read or -1
 */
int Ascii_file::read_line(char* buf, uint size)
{
	if (get_state() != OPEN)
		return -1;

	set_state(READS);
		 
	char* ret;
	ret = fgets(buf, size, f);

	if(!ret)
	{
		set_state(END_OF_FILE);
		return -1;
	}

	uint i;
	for (i=0; i<size; i++)
		if (buf[i]=='\n')
		{
			if(i && buf[i-1]=='\r')
				buf[i-1]='\0';
			else
				buf[i] = '\0';
			break;
		}
		  
	set_state(OPEN);

	return i;
}
