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

#include "md5file.h"

//
//  class MD5File
//  
MD5_File::MD5_File(void) : sum() { }
MD5_File::~MD5_File(void) { }

// This can only be called once. after this, the checksum 
// is computet and won't be changed. you need to reopen another file
// if you need another checksum
ulong* MD5_File::get_chksum(void) {
   sum.finish_chksum();
   return sum.get_chksum();
}


//
// class MD5_Ascii_File
//
MD5_Ascii_file::MD5_Ascii_file(void) : Ascii_file(), MD5_File()  { }
MD5_Ascii_file::~MD5_Ascii_file(void) { }

int MD5_Ascii_file::read(void* buf, int size) {
   int retval;

   retval=Ascii_file::read(buf, size);
   sum.pass_data(buf, retval);

   return retval;
}

int MD5_Ascii_file::write(const void* buf, int size) {
   int retval;

   retval=Ascii_file::write(buf, size);
   sum.pass_data(buf, retval);

   return retval;
};



//
// class MD5_Binary_File
//
MD5_Binary_file::MD5_Binary_file(void) : Binary_file(), MD5_File()  { }
MD5_Binary_file::~MD5_Binary_file(void) { }

int MD5_Binary_file::read(void* buf, int size) {
   int retval;

   retval=Binary_file::read(buf, size);
   sum.pass_data(buf, retval);

   return retval;
}

int MD5_Binary_file::write(const void* buf, int size) {
   int retval;

   retval=Binary_file::write(buf, size);
   sum.pass_data(buf, retval);

   return retval;
};
