/*
 * Copyright (C) 2002-5 by the Widelands Development Team
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

#ifndef __S__TRIGGER_MANAGER_H
#define __S__TRIGGER_MANAGER_H

#include "error.h"
#include "event_chain.h"
#include <string>

class Trigger;

/*
 * The Trigger Manager keeps all triggers 
 * in order
 *
 */
/*
 * The manager himself. 
 * This is mainly a wrapper, the function
 * could have also been implemented directly in the map.
 *
 * But it is better this way.
 */
class MapTriggerManager {
   public:
      MapTriggerManager( void );
      ~MapTriggerManager( void );

      /*
       * Register a new trigger
       */
      bool register_new_trigger( Trigger* );

      /*
       * Delete unused trigger
       */
      void delete_unreferenced_triggers( void );

      /*
       * Get a trigger
       */
      Trigger* get_trigger( const wchar_t* name );
      void delete_trigger( const wchar_t* name );

      inline int get_nr_triggers( void ) { return m_triggers.size(); }
      inline Trigger* get_trigger_by_nr( int i ) { assert(i < (int)m_triggers.size()); return m_triggers[i]; }

   private:
      std::vector<Trigger*>      m_triggers; 
};

#endif

