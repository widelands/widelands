/*
 * Copyright (C) 2003 by The Widelands Development Team
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

#ifndef __S__EDITOR_TOOLS_H
#define __S__EDITOR_TOOLS_H

class Editor_Interactive;
#include "map.h"
#include "ui.h"
#include "world.h"
#include <vector>

#define MAX_TOOL_AREA 6

/*
=============================
class Editor_Tool

an editor tool is a tool that can be selected in the editor.
Examples are: modify height, place bob, place critter,
place building. A Tool only makes one function (like delete_building,
place building, modify building are 3 tools)
=============================
*/
class Editor_Tool {
   public:
      Editor_Tool(Editor_Tool* second, Editor_Tool* third) { m_second=second; m_third=third; if(!m_second) m_second=this; if(!m_third) m_third=this; }
      virtual ~Editor_Tool() { 
         if(m_second==m_third) m_third=0;
         if(m_second && m_second!=this) { delete m_second; m_second=0; }
         if(m_third && m_third!=this) { delete m_third; m_third=0; }
      }

      int handle_click(int n, const Coords* c, Field* f, Map* m, Editor_Interactive* parent) {
         if(n==0) return this->handle_click_impl(c,f,m,parent);
         if(n==1) return m_second->handle_click_impl(c,f,m,parent);
         if(n==2) return m_third->handle_click_impl(c,f,m,parent);
         return 0;
      }
      const char* get_fsel(int n) { 
         if(n==0) return this->get_fsel_impl(); 
         if(n==1) return m_second->get_fsel_impl(); 
         if(n==2) return m_third->get_fsel_impl(); 
         return 0; 
      }
      
      virtual int handle_click_impl(const Coords*, Field* field, Map* m, Editor_Interactive* parent) = 0;
      virtual const char* get_fsel_impl(void) = 0;

   protected:
      Editor_Tool* m_second, *m_third;
};

/*
=============================
class Editor_Info_Tool

this is a simple tool to show information about the clicked field
=============================
*/
class Editor_Info_Tool : public Editor_Tool {
   public:
      Editor_Info_Tool() : Editor_Tool(this,this) { }
      virtual ~Editor_Info_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_info.png"; }
};


/*
=============================
class Editor_Decrease_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Decrease_Height_Tool : public Editor_Tool {
   public:
      Editor_Decrease_Height_Tool() : Editor_Tool(this,this) { m_changed_by=1; }
      virtual ~Editor_Decrease_Height_Tool() { }
  
      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_decrease_height.png"; }
      
      inline int get_changed_by(void) { return m_changed_by; }
      inline void set_changed_by(int n) { m_changed_by=n; }
      
   private:
      int m_changed_by;
};

/*
=============================
class Editor_Set_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Height_Tool : public Editor_Tool {
   public:
      Editor_Set_Height_Tool() : Editor_Tool(this,this) { m_set_to=10; }
      virtual ~Editor_Set_Height_Tool() { }
  
      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_set_height.png"; }
      
      inline int get_set_to(void) { return m_set_to; }
      inline void set_set_to(int n) { m_set_to=n; }
      
   private:
      int m_set_to;
};

/*
=============================
class Editor_Increase_Height_Tool

this increases the height of a field by a value
=============================
*/
class Editor_Increase_Height_Tool : public Editor_Tool {
   public:
      Editor_Increase_Height_Tool(Editor_Decrease_Height_Tool* dht, Editor_Set_Height_Tool* sht)
        : Editor_Tool(dht, sht) { m_changed_by=1; m_dht=dht; m_sht=sht; }
      virtual ~Editor_Increase_Height_Tool() {  }
  
      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_increase_height.png"; }
     
      inline int get_changed_by(void) { return m_changed_by; }
      inline void set_changed_by(int n) { m_changed_by=n; }
      
      Editor_Decrease_Height_Tool* get_dht(void) { return m_dht; }
      Editor_Set_Height_Tool* get_sht(void) { return m_sht; }
      
   private:
      Editor_Decrease_Height_Tool* m_dht; 
      Editor_Set_Height_Tool* m_sht;
      int m_changed_by;
};


/*
=============================
class Editor_Noise_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Noise_Height_Tool : public Editor_Tool {
   public:
      Editor_Noise_Height_Tool(Editor_Set_Height_Tool* sht) :
        Editor_Tool(sht,sht) { m_upper_value=MAX_FIELD_HEIGHT/2; m_lower_value=10; m_sht=sht; }
      virtual ~Editor_Noise_Height_Tool() { m_third=m_second=0; } // don't delete this, somone else will care  
  
      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_noise_height.png"; }
     
      inline Editor_Set_Height_Tool* get_sht(void) { return m_sht; }
      
      inline void get_values(int* a, int* b) { *a=m_lower_value; *b=m_upper_value; }
      inline void set_values(int a, int b) { m_lower_value=a; m_upper_value=b; }

   private:
      Editor_Set_Height_Tool* m_sht;
      int m_upper_value;
      int m_lower_value;
};

/*
=============================
class MultiSelect

this class allows for selection of more than just one 
thing. Like more than one texture, more than one map object
=============================
*/
class MultiSelect {
   public:
      MultiSelect() { m_nr_enabled=0; }
      ~MultiSelect() { }

      void enable(int n, bool t) {
         if((int)m_enabled.size()<(n+1)) 
            m_enabled.resize(n+1,false);

         if(m_enabled[n]==t) return;
         m_enabled[n]=t;
         if(t) ++m_nr_enabled; 
         else --m_nr_enabled;  
         assert(m_nr_enabled>=0); 
      }
      inline bool is_enabled(int n) {
         if((int)m_enabled.size()<(n+1)) return false;
         return m_enabled[n];
      }
      inline int get_nr_enabled(void) { return m_nr_enabled; }
      inline int get_random_enabled(void) {
         int rand_value=(int) ((double)(get_nr_enabled())*rand()/(RAND_MAX+1.0));
         int i=0;
         int j=rand_value+1;
         while(j) { if(is_enabled(i)) j--; ++i; }
         return i-1;
      }

   private:
      int m_nr_enabled;
      std::vector<bool> m_enabled;
};

/*
=============================
class Set_Terrain 

This is the base class for the terrain set tools. 
If offers the functionality to enable/disable terrains
and to set the texture effectivly
=============================
*/
class Set_Terrain : public MultiSelect {
   public:
      Set_Terrain() { }
      ~Set_Terrain() { }

   protected:
      int set_terrain(const Coords*, Field*, Map*, Editor_Interactive*, bool right, bool down);
};
      
/*
=============================
class Editor_Set_Right_Terrain_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Right_Terrain_Tool : public Editor_Tool, public Set_Terrain {
   public:
      Editor_Set_Right_Terrain_Tool() : Editor_Tool(this,this) { }
      virtual ~Editor_Set_Right_Terrain_Tool() { }
  
      virtual int handle_click_impl(const Coords* c, Field* f, Map* m, Editor_Interactive* e) {
         return set_terrain(c,f,m,e,true,false);
      }
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_terrain_right.png"; }
};

/*
=============================
class Editor_Set_Down_Terrain_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Down_Terrain_Tool : public Editor_Tool, public Set_Terrain {
   public:
      Editor_Set_Down_Terrain_Tool() : Editor_Tool(this,this) { }
      virtual ~Editor_Set_Down_Terrain_Tool() { }
  
      virtual int handle_click_impl(const Coords* c, Field* f, Map* m, Editor_Interactive* e) {
         return set_terrain(c,f,m,e,false,true);
      }
      
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_terrain_down.png"; }
};


/*
=============================
class Editor_Set_Both_Terrain_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Both_Terrain_Tool : public Editor_Tool, public Set_Terrain {
   public:
      Editor_Set_Both_Terrain_Tool(Editor_Set_Down_Terrain_Tool* sdt, Editor_Set_Right_Terrain_Tool* srt) : 
        Editor_Tool(sdt, srt) { m_sdt=sdt; m_srt=srt; }
      virtual ~Editor_Set_Both_Terrain_Tool() { }
  
      virtual int handle_click_impl(const Coords* c, Field* f, Map* m, Editor_Interactive* e) {
         return set_terrain(c,f,m,e,true,true);
      }
     
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_terrain_both.png"; }
      void enable(int n, bool t) {
         m_sdt->enable(n,t);
         m_srt->enable(n,t);
         MultiSelect::enable(n,t);
      }
      
   private:
      Editor_Set_Down_Terrain_Tool* m_sdt;
      Editor_Set_Right_Terrain_Tool* m_srt;
};

/*
=============================
class Editor_Delete_Immovable_Tool

this deletes immovables from the map
=============================
*/
class Editor_Delete_Immovable_Tool : public Editor_Tool {
   public:
      Editor_Delete_Immovable_Tool() : Editor_Tool(this,this) { }
      ~Editor_Delete_Immovable_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_delete.png"; }
};

/*
=============================
class Editor_Place_Immovable_Tool

this places immovables on the map
=============================
*/
class Editor_Place_Immovable_Tool : public Editor_Tool, public MultiSelect {
   public:
      Editor_Place_Immovable_Tool(Editor_Delete_Immovable_Tool* tool) : Editor_Tool(tool, tool) { 
      }
      ~Editor_Place_Immovable_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_place_immovable.png"; }
};

#endif // __S__EDITOR_TOOLS_H
