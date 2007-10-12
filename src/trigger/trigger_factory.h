/*
 * Copyright (C) 2002-2004, 2007 by the Widelands Development Team
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

#ifndef __S__TRIGGER_FACTORY_H
#define __S__TRIGGER_FACTORY_H

#include <string>

#include <stdint.h>

class Trigger;
class Editor_Interactive;


struct Trigger_Descr {
   std::string id;
   const std::string name;
   const std::string descr;
};

/*
 * This class returns the correct descriptions,
 * ids and creates the correct option dialog and (of course) trigger
 * for each trigger-id
 */
struct Trigger_Factory {
      static Trigger* get_correct_trigger(const char* id);
      static Trigger* make_trigger_with_option_dialog(const char* id, Editor_Interactive*, Trigger*);

      static Trigger_Descr* get_correct_trigger_descr(const char* id);
      static const uint32_t get_nr_of_available_triggers();
      static Trigger_Descr* get_trigger_descr(uint32_t);
};

#endif
