/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#ifndef FIELDACTION_H
#define FIELDACTION_H

class Building;
class PlayerImmovable;
class Interactive_Base;
struct UIUniqueWindowRegistry;

// fieldaction.cc
void show_field_action(Interactive_Base *parent, Player* player, UIUniqueWindowRegistry* registry);

// building_ui.cc
void show_bulldoze_confirm(Interactive_Base* player, Building* building, PlayerImmovable* todestroy);

// battle.cc
//void show_attack_dialog((Interactive_Base* player, Building* building);

#endif /* FIELDACTION_H */
