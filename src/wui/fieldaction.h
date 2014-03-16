/*
 * Copyright (C) 2002, 2008-2009 by The Widelands Development Team
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

#ifndef FIELDACTION_H
#define FIELDACTION_H

#include "ui_basic/unique_window.h"

namespace Widelands {
class Building;
class Player;
struct PlayerImmovable;
}
struct Interactive_Base;
struct Interactive_Player;

// fieldaction.cc
void show_field_action
	(Interactive_Base           * parent,
	 Widelands::Player          * player,
	 UI::UniqueWindow::Registry * registry);

#endif
