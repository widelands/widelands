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

#ifndef included_immovable_h
#define included_immovable_h

#include "instances.h"
#include "animation.h"

class Profile;
class Request;
class Flag;
class Economy;
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
	
	virtual void draw(Game* game, RenderTarget* dst, FCoords coords, int posx, int posy) = 0;
	
protected:
	void set_position(Game *g, Coords c);
	void unset_position(Game *g, Coords c);
};

class Immovable;

/*
Immovable represents a standard immovable such as trees or stones.
*/
class Immovable_Descr : public Map_Object_Descr {
public:
	Immovable_Descr(const char *name);

	inline const char* get_name(void) { return m_name; }
	inline Animation* get_anim(void) { return &m_anim; }
	inline int get_size(void) { return m_size; }
		
	void parse(const char *directory, Profile *s);
	Immovable *create(Game *g, Coords coords);
	
protected:
	char			m_name[30];
	Animation	m_anim; // the default animation
	int			m_size;
};

class Immovable : public BaseImmovable {
	friend class Immovable_Descr;

	MO_DESCR(Immovable_Descr);

public:
	Immovable(Immovable_Descr *descr);
	~Immovable();
	
	int get_type();
	int get_size();
	bool get_passable();
	
	void init(Game *g);
	void cleanup(Game *g);

	void draw(Game* game, RenderTarget* dst, FCoords coords, int posx, int posy);

protected:
	Coords				m_position;
	
	Animation			*m_anim;
	int					m_animstart;
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
	
	void add_worker(Worker *w);
	void remove_worker(Worker *w);

	virtual void request_success(Request *req);

protected:
	void set_owner(Player *owner);
	
	virtual void init(Game *g);
	virtual void cleanup(Game *g);
		
private:
	Player		*m_owner;
	Economy		*m_economy;
	
	std::vector<Worker*>	m_workers;
};


#endif // included_immovable_h
