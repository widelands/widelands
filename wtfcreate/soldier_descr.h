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

#ifndef __S__SOLDIER_DESCR_H
#define __S__SOLDIER_DESCR_H

#include "../src/myfile.h"
#include "../src/profile.h"
#include "../src/helper.h"
#include "file_descr.h"
#include "pic_descr.h"
#include "bob_descr.h"
#include "fabric.h"
#include "wtfcreate.h"
#include "worker_descr.h"

class Soldier_Descr : virtual public Worker_Descr,
      virtual public Menu_Worker_Descr {
   friend class Fabric<Soldier_Descr>;
      
   
   private:
         Soldier_Descr(const char* name);
         virtual const char* get_subdir(void) { return SOLDIERS_DIR; }

   public:
         virtual ~Soldier_Descr(void);
         virtual int construct(Profile* p, Section* s);
         virtual int write(Binary_file* f);

   private:
         uint energy;

         Bob_Descr attack_l;
         Bob_Descr attack1_l;
         Bob_Descr evade_l;
         Bob_Descr evade1_l;
         Bob_Descr attack_r;
         Bob_Descr attack1_r;
         Bob_Descr evade_r;
         Bob_Descr evade1_r;
      };

extern Fabric<Soldier_Descr> soldierf;

#endif
