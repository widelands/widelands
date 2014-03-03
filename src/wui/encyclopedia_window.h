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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef ENCYCLOPEDIA_WINDOW_H
#define ENCYCLOPEDIA_WINDOW_H

#include "logic/ware_descr.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

namespace Widelands {
struct WareDescr;
struct Tribe_Descr;
};

struct Interactive_Player;

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(Interactive_Player &, UI::UniqueWindow::Registry &);
private:
	struct Ware {
		Ware(Widelands::Ware_Index i, const Widelands::WareDescr * descr)
			:
			m_i(i),
			m_descr(descr)
			{}
		Widelands::Ware_Index m_i;
		const Widelands::WareDescr * m_descr;

		bool operator<(const Ware o) const {
			return m_descr->descname() < o.m_descr->descname();
		}
	};

	Interactive_Player & iaplayer() const;
	UI::Listselect<Widelands::Ware_Index> wares;
	UI::Listselect<Widelands::Building_Index> prodSites;
	UI::Table     <uintptr_t>                 condTable;
	UI::Multiline_Textarea    descrTxt;
	Widelands::WareDescr const * selectedWare;
	void fillWares();
	void wareSelected(uint32_t);
	void prodSiteSelected(uint32_t);
};

#endif
