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

#ifndef __S__MD5FILE_H
#define __S__MD5FILE_H

#include "myfile.h"
#include "md5.h"

/* This class simply extents the File class to 
 * allow on the fly creation of md5 checksums
*/

class MD5_File {
   public:
      MD5_File(void);
      ~MD5_File(void);

      ulong* get_chksum(void);
      
   protected:
      ChkSum sum;
};


class MD5_Ascii_file : public Ascii_file, public MD5_File {
   public: 
      MD5_Ascii_file(void);
      ~MD5_Ascii_file(void);

      virtual int read(void*, int);
      virtual int write(const void*, int);
};

class MD5_Binary_file : public Binary_file, public MD5_File {
   public: 
      MD5_Binary_file(void);
      ~MD5_Binary_file(void);

      virtual int read(void*, int);
      virtual int write(const void*, int);
};

#endif
