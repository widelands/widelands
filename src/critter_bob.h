/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__CRITTER_BOB_H
#define __S__CRITTER_BOB_H

#include "bob.h"

class Critter_BobAction;
class Critter_BobProgram;

//
// Description
//
class Critter_Bob_Descr : public Bob_Descr {
	typedef std::map<std::string, Critter_BobProgram*> ProgramMap;
   
   public:
      Critter_Bob_Descr(const char *name, Tribe_Descr* tribe);
      virtual ~Critter_Bob_Descr(void) { }

      virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);
      Bob *create_object();

      inline bool is_swimming(void) { return m_swimming; }
      inline DirAnimations* get_walk_anims(void) { return &m_walk_anims; }
	
      const Critter_BobProgram* get_program(std::string name) const;

   private:
		DirAnimations	m_walk_anims;
      bool				m_swimming;
      ProgramMap     m_programs;
};

class Critter_Bob : public Bob {
   friend class Widelands_Map_Bobdata_Data_Packet;
	friend class Critter_BobProgram;

   MO_DESCR(Critter_Bob_Descr);

public:
	Critter_Bob(Critter_Bob_Descr *d);
	virtual ~Critter_Bob(void);

   virtual int get_bob_type() { return Bob::CRITTER; }
	uint get_movecaps();

	virtual void init_auto_task(Game* g);

	void start_task_program(Game* g, std::string name);

private:
	void roam_update(Game* g, State* state);
	void roam_signal(Game* g, State* state);
	
   void program_update(Game* g, State* state);
	void program_signal(Game* g, State* state);
	
private:
   bool run_remove(Game* g, State* state, const Critter_BobAction* act);

private:
	static Task taskRoam;
	static Task taskProgram;
};

#endif // __S__CRITTER_BOB_H
