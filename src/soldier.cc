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

#include "editor_game_base.h"
#include "game.h"
#include "graphic.h"
#include "profile.h"
#include "rendertarget.h"
#include "rgbcolor.h"
#include "soldier.h"
#include "util.h"
#include "wexception.h"

/*
==============================================================

SOLDIER DESCRIPTION IMPLEMENTATION

==============================================================
*/

/*
===============
Soldier_Descr::Soldier_Descr
Soldier_Descr::~Soldier_Descr
===============
*/
Soldier_Descr::Soldier_Descr(Tribe_Descr *tribe, const char *name)
	: Worker_Descr(tribe, name)
{
}

Soldier_Descr::~Soldier_Descr(void)
{
}

/*
===============
Soldier_Descr::parse

Parse carrier-specific configuration data
===============
*/
void Soldier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
   Section* sglobal=prof->get_section("global");
  
   // Parse hitpoints
   std::string hp=sglobal->get_safe_string("hp");
   std::vector<std::string> list;
   split_string(hp, &list, "-");
   if(list.size()!=2) 
      throw wexception("Parse error in hp string: \"%s\" (must be \"min-max\")", hp.c_str());
   uint i=0;
   for(i=0; i<list.size(); i++)
      remove_spaces(&list[i]);
   char* endp;
   m_min_hp= strtol(list[0].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in hp string: %s is a bad value", list[0].c_str());
   m_max_hp = strtol(list[1].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in hp string: %s is a bad value", list[1].c_str());

   // Parse attack
   std::string attack=sglobal->get_safe_string("attack");
   list.resize(0);
   split_string(attack, &list, "-");
   if(list.size()!=2) 
      throw wexception("Parse error in attack string: \"%s\" (must be \"min-max\")", attack.c_str());
   for(i=0; i<list.size(); i++)
      remove_spaces(&list[i]);
   m_min_attack= strtol(list[0].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in attack string: %s is a bad value", list[0].c_str());
   m_max_attack = strtol(list[1].c_str(),&endp, 0);
   if(endp && *endp)
      throw wexception("Parse error in attack string: %s is a bad value", list[1].c_str());

   // Parse defend
   m_defense=sglobal->get_safe_int("defense");
   
   // Parse evade
   m_evade=sglobal->get_safe_int("evade");

   // Parse increases per level
   m_hp_incr=sglobal->get_safe_int("hp_incr_per_level");
   m_attack_incr=sglobal->get_safe_int("attack_incr_per_level");
   m_defense_incr=sglobal->get_safe_int("defense_incr_per_level");
   m_evade_incr=sglobal->get_safe_int("evade_incr_per_level");

   // Parse max levels
   m_max_hp_level=sglobal->get_safe_int("max_hp_level");
   m_max_attack_level=sglobal->get_safe_int("max_attack_level");
   m_max_defense_level=sglobal->get_safe_int("max_defense_level");
   m_max_evade_level=sglobal->get_safe_int("max_evade_level");
 
   // Load the filenames
   m_hp_pics_fn.resize(m_max_hp_level);
   m_attack_pics_fn.resize(m_max_attack_level);
   m_defense_pics_fn.resize(m_max_defense_level);
   m_evade_pics_fn.resize(m_max_evade_level);
   char buffer[256];
   std::string dir=directory;
   dir+="/";
   for(i=1; i<=m_max_hp_level; i++) {
      sprintf(buffer, "hp_level_%i_pic", i);
      m_hp_pics_fn[i-1]=dir;
      m_hp_pics_fn[i-1]+=sglobal->get_safe_string(buffer);
   }
   for(i=1; i<=m_max_attack_level; i++) {
      sprintf(buffer, "attack_level_%i_pic", i);
      m_attack_pics_fn[i-1]=dir;
      m_attack_pics_fn[i-1]+=sglobal->get_safe_string(buffer);
   }
   for(i=1; i<=m_max_defense_level; i++) {
      sprintf(buffer, "defense_level_%i_pic", i);
      m_defense_pics_fn[i-1]=dir;
      m_defense_pics_fn[i-1]+=sglobal->get_safe_string(buffer);
   }
   for(i=1; i<=m_max_evade_level; i++) {
      sprintf(buffer, "evade_level_%i_pic", i);
      m_evade_pics_fn[i-1]=dir;
      m_evade_pics_fn[i-1]+=sglobal->get_safe_string(buffer);
   }
}

/*
 * Load the graphics
 */
void Soldier_Descr::load_graphics(void) {
   m_hp_pics.resize(m_max_hp_level);
   m_attack_pics.resize(m_max_attack_level);
   m_defense_pics.resize(m_max_defense_level);
   m_evade_pics.resize(m_max_evade_level);
   uint i;
   for(i=1; i<=m_max_hp_level; i++) {
      m_hp_pics[i-1]=g_gr->get_picture(PicMod_Game, m_hp_pics_fn[i-1].c_str(), true);
   }
   for(i=1; i<=m_max_attack_level; i++) {
      m_attack_pics[i-1]=g_gr->get_picture(PicMod_Game, m_attack_pics_fn[i-1].c_str(), true);
   }
   for(i=1; i<=m_max_defense_level; i++) {
      m_defense_pics[i-1]=g_gr->get_picture(PicMod_Game, m_defense_pics_fn[i-1].c_str(), true);
   }
   for(i=1; i<=m_max_evade_level; i++) {
      m_evade_pics[i-1]=g_gr->get_picture(PicMod_Game, m_evade_pics_fn[i-1].c_str(), true);
   }
   m_hp_pics_fn.resize(0);
   m_attack_pics_fn.resize(0);
   m_defense_pics_fn.resize(0);
   m_evade_pics_fn.resize(0);

   Worker_Descr::load_graphics();
}

/*
 * Create a new soldier
 */
Bob* Soldier_Descr::create_object() {
   return new Soldier(this);
}

/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Soldier::Soldier
Soldier::~Soldier
===============
*/
Soldier::Soldier(Soldier_Descr *descr)
	: Worker(descr)
{
   // all done through init
}

Soldier::~Soldier()
{
}

/*
 * Initialize this soldier
 */
void Soldier::init(Editor_Game_Base* gg) {
   m_hp_level=1;
   m_attack_level=1;
   m_defense_level=1;
   m_evade_level=1;

   m_hp_max=0;
   m_min_attack=get_descr()->get_min_attack();
   m_max_attack=get_descr()->get_max_attack();
   m_defense=get_descr()->get_defense();
   m_evade=get_descr()->get_evade();
   if(gg->is_game()) {
      Game* g= static_cast<Game*>(g);
      int range=get_descr()->get_max_hp()-get_descr()->get_min_hp();
      int value=g->logic_rand() % range;
      m_hp_max=value;
   }
   m_hp_current=m_hp_max;
   
   Worker::init(gg);
}

/*
 * Draw this soldier. This basically draws him as a worker, but add hitpoints
 */
void Soldier::draw(Editor_Game_Base* g, RenderTarget* dst, Point pos) {
   uint anim = get_current_anim();

	if (!anim)
		return;

	Point drawpos;
	calc_drawpos(g, pos, &drawpos);

   int w, h;
   g_gr->get_animation_size(anim, g->get_gametime() - get_animstart(), &w, &h);
   
   // Draw energy bar
   // first: draw white sourrounding
   const int frame_width=w<<1;  // width * 2
   const int frame_height=5;
   const int frame_beginning_x=drawpos.x-(frame_width>>1);  // TODO: these should be calculated from the hot spot, not assumed
   const int frame_beginning_y=drawpos.y-h-7;     
   dst->draw_rect(frame_beginning_x, frame_beginning_y, frame_width, frame_height, HP_FRAMECOLOR);
   // Draw energybar
   float percent = (float)m_hp_current/m_hp_max;
   int energy_width=static_cast<int>(percent * (frame_width-2));
   RGBColor color;
   if (percent <= 0.15)
		color = RGBColor(255, 0, 0);
	else if (percent <= 0.5)
		color = RGBColor(255, 255, 0);
	else
		color = RGBColor(17,192,17);
   dst->fill_rect(frame_beginning_x+1, frame_beginning_y+1, energy_width, frame_height-2, color); 

   // Draw information fields about levels
   // first, gather informations
   uint hppic=get_hp_level_pic();
   uint attackpic=get_attack_level_pic();
   uint defensepic=get_defense_level_pic();
   uint evadepic=get_evade_level_pic();
   int hpw,hph,atw,ath,dew,deh,evw,evh; 
   g_gr->get_picture_size(hppic, &hpw, &hph);
   g_gr->get_picture_size(attackpic, &atw, &ath);
   g_gr->get_picture_size(defensepic, &dew, &deh);
   g_gr->get_picture_size(evadepic, &evw, &evh);

   dst->blit(frame_beginning_x, frame_beginning_y-hph-ath, attackpic);
   dst->blit(frame_beginning_x+(frame_width>>1), frame_beginning_y-evh-deh, defensepic);
   dst->blit(frame_beginning_x, frame_beginning_y-hph, hppic);
   dst->blit(frame_beginning_x+(frame_width>>1), frame_beginning_y-evh, evadepic);
   
   Worker::draw(g,dst,pos);
}

