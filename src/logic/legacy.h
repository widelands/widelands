/*
 * Copyright (C) 2008, 2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef LEGACY_H
#define LEGACY_H

#include "logic/instances.h"

namespace Widelands {
struct Tribe_Descr;

/**
 * This namespace contains functions whose only purpose is to maintain
 * backwards compatibility with old savegames and the like.
 */
namespace Legacy {

///  When loading a legacy savegame a ware/worker type index may be encoutered.
///  Back then, each ware/worker type had a fixed index. Look up the name in a
///  table and then use that name to look up the real index that the
///  ware/worker type with that name has.
Ware_Index safe_ware_index
	(const Tribe_Descr &,
	 const std::string & owner,
	 char        const * relation,
	 uint32_t            legacy_index);
Ware_Index ware_index
	(const Tribe_Descr &,
	 const std::string & owner,
	 char        const * relation,
	 uint32_t            legacy_index);
Ware_Index worker_index
	(const Tribe_Descr &,
	 const std::string & owner,
	 char        const * relation,
	 uint32_t            legacy_index);

/**
 * Deal with old AttackControllers in savegames.
 *
 * AttackControllers have been outdated since build-12 (April 2008).
 */
Map_Object::Loader * loadAttackController
	(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);

/**
 * Deal with old Battle objects in savegames.
 *
 * These old Battle objects have been outdated since build-12 (April 2008).
 */
Map_Object::Loader * loadBattle
	(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);

}

}

#endif
