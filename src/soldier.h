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

#ifndef __S__SOLDIER_H
#define __S__SOLDIER_H

#include "worker.h"

class Editor_Game_Base;

#define HP_FRAMECOLOR RGBColor(255,255,255)

class Soldier_Descr : public Worker_Descr {
public:
	Soldier_Descr(Tribe_Descr *tribe, const char *name);
	virtual ~Soldier_Descr(void);
   
   virtual Worker_Type get_worker_type(void) const { return SOLDIER; }

   virtual void load_graphics(void); 

   uint get_max_hp_level(void) { return m_max_hp_level; }
   uint get_max_attack_level(void) { return m_max_attack_level; }
   uint get_max_defense_level(void) { return m_max_defense_level; }
   uint get_max_evade_level(void) { return m_max_evade_level; }

   uint get_min_hp() { return m_min_hp; }
   uint get_max_hp() { return m_max_hp; }
   uint get_min_attack() { return m_min_attack; }
   uint get_max_attack() { return m_max_attack; }
   uint get_defense() { return m_defense; }
   uint get_evade() { return m_evade; }

   uint get_hp_incr_per_level() { return m_hp_incr; }
   uint get_attack_incr_per_level() { return m_attack_incr; }
   uint get_defense_incr_per_level() { return m_defense_incr; }
   uint get_evade_incr_per_level() { return m_evade_incr; }

   uint get_hp_level_pic(uint level) { assert(level<=m_max_hp_level && level>=0); return m_hp_pics[level]; }
   uint get_attack_level_pic(uint level) { assert(level<=m_max_attack_level && level>=0); return m_attack_pics[level]; }
   uint get_defense_level_pic(uint level) { assert(level<=m_max_defense_level && level>=0); return m_defense_pics[level]; }
   uint get_evade_level_pic(uint level) { assert(level<=m_max_evade_level && level>=0); return m_evade_pics[level]; }

protected:
	virtual Bob *create_object();
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);

   // Start values
   uint m_min_hp;
   uint m_max_hp;
   uint m_min_attack;
   uint m_max_attack;
   uint m_defense;
   uint m_evade;
   
   // per level increases
   uint m_hp_incr;
   uint m_attack_incr;
   uint m_defense_incr;
   uint m_evade_incr;
   
   // max levels
   uint m_max_hp_level;
   uint m_max_attack_level;
   uint m_max_defense_level;
   uint m_max_evade_level;

   // Level pictures
   std::vector<uint> m_hp_pics;
   std::vector<uint> m_attack_pics;
   std::vector<uint> m_evade_pics;
   std::vector<uint> m_defense_pics;
   std::vector<std::string> m_hp_pics_fn;
   std::vector<std::string> m_attack_pics_fn;
   std::vector<std::string> m_evade_pics_fn;
   std::vector<std::string> m_defense_pics_fn;
};

class Soldier : public Worker {
   friend class Widelands_Map_Bobdata_Data_Packet; // Writes this to disk
	MO_DESCR(Soldier_Descr);

public:
	Soldier(Soldier_Descr *descr);
	virtual ~Soldier();
   
   virtual void init(Editor_Game_Base*); 

   void set_level(uint, uint, uint, uint);
   void set_hp_level(uint); 
   void set_attack_level(uint);
   void set_defense_level(uint);
   void set_evade_level(uint); 
   uint get_hp_level(void) { return m_hp_level; }
   uint get_attack_level(void) { return m_attack_level; }
   uint get_defense_level(void) { return m_defense_level; }
   uint get_evade_level(void) { return m_evade_level; }
   
public:
   virtual Worker_Descr::Worker_Type get_worker_type(void) const { return get_descr()->get_worker_type(); }

   // Draw this soldier
   void draw(Editor_Game_Base* g, RenderTarget* dst, Point pos);

   // Information function from description
   uint get_max_hp_level(void) { return get_descr()->get_max_hp_level(); }
   uint get_max_attack_level(void) { return get_descr()->get_max_attack_level(); }
   uint get_max_defense_level(void) { return get_descr()->get_max_defense_level(); }
   uint get_max_evade_level(void) { return get_descr()->get_max_evade_level(); }

   // information functions
   uint get_current_hitpoints(void) { return m_hp_current; }
   uint get_max_hitpoints(void) { return m_hp_max; }
   uint get_min_attack(void) { return m_min_attack; }
   uint get_max_attack(void) { return m_max_attack; }
   uint get_defense(void) { return m_defense; }
   uint get_evade(void) { return m_evade; }
   
   // get pictures
   uint get_hp_level_pic(void) { return get_descr()->get_hp_level_pic(m_hp_level); }
   uint get_attack_level_pic(void) { return get_descr()->get_attack_level_pic(m_attack_level); }
   uint get_defense_level_pic(void) { return get_descr()->get_defense_level_pic(m_defense_level); }
   uint get_evade_level_pic(void) { return get_descr()->get_evade_level_pic(m_evade_level); }

private:
   // Private data
   uint m_hp_current;
   uint m_hp_max;
   uint m_min_attack;
   uint m_max_attack;
   uint m_defense;
   uint m_evade;

   uint m_hp_level;
   uint m_attack_level;
   uint m_defense_level;
   uint m_evade_level;
};


#endif // __S__SOLDIER_H
