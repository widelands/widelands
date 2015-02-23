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

#ifndef WL_WUI_ENCYCLOPEDIA_WINDOW_H
#define WL_WUI_ENCYCLOPEDIA_WINDOW_H

#include "logic/ware_descr.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

namespace Widelands {
class WareDescr;
class TribeDescr;
}

class InteractivePlayer;

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(InteractivePlayer &, UI::UniqueWindow::Registry &);
private:
	struct Ware {
		Ware(Widelands::WareIndex i, const Widelands::WareDescr * descr)
			:
			m_i(i),
			m_descr(descr)
			{}
		Widelands::WareIndex m_i;
		const Widelands::WareDescr * m_descr;

		bool operator<(const Ware o) const {
			return m_descr->descname() < o.m_descr->descname();
		}
	};

	InteractivePlayer & iaplayer() const;
	UI::Listselect<Widelands::WareIndex> wares;
	UI::Listselect<Widelands::BuildingIndex> prodSites;
	UI::Table     <uintptr_t>                 condTable;
	UI::MultilineTextarea    descrTxt;
	Widelands::WareDescr const * selectedWare;
	void fill_wares();
	void ware_selected(uint32_t);
	void prod_site_selected(uint32_t);
};

#endif  // end of include guard: WL_WUI_ENCYCLOPEDIA_WINDOW_H
