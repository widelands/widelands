/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef included_immovable_h
#define included_immovable_h

#include "instances.h"
#include "animation.h"

class Profile;
class Request;
class Flag;
class Economy;
class WaresQueue;
class Worker;

/*
BaseImmovable is the base for all non-moving objects (immovables such as trees,
buildings, flags, roads).

The immovable's size influences building capabilities around it.
If size is NONE, the immovable can simply be removed by placing something on it
(this is usually true for decorations).
For more information, see the Map::recalc_* functions.
*/
class BaseImmovable : public Map_Object {
public:
	enum {
		NONE,			// not robust
		SMALL,
		MEDIUM,
		BIG
	};

	BaseImmovable(Map_Object_Descr *descr);
	virtual ~BaseImmovable();

	virtual int get_size() = 0;
	virtual bool get_passable() = 0;
	virtual void draw(Editor_Game_Base*, RenderTarget* dst, FCoords coords, Point pos) = 0;

protected:
	void set_position(Editor_Game_Base *g, Coords c);
	void unset_position(Editor_Game_Base *g, Coords c);
};


class Immovable;
class ImmovableProgram;
struct ImmovableAction;

/*
Immovable represents a standard immovable such as trees or stones.
*/
class Immovable_Descr : public Map_Object_Descr {
public:
	typedef std::map<std::string, ImmovableProgram*> ProgramMap;
	typedef std::map<std::string, uint> AnimationMap;

public:
	Immovable_Descr(const char *name);
	~Immovable_Descr();

	inline const char* get_name(void) const { return m_name; }
	inline int get_size(void) const { return m_size; }
   inline const char* get_picture(void) const { return m_picture.c_str(); }
	inline const ImmovableProgram* get_program(std::string name) const;
	inline const EncodeData& get_default_encodedata() const { return m_default_encodedata; }

	void parse(const char *directory, Profile *s);
	void parse_program(std::string directory, Profile* prof, std::string name);
	uint parse_animation(std::string directory, Profile* prof, std::string name);
	Immovable *create(Editor_Game_Base *g, Coords coords);

protected:
   std::string m_picture;
   char			m_name[30];
	int			m_size;
	EncodeData	m_default_encodedata;

	ProgramMap		m_programs;
	AnimationMap	m_animations;
};

class Immovable : public BaseImmovable {
	friend class Immovable_Descr;
	friend class ImmovableProgram;

	MO_DESCR(Immovable_Descr);

public:
	Immovable(Immovable_Descr *descr);
	~Immovable();

	Coords get_position() const { return m_position; }

	int get_type();
	int get_size();
	bool get_passable();

	void init(Editor_Game_Base *g);
	void cleanup(Editor_Game_Base *g);
	void act(Game *g, uint data);

	void draw(Editor_Game_Base*, RenderTarget* dst, FCoords coords, Point pos);

	void switch_program(Game* g, std::string name);

protected:
	void set_program_animation(Editor_Game_Base* g);
	void run_program(Game* g, bool killable);

private:
	bool run_animation(Game* g, bool killable, const ImmovableAction& action);
	bool run_transform(Game* g, bool killable, const ImmovableAction& action);
	bool run_remove(Game* g, bool killable, const ImmovableAction& action);

protected:
	Coords		m_position;

	uint			m_anim;
	int			m_animstart;

	const ImmovableProgram*	m_program;
	uint							m_program_ptr;			// index of next instruction to execute
	int							m_program_step;		// time of next step
};


/*
PlayerImmovable is an immovable owned by a player that belongs to an economy:
building, flag or road

A PlayerImmovable can also house a number of workers, which are automatically
turned into fugitives when the immovable is destroyed, and their economy is also
adjusted automatically.
*/
class PlayerImmovable : public BaseImmovable {
public:
	PlayerImmovable(Map_Object_Descr *descr);
	virtual ~PlayerImmovable();

	inline Player *get_owner() const { return m_owner; }
	inline Economy *get_economy() const { return m_economy; }

	virtual Flag *get_base_flag() = 0;

	virtual void set_economy(Economy *e);

	virtual void add_worker(Worker *w);
	virtual void remove_worker(Worker *w);

	const std::vector<Worker*>& get_workers() const { return m_workers; }

protected:
	void set_owner(Player *owner);

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

private:
	Player		*m_owner;
	Economy		*m_economy;

	std::vector<Worker*>	m_workers;
};


#endif // included_immovable_h
