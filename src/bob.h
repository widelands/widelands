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

#ifndef __BOB_H
#define __BOB_H

#include "graphic.h"
#include "pic.h"
#include "instances.h"


class Profile;
class Section;
class Animation;

// TODO: move animations into extra animation.* files
struct Animation_Pic {
   ushort *data;
   Animation* parent;
};

struct EncodeData {
	bool hasclrkey;
	uchar clrkey_r, clrkey_g, clrkey_b;
	bool hasshadow;
	uchar shadow_r, shadow_g, shadow_b;
	bool hasplrclrs;
	uchar plrclr_r[4];
	uchar plrclr_g[4];
	uchar plrclr_b[4];

	void clear();
	void parse(Section *s);
	void add(const EncodeData *o);
};

class Animation {
   public:
      enum {
         HAS_TRANSPARENCY = 1,
         HAS_SHADOW = 2,
         HAS_PL_CLR = 3
      };

      Animation(void);
      ~Animation(void);
   
      inline ushort get_w(void) { return w; }
      inline ushort get_h(void) { return h; }
      inline ushort get_hsx(void) { return hsx; }
      inline ushort get_hsy(void) { return hsy; }

      void add_pic(ushort size, ushort* data);
		void add_pic(Pic* pic, const EncodeData *enc);

      void set_flags(uint mflags) { flags=mflags; }
      void set_dimensions(ushort mw, ushort mh) { w=mw; h=mh; }
      void set_hotspot(ushort x, ushort y) { hsx=x; hsy=y; }

      int read(FileRead*);
		void parse(const char *directory, Section *s, const char *picnametempl = 0, const EncodeData *encdefaults = 0);
		
      inline Animation_Pic* get_pic(ushort n) { assert(n<npics); return &pics[n]; }
      inline ushort get_npics(void) { return npics; }

		inline uint get_duration() { return m_frametime * npics; }
		inline Animation_Pic* get_time_pic(uint time) { return &pics[(time / m_frametime) % npics]; }
		
   private:
      uint flags;
		uint m_frametime;
      ushort w, h;
      ushort hsx, hsy;
      ushort npics;
      Animation_Pic *pics;
};

class Bob;

/*
Bobs are moving map objects: Animals, humans, ships...
*/
class Bob_Descr : public Map_Object_Descr {
public:
	Bob_Descr(const char *name);
	virtual ~Bob_Descr(void);

	inline const char* get_name(void) { return m_name; }
	inline Animation* get_anim(void) { return &anim; }

	Bob *create(Game *g, Player *owner, Coords coords);
	
protected:
	virtual Bob *create_object() = 0;
	virtual void read(const char *directory, Section *s);
	
	char m_name[30];
	Animation anim; // the default animation

public:
	static Bob_Descr *create_from_dir(const char *name, const char *directory, Profile *prof);
};

class Bob : public Map_Object {
public:
	enum {
		TASK_NONE = 0,

		// Do nothing. Use start_task_idle() to invoke this task.
		TASK_IDLE = 1,

		// Move along a path. Use start_task_movepath() to invoke this task
		TASK_MOVEPATH = 2,

		// descendants of Map_Objects must use task IDs greater than this
		TASK_FIRST_USER = 10,
	};
		
protected:
	Bob(Bob_Descr *descr);
	virtual ~Bob(void);

public:
	virtual int get_type();
	virtual uint get_movecaps() { return 0; }
		
	virtual void init(Game*);
	virtual void cleanup(Game*);
	virtual void act(Game*);
		
	virtual void draw(Game* game, Bitmap* dst, int posx, int posy);

	inline void set_owner(Player *player) { m_owner = player; }
	inline Player *get_owner() { return m_owner; }
	
	void set_position(Game* g, Coords f);
	inline const FCoords &get_position() const { return m_position; }
	inline Bob* get_next_bob(void) { return m_linknext; }

protected: // default tasks
	void start_task_idle(Game*, Animation* anim, int timeout);
	bool start_task_movepath(Game*, Coords dest, int persist, Animation **anims);
	
protected: // higher level handling (task-based)
	inline int get_current_task() { return m_task; }
	void start_task(Game*, uint task);
	void end_task(Game*, bool success, uint nexttask);
	
	// handler functions
	virtual int task_begin(Game*);
	virtual int task_act(Game*);
	virtual void task_end(Game*);
	
	/** Map_Object::task_start_best(Game*, uint prev, bool success) [virtual]
	 *
	 * prev is the task that was last run (can be 0 on initial startup).
	 * success is the success parameter passed to end_task().
	 * nexthint is the nexttask parameter passed to end_task().
	 *
	 * You must call start_task() (directly or indirectly) from this function.
	 * Therefor, you MUST override this function in derived classes.
	 */
	virtual void task_start_best(Game*, uint prev, bool success, uint nexthint) = 0;

private:
	void do_next_task(Game*);
	void do_start_task(Game*);

protected: // low level handling
	void set_animation(Game* g, Animation* anim);

	int start_walk(Game* g, WalkingDir dir, Animation* a);
	void end_walk(Game* g);
	bool is_walking();

protected:
	Player *m_owner; // can be 0

	FCoords m_position; // where are we right now?
	Bob* m_linknext; // next object on this field
	Bob** m_linkpprev;

	Animation* m_anim;
	int m_animstart; // gametime when the animation was started

	WalkingDir m_walking;
	int m_walkstart; // start and end time used for interpolation
	int m_walkend;

	// Task framework variables		
	uint m_task; // the task we are currently performing
	bool m_task_acting;
	bool m_task_switching;
	bool m_lasttask_success;
	uint m_lasttask;
	uint m_nexttask;

	// Variables used by the default tasks
	union {
		struct {
			int timeout;
		} idle;
		struct {
			int step;
			Animation* anims[6];
			Path* path;
		} movepath;
	} task;
};

#endif
