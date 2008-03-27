/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef LEGACY_H
#define LEGACY_H

#include "instances.h"

namespace Widelands {

/**
 * This namespace contains functions whose only purpose is to maintain
 * backwards compatibility with old savegames and the like.
 */
namespace Legacy {

/**
 * Deal with old AttackControllers in savegames.
 *
 * AttackControllers have been outdated since build-12 (April 2008).
 */
Map_Object::Loader* loadAttackController(Editor_Game_Base *, Map_Map_Object_Loader *, FileRead &);

/**
 * Deal with old Battle objects in savegames.
 *
 * These old Battle objects have been outdated since build-12 (April 2008).
 */
Map_Object::Loader* loadBattle(Editor_Game_Base *, Map_Map_Object_Loader *, FileRead &);

}

}

#endif // LEGACY_H
