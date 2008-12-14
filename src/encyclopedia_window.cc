/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "encyclopedia_window.h"

#include "building.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "helper.h"
#include "player.h"
#include "productionsite.h"
#include "production_program.h"
#include "tribe.h"
#include "warelist.h"

#include "ui_window.h"
#include "ui_unique_window.h"
#include "ui_table.h"

#include "upcast.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <typeinfo>

#define WINDOW_WIDTH  std::min(600, g_gr->get_xres() - 40)
#define WINDOW_HEIGHT std::min(550, g_gr->get_yres() - 40)

#define WARE_PICTURE_COLUMN_WIDTH 32
#define QUANTITY_COLUMN_WIDTH 64
#define WARE_GROUPS_TABLE_WIDTH (WINDOW_WIDTH * 2 / 3 - 5)

using namespace Widelands;

EncyclopediaWindow::EncyclopediaWindow
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&plr, &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribe ware encyclopedia")),
interactivePlayer(plr),
wares            (this, 5, 5, WINDOW_WIDTH - 10, WINDOW_HEIGHT - 250),
prodSites        (this, 5, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 3 - 5, 140),
condTable
	(this,
	 WINDOW_WIDTH / 3, WINDOW_HEIGHT - 150, WARE_GROUPS_TABLE_WIDTH, 140),
descrTxt         (this, 5, WINDOW_HEIGHT - 240, WINDOW_WIDTH - 10, 80, "")
{
	wares.selected.set(this, &EncyclopediaWindow::wareSelected);

	prodSites.selected.set(this, &EncyclopediaWindow::prodSiteSelected);

	condTable.add_column (WARE_PICTURE_COLUMN_WIDTH);
	condTable.add_column
		(WARE_GROUPS_TABLE_WIDTH
		 - WARE_PICTURE_COLUMN_WIDTH
		 - QUANTITY_COLUMN_WIDTH,
		 _("Consumed ware type(s)"));
	condTable.add_column (QUANTITY_COLUMN_WIDTH, _("Quantity"));

	fillWares();

	if (get_usedefaultpos())
		center_to_parent();
}


void EncyclopediaWindow::fillWares() {
	Tribe_Descr const & tribe = interactivePlayer.get_player()->tribe();
	Ware_Index const nr_wares = tribe.get_nrwares();
	for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
		Item_Ware_Descr const & ware = *tribe.get_ware_descr(i);
		wares.add(ware.descname().c_str(), i, ware.icon());
	}
}

void EncyclopediaWindow::wareSelected(uint32_t) {
	Tribe_Descr const & tribe = interactivePlayer.get_player()->tribe();
	selectedWare = tribe.get_ware_descr(wares.get_selected());

	descrTxt.set_text(selectedWare->helptext());

	prodSites.clear();
	condTable.clear();

	bool found = false;

	Building_Index const nr_buildings = tribe.get_nrbuildings();
	for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i)
		if (upcast(ProductionSite_Descr const, de, tribe.get_building_descr(i)))
		{

			const char * const name = de->name().c_str();
			if
				(strcmp(name, "constructionsite") and strcmp(name, "headquarters")
				 and
				 de->output().find(wares.get_selected())
				 !=
				 de->output().end())
			{
				prodSites.add(de->descname().c_str(), i, de->get_buildicon());
				found = true;
			}
		}
	if (found)
		prodSites.select(0);

}

void EncyclopediaWindow::prodSiteSelected(uint32_t) {
	assert(prodSites.has_selection());
	condTable.clear();
	Tribe_Descr const & tribe = interactivePlayer.get_player()->tribe();

	ProductionSite_Descr::Programs const & programs =
		static_cast<const ProductionSite_Descr &>
			(*tribe.get_building_descr(prodSites.get_selected()))
		.programs();

	//  FIXME This needs reworking. A program can indeed produce iron even if
	//  FIXME the program name is not any of produce_iron, smelt_iron, prog_iron
	//  FIXME or work. What matters is whether the program has a statement such
	//  FIXME as "produce iron" or "createitem iron". The program name is not
	//  FIXME supposed to have any meaning to the game logic except to uniquely
	//  FIXME identify the program.
	//  Only shows information from the first program that has a name indicating
	//  that it produces the considered ware type.
	std::map<std::string, ProductionProgram*>::const_iterator programIt =
		programs.find(std::string("produce_") + selectedWare->name());

	if (programIt == programs.end())
		programIt = programs.find(std::string("smelt_")  + selectedWare->name());

	if (programIt == programs.end())
		programIt = programs.find(std::string("smoke_")  + selectedWare->name());

	if (programIt == programs.end())
		programIt = programs.find(std::string("mine_")   + selectedWare->name());

	if (programIt == programs.end())
		programIt = programs.find("work");

	if (programIt != programs.end()) {
		ProductionProgram::Actions const & actions =
			programIt->second->actions();

		container_iterate_const(ProductionProgram::Actions, actions, i)
			if (upcast(ProductionProgram::ActConsume const, action, *i.current)) {
				ProductionProgram::ActConsume::Groups const & groups =
					action->groups();
				container_iterate_const
					(ProductionProgram::ActConsume::Groups, groups, j)
				{
					std::set<Ware_Index> const & ware_types = j.current->first;
					assert(ware_types.size());
					std::string ware_type_names;
					for
						(struct {
						 	std::set<Ware_Index>::const_iterator       current;
						 	std::set<Ware_Index>::const_iterator const end;
						 } k = {ware_types.begin(), ware_types.end()};;)
					{
						ware_type_names +=
							tribe.get_ware_descr(*k.current)->descname();
						if (++k.current == k.end)
							break;
						ware_type_names += _(" or ");
					}

					//  Make sure to detect if someone changes the type so that it
					//  needs more than 3 decimal digits to represent.
					compile_assert
						(std::numeric_limits<typeof(j.current->second)>::max()
						 <=
						 999);
					char amount_string[4]; //  Space for 3 digits + terminator.
					sprintf(amount_string, "%u", j.current->second);

					//  picture only of first ware type in group
					UI::Table<intptr_t>::Entry_Record & tableEntry =
						condTable.add(0);
					tableEntry.set_picture
						(0, tribe.get_ware_descr(*ware_types.begin())->icon());
					tableEntry.set_string (1, ware_type_names);
					tableEntry.set_string (2, amount_string);
					condTable.set_sort_column(1);
					condTable.sort();
				}
			}
	}
}
