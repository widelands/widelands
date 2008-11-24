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

/// \file
/// This is a place where the user interface links into the game logic code.

#include "editor/ui_menus/event_allow_building_option_menu.h"
#include "editor/ui_menus/event_conquer_area_option_menu.h"
#include "editor/ui_menus/event_message_box_option_menu.h"
#include "editor/ui_menus/event_move_view_option_menu.h"
#include "editor/ui_menus/event_unhide_area_option_menu.h"

#include "event_allow_building.h"
#include "event_building.h"
#include "event_conquer_area.h"
#include "event_flag.h"
#include "event_immovable.h"
#include "event_message_box.h"
#include "event_move_view.h"
#include "event_reveal_campaign.h"
#include "event_reveal_objective.h"
#include "event_reveal_scenario.h"
#include "event_road.h"
#include "event_set_timer.h"
#include "event_unhide_area.h"

namespace Widelands {

int32_t Event_Allow_Building            ::option_menu(Editor_Interactive & eia)
{Event_Allow_Building_Option_Menu               m(eia, *this); return m.run();}

int32_t Event_Building                  ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Conquer_Area              ::option_menu(Editor_Interactive & eia)
{Event_Conquer_Area_Option_Menu                 m(eia, *this); return m.run();}

int32_t Event_Flag                      ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Immovable                 ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Message_Box               ::option_menu(Editor_Interactive & eia)
{Event_Message_Box_Option_Menu                  m(eia, *this); return m.run();}

int32_t Event_Reveal_Campaign           ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Reveal_Objective          ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Reveal_Scenario           ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Road                      ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Set_Timer                 ::option_menu(Editor_Interactive &)
{return 0;}

int32_t Event_Move_View                 ::option_menu(Editor_Interactive & eia)
{Event_Move_View_Option_Menu                    m(eia, *this); return m.run();}

int32_t Event_Unhide_Area               ::option_menu(Editor_Interactive & eia)
{Event_Unhide_Area_Option_Menu                  m(eia, *this); return m.run();}

};
