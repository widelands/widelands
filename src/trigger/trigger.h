/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef __S__TRIGGER_H
#define __S__TRIGGER_H

#include <map>
#include <string>

struct Section;

namespace Widelands {

class Game;
class Editor_Game_Base;
class TriggerReferencer;

/*
 * A trigger is a switch for events. Each event can register
 * one or more triggers with himself; when all triggers are set
 * the event runs.
 */
struct Trigger {
	friend struct Map_Trigger_Data_Packet;

	Trigger(const std::string & Name) : m_name(Name) {}
      virtual ~Trigger() {}

      // virtual functions, implemented by the real triggers
      virtual void check_set_conditions(Game*) = 0;
	virtual const char * get_id() const = 0; // this function is needed to recreate the correct option window

      // Toggle the triggers state (if it isn't a one timer)
      // and give it a chance to reinitialize
      virtual void reset_trigger(Game*) = 0;

      // Functions needed by all
      void set_name(const char* name) {m_name=name;}
	const char * get_name() const {return m_name.c_str();}
	bool is_set() const {return m_is_set;}

      // File functions, to save or load this trigger
	virtual void Write(Section &) const = 0;
      virtual void Read(Section*, Editor_Game_Base*) = 0;

      // Reference this event
      void reference(TriggerReferencer* ref);
      void unreference(TriggerReferencer* ref);
	typedef std::map<TriggerReferencer *, uint32_t> TriggerReferencerMap;
	const TriggerReferencerMap & get_referencers() const throw ()
	{return m_referencers;}

protected:
	// This is only for child classes to toggle the trigger
	void set_trigger(bool t) {m_is_set = t;}

private:
      std::string                m_name;
      bool                        m_is_set;
	TriggerReferencerMap m_referencers;
};

};

#endif
