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

#include "widelands.h"
#include "graphic.h"
#include "profile.h"
#include "game.h"
#include "map.h"
#include "immovable.h"


/*
==============================================================================

BaseImmovable IMPLEMENTATION

==============================================================================
*/

/*
===============
BaseImmovable::BaseImmovable
BaseImmovable::~BaseImmovable

Base immovable creation and destruction
===============
*/
BaseImmovable::BaseImmovable(Map_Object_Descr *descr)
	: Map_Object(descr)
{
}

BaseImmovable::~BaseImmovable()
{
}


/*
===============
BaseImmovable::set_position

Associate the given field with this immovable. Recalculate if necessary.
Only call this during init.
===============
*/
void BaseImmovable::set_position(Editor_Game_Base *g, Coords c)
{
	Field *f = g->get_map()->get_field(c);

	if (f->immovable) {
		BaseImmovable *other = f->immovable;
		
		assert(other->get_size() == NONE);
		
		other->cleanup(g);
		delete other;
	}

	f->immovable = this;

	if (get_size() >= SMALL)
		g->recalc_for_field(c);
}

/*
===============
BaseImmovable::unset_position

Remove the link to the given field.
Only call this during cleanup.
===============
*/
void BaseImmovable::unset_position(Editor_Game_Base *g, Coords c)
{
	Field *f = g->get_map()->get_field(c);

	assert(f->immovable == this);
	
	f->immovable = 0;
	
	if (get_size() >= SMALL)
		g->recalc_for_field(c);
}


/*
==============================================================================

Immovable_Descr IMPLEMENTATION

==============================================================================
*/

/*
===============
Immovable_Descr::Immovable_Descr

Initialize with sane defaults
===============
*/
Immovable_Descr::Immovable_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
	m_size = BaseImmovable::NONE;
}

/*
===============
Immovable_Descr::parse

Parse an immovable from its conf file
===============
*/
void Immovable_Descr::parse(const char *directory, Profile *prof)
{
	Section *s = prof->get_safe_section("global");
	char picname[256];
	
	snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
   m_anim = g_anim.get(directory, s, picname);

	const char *string;
	 
	string = s->get_string("size", 0);
	if (string) {
		if (!strcasecmp(string, "volatile") || !strcasecmp(string, "none"))
		{
			m_size = BaseImmovable::NONE;
		}
		else if (!strcasecmp(string, "small"))
		{
			m_size = BaseImmovable::SMALL;
		}
		else if (!strcasecmp(string, "normal") || !strcasecmp(string, "medium"))
		{
			m_size = BaseImmovable::MEDIUM;
		}
		else if (!strcasecmp(string, "big"))
		{
			m_size = BaseImmovable::BIG;
		}
		else
			throw wexception("Unknown size '%s'. Possible values: none, small, medium, big", string);
	}
	
	// TODO
	s->get_string("ends_in");
	s->get_int("life_time");
	s->get_int("stock");
}

/*
===============
Immovable_Descr::create

Create an immovable of this type
===============
*/
Immovable *Immovable_Descr::create(Game *g, Coords coords)
{
	Immovable *im = new Immovable(this);
	im->m_position = coords;
	im->init(g);
	return im;
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Immovable::Immovable
Immovable::~Immovable
===============
*/
Immovable::Immovable(Immovable_Descr *descr)
	: BaseImmovable(descr)
{
	m_anim = 0;
}

Immovable::~Immovable()
{
}

/*
===============
Immovable::get_type	
Immovable::get_size
Immovable::get_passable
===============
*/
int Immovable::get_type()
{
	return IMMOVABLE;
}

int Immovable::get_size()
{
	return get_descr()->get_size();
}

bool Immovable::get_passable()
{
	return get_descr()->get_size() < BIG;
}

/*
===============
Immovable::init

Actually initialize the immovable.
===============
*/
void Immovable::init(Game *g)
{
	BaseImmovable::init(g);
	
	set_position(g, m_position);
	
	m_anim = get_descr()->get_anim();
	m_animstart = g->get_gametime();
}

/*
===============
Immovable::cleanup

Cleanup before destruction
===============
*/
void Immovable::cleanup(Game *g)
{
	unset_position(g, m_position);
	
	BaseImmovable::cleanup(g);
}

/*
===============
Immovable::draw

Draw the immovable at the given position.
coords is the field that draw() was called for.
===============
*/
void Immovable::draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos)
{
	if (!m_anim)
		return;

	dst->drawanim(pos.x, pos.y, m_anim, game->get_gametime() - m_animstart, 0);
}


/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/*
===============
PlayerImmovable::PlayerImmovable

Zero-initialize
===============
*/
PlayerImmovable::PlayerImmovable(Map_Object_Descr *descr)
	: BaseImmovable(descr)
{
	m_owner = 0;
	m_economy = 0;
}

/*
===============
PlayerImmovable::~PlayerImmovable

Cleanup
===============
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log("Building::~Building: %i workers left!\n", m_workers.size());
}

/*
===============
PlayerImmovable::set_economy

Change the economy, transfer the workers
===============
*/
void PlayerImmovable::set_economy(Economy *e)
{
	if (m_economy == e)
		return;

	for(uint i = 0; i < m_workers.size(); i++)
		m_workers[i]->set_economy(e);
	
	m_economy = e;
}

/*
===============
PlayerImmovable::add_worker

Associate the given worker with this immovable.
The worker will be transferred along to another economy, and it will be 
released when the immovable is destroyed.
This should only be called from Worker::set_location.
===============
*/
void PlayerImmovable::add_worker(Worker *w)
{
	m_workers.push_back(w);
}

/*
===============
PlayerImmovable::remove_worker

Disassociate the given worker with this building.
This should only be called from Worker::set_location.
===============
*/
void PlayerImmovable::remove_worker(Worker *w)
{
	for(uint i = 0; i < m_workers.size(); i++) {
		if (m_workers[i] == w) {
			if (i < m_workers.size()-1)
				m_workers[i] = m_workers[m_workers.size()-1];
			m_workers.pop_back();
			return;
		}
	}
	
	throw wexception("PlayerImmovable::remove_worker: not in list");
}

/*
===============
PlayerImmovable::request_success

The given request has completed successfully. You should now remove it from
the economy and delete it.
===============
*/
void PlayerImmovable::request_success(Request *req)
{
	throw wexception("request_success: unhandled");
}

/*
===============
PlayerImmovable::set_owner

Set the immovable's owner. Currently, it can only be set once.
===============
*/
void PlayerImmovable::set_owner(Player *owner)
{
	// Change these asserts once you've made really sure that changing owners
	// works (necessary for military building)
	assert(!m_owner);
	assert(owner);
	
	m_owner = owner;
}

/*
===============
PlayerImmovable::init

Initialize the immovable.
===============
*/
void PlayerImmovable::init(Editor_Game_Base *g)
{
	BaseImmovable::init(g);
}

/*
===============
PlayerImmovable::init_for_game

Initialize the immovable.
===============
*/
void PlayerImmovable::init_for_game(Game *g)
{
	BaseImmovable::init_for_game(g);
}

/*
===============
PlayerImmovable::cleanup

Release workers
===============
*/
void PlayerImmovable::cleanup(Editor_Game_Base *g)
{
	while(m_workers.size())
		m_workers[0]->set_location(0);

	BaseImmovable::cleanup(g);
}

/*
===============
PlayerImmovable::cleanup_for_game

Release workers
===============
*/
void PlayerImmovable::cleanup_for_game(Game *g)
{
	while(m_workers.size())
		m_workers[0]->set_location(0);
	
   BaseImmovable::cleanup_for_game(g);
}

