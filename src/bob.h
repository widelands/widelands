/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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
#include "animation.h"
#include "instances.h"


class Profile;
class Bob;
class Route;
class Request;
class Transfer;
class WorkerProgram;


/*
Bobs are moving map objects: Animals, humans, ships...
*/
class Bob_Descr : public Map_Object_Descr {
public:
	Bob_Descr(const char *name);
	virtual ~Bob_Descr(void);

	inline const char* get_name(void) const { return m_name; }
	inline uint get_idle_anim(void) const { return m_idle_anim; }

	Bob *create(Editor_Game_Base *g, Player *owner, Coords coords);

protected:
	virtual Bob *create_object() = 0;
	virtual void parse(const char *directory, Profile *prof, const EncodeData *encdata);

	char	m_name[30];
	uint	m_idle_anim; // the default animation

public:
	static Bob_Descr *create_from_dir(const char *name, const char *directory, Profile *prof);
};

class Bob : public Map_Object {
	MO_DESCR(Bob_Descr);

public:
	struct State;

	typedef void (Bob::*Ptr)(Game*, State*);

	struct Task {
		const char* name;

		Ptr update;
		Ptr signal;
		Ptr mask;
	};

	struct State {
		Task*				task;
		int				ivar1;
		int				ivar2;
		Object_Ptr		objvar1;
		std::string		svar1;

		Coords					coords;
		DirAnimations*			diranims;
		Path*						path;
		Transfer*				transfer;
		Route*					route;
		const WorkerProgram*	program;
	};

protected:
	Bob(Bob_Descr *descr);
	virtual ~Bob(void);

public:
	uint get_current_anim() const { return m_anim; }
	int get_animstart() const { return m_animstart; }

	virtual int get_type();
	virtual uint get_movecaps() { return 0; }
	std::string get_name() const { return get_descr()->get_name(); }

	virtual void init(Editor_Game_Base*);
	virtual void cleanup(Editor_Game_Base*);
	virtual void act(Game*, uint data);

	void schedule_destroy(Game* g);
	void schedule_act(Game* g, uint tdelta);
	void skip_act(Game* g);
	void force_skip_act(Game* g);

	void calc_drawpos(Editor_Game_Base* game, Point pos, Point* drawpos);
	virtual void draw(Editor_Game_Base* game, RenderTarget* dst, Point pos);

	inline void set_owner(Player *player) { m_owner = player; }
	inline Player *get_owner() { return m_owner; }

	void set_position(Game* g, Coords f);
	inline const FCoords& get_position() const { return m_position; }
	inline Bob* get_next_bob(void) { return m_linknext; }

public: // default tasks
	void reset_tasks(Game*);
	void send_signal(Game*, std::string sig);

	void start_task_idle(Game*, uint anim, int timeout);
	bool start_task_movepath(Game*, Coords dest, int persist, DirAnimations *anims, bool forceonlast = false);
	void start_task_movepath(Game*, const Path &path, DirAnimations *anims, bool forceonlast = false);
	bool start_task_movepath(Game* g, const Path& path, int index, DirAnimations* anims, bool forceonlast = false);
	void start_task_forcemove(Game*, int dir, DirAnimations *anims);

protected: // higher level handling (task-based)
	inline State* get_state() { return m_stack.size() ? &m_stack[m_stack.size() - 1] : 0; }
	inline std::string get_signal() { return m_signal; }
	State* get_state(Task* task);

	void push_task(Game*, Task* task);
	void pop_task(Game*);
	void set_signal(std::string sig);

	virtual void init_auto_task(Game*);

protected: // low level animation and walking handling
	void set_animation(Game* g, uint anim);

	int start_walk(Game* g, WalkingDir dir, uint anim, bool force = false);
	void end_walk(Game* g);
	bool is_walking();

private:
	void do_act(Game* g, bool signalhandling);

	void idle_update(Game* g, State* state);
	void idle_signal(Game* g, State* state);
	void movepath_update(Game* g, State* state);
	void forcemove_update(Game* g, State* state);

private:
	static Task taskIdle;
	static Task taskMovepath;
	static Task taskForcemove;

private:
	Player*		m_owner; // can be 0

	FCoords		m_position; // where are we right now?
	Bob*			m_linknext; // next object on this field
	Bob**			m_linkpprev;

	uint			m_actid; // CMD_ACT counter, used to eliminate spurious act()s

	uint			m_anim;
	int			m_animstart; // gametime when the animation was started

	WalkingDir	m_walking;
	int			m_walkstart; // start and end time used for interpolation
	int			m_walkend;

	// Task framework variables
	std::vector<State>	m_stack;
	bool						m_stack_dirty;
	bool						m_sched_init_task;	// if init_auto_task was scheduled
	std::string				m_signal;
};

#endif
