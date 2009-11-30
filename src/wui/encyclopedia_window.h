/*
 * Copyright (C) 2002-2004, 2006, 2009 by the Widelands Development Team
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

#ifndef ENCYCLOPEDIA_WINDOW_H
#define ENCYCLOPEDIA_WINDOW_H

#include "graphic/graphic.h"

#include "ui_basic/listselect.h"
#include "ui_basic/window.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/table.h"
#include "ui_basic/multilinetextarea.h"

namespace Widelands {
struct Item_Ware_Descr;
struct Tribe_Descr;
};

struct Interactive_Player;

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(Interactive_Player &, UI::UniqueWindow::Registry &);
private:
	Interactive_Player & iaplayer() const;
	UI::Listselect<Widelands::Ware_Index> wares;
	UI::Listselect<Widelands::Building_Index> prodSites;
	UI::Table     <uintptr_t>                 condTable;
	UI::Multiline_Textarea    descrTxt;
	Widelands::Item_Ware_Descr const * selectedWare;
	void fillWares();
	void wareSelected(uint32_t);
	void prodSiteSelected(uint32_t);
};

#endif
