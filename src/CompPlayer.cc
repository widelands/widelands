/*
 * Copyright (C) 2004 by The Widelands Development Team
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

#include "error.h"
#include "CompPlayer.h"

Computer_Player::Computer_Player (Game *g, uchar pid)
{
	m_game = g;
	m_player_number = pid;
	
	log ("new Comp_Player for player %d\n", pid);
}

Computer_Player::~Computer_Player ()
{
}

void Computer_Player::think ()
{
}

void Computer_Player::gain_building (Building* b)
{
	printf ("AI player %d gained building %s/%s\n", m_player_number, b->get_name(), b->get_descname());
}

void Computer_Player::lose_building (Building* b)
{
	printf ("AI player %d lost building %s/%s\n", m_player_number, b->get_name(), b->get_descname());
}

void Computer_Player::gain_immovable (PlayerImmovable* pi)
{
	switch (pi->get_type()) {
	case BaseImmovable::BUILDING:
		gain_building (static_cast<Building*>(pi));
		break;
	}
}

void Computer_Player::lose_immovable (PlayerImmovable* pi)
{
	switch (pi->get_type()) {
	case BaseImmovable::BUILDING:
		lose_building (static_cast<Building*>(pi));
		break;
	}
}


