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

#ifndef __S__MYFILE_H
#define __S__MYFILE_H

#include <stdio.h>
#include "mytypes.h"

/** class File
 *
 * this works for binary files
 * 
 */
class File {
		  public:
					 enum State {
								NOTHING, 
								OPEN,
								WRITES,
								READS,
								CLOSE,
								END_OF_FILE
					 };

					 enum For {
								READ,
								WRITE
					 };

					 // Constructor, Deconstructor
					 File(void);
					 virtual ~File(void);

					 // File specific functions
					 virtual void open(const char*,  For)=0;
					 virtual void close()=0;
					 virtual int read(void*, int)=0;
					 virtual int write(const void*, int)=0;
					 
					 // Common functions
					 State get_state();

		  protected:
					 void set_state(State);

		  private:
					 // This functions can't be called with files
					 File& operator=(const File&);
					 File(const File&);

					 State st;
}; 

/** A Ascii file is a file containing nothing but ascii letters
 *
 * This class makes sure that ascii files are treated the same under 
 * Windows and Unix
 *
 * A Ascii File can also be read linewise
 */
class Ascii_file : public File {
		  public:
					 Ascii_file();
					 ~Ascii_file();
					 
					 int read_line(char*, uint);
					 
					 void open(const char*, For);
					 void close(void);
					 int read(void*, int);
					 int write(const void*, int);
	
		  private:
					 FILE* f;
};

/** class Binary_file
 *
 * Binary files are just normal files. nothing special about them
 */
class Binary_file : public File {
		  public:
					 Binary_file();
					 ~Binary_file();
					 
					 void open(const char*, For);
					 void close(void);
					 int read(void*, int);
					 int write(const void*, int);

		  private:
					 FILE* f;
};


#endif /* __S__MYFILE_H */
