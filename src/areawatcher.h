/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef __S__AREAWATCHER_H
#define __S__AREAWATCHER_H

#include "instances.h"
#include "player_area.h"
#include <stdint.h>

struct AreaWatcher_Descr;

/**
 * An area watcher is an abstract object that sees an area for a player. It is
 * not abstract as in abstract data type, but in the sense that it has no size
 * and does not interact with other objects. Nor is it visualized with any
 * image. The best analogy would perhaps be a geostationary remote sensing
 * satellite, but it sees as good as a person on the ground. It is used to show
 * an area to a player for a limited time. When act is called, the area watcher
 * is destroyed. Use schedule_act to determine the area watcher's lifetime.
 */
struct AreaWatcher : public Map_Object, private Player_Area<> {
	friend struct Player;
	friend struct Widelands_Map_Players_AreaWatchers_Data_Packet;
	AreaWatcher(const Player_Area<>);
	virtual int get_type() const throw () {return AREAWATCHER;}
	static AreaWatcher & create(Editor_Game_Base &, const Player_Area<>);
	virtual void act(Game *, uint32_t data);
};

#endif
