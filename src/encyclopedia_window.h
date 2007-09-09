/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__ENCYCLOPEDIA_WINDOW_H
#define __S__ENCYCLOPEDIA_WINDOW_H

#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"

#include "ui_listselect.h"
#include "ui_window.h"
#include "ui_unique_window.h"
#include "ui_table.h"
#include "ui_multilinetextarea.h"

#include <string>

class Item_Ware_Descr;
class Tribe_Descr;

struct WareCondition {
	uint amount;
   bool isGrouped;
	uint groupId;
};

struct EncyclopediaWindow : public UI::UniqueWindow {
	  EncyclopediaWindow(Interactive_Player&, UI::UniqueWindow::Registry&);
	  ~EncyclopediaWindow();
private:
	  Interactive_Player& interactivePlayer;
	UI::Listselect<uintptr_t> wares;
	UI::Listselect<uintptr_t> prodSites;
	UI::Table     <uintptr_t> condTable;
	UI::Multiline_Textarea    descrTxt;
	  const Item_Ware_Descr* selectedWare;
	  const Tribe_Descr* tribe;
	void fillWares();
	void wareSelected(uint);
	void prodSiteSelected(uint);
	void createCondTableEntry
		(const uint            index,
		 const std::string   & wareName,
		 const bool            consumed,
		 const WareCondition & wareCondition);
};
#endif
