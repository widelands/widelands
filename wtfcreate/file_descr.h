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

#ifndef __S__FILE_DESCR_H
#define __S__FILE_DESCR_H

#include "../src/myfile.h"
#include "../src/profile.h"
#include "bob_descr.h"

/** 
 * virtual class to define the functionality of a 
 * class that can save itself to and construct itself 
 * through configuration files
 *
 * This class is only needed in the creators
 *
 * needs a file to write 
 */
class File_Descr {
   File_Descr(const File_Descr&);
   File_Descr& operator=(const File_Descr&);

   public:
      enum {
         OK = 0,
         SECTION_MISSING,
         KEY_MISSING,
         PARSE_ERROR,
         SYNTAX_ERROR,
         ERROR
      };
         
      File_Descr(void);
      virtual ~File_Descr(void);

      const char* get_last_err_section(void) { return err_sec; }
      const char* get_last_err_key(void) { return err_key; }
      const char* get_last_err_msg(void) { return err_msg; }

      virtual int write(Binary_file* f)=0; 
      virtual int construct(Profile* p, Section* s)=0;

   protected:
      char err_sec[256];
      char err_key[256];
      char err_msg[256];
};


#endif

