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

#include <set>
#include <map>
#include <vector>
#include <string>
#include <typeinfo>

#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 550

EncyclopediaWindow::EncyclopediaWindow (Interactive_Player& plr, UI::UniqueWindow::Registry& registry) :

UI::UniqueWindow
(&plr, &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribe ware encyclopedia")),

interactivePlayer(plr),

wares(this, 5, 5, WINDOW_WIDTH - 10, WINDOW_HEIGHT - 250),

prodSites(this, 5, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140),

condTable
(this, WINDOW_WIDTH / 2, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140),

descrTxt(this, 5, WINDOW_HEIGHT - 240, WINDOW_WIDTH - 10, 80, ""),

tribe(&interactivePlayer.get_player()->tribe())

{
	wares.selected.set(this, &EncyclopediaWindow::wareSelected);

	prodSites.selected.set(this, &EncyclopediaWindow::prodSiteSelected);

   condTable.add_column(_("Needs Ware"), 140); //was 240
   condTable.add_column(_("Consumed"),    80);
   condTable.add_column(_("Group"),       70);

	fillWares();

   if (get_usedefaultpos())
		center_to_parent();
}

EncyclopediaWindow::~EncyclopediaWindow() {
}

void EncyclopediaWindow::fillWares() {
   int nrWares = tribe->get_nrwares();
   int i;
   for (i=0;i<nrWares;i++) {
      Item_Ware_Descr* ware = tribe->get_ware_descr(i);
		wares.add(ware->descname().c_str(), i, ware->get_icon());
	}
}

void EncyclopediaWindow::wareSelected(uint) {
	selectedWare = tribe->get_ware_descr(wares.get_selected());

	descrTxt.set_text(selectedWare->get_helptext());

	prodSites.clear();
	condTable.clear();

   bool found = false;

	const Building_Descr::Index nr_buildings = tribe->get_nrbuildings();
	for (Building_Descr::Index i = 0; i < nr_buildings; ++i)
		if
			(const ProductionSite_Descr * const curProdSite =
		    dynamic_cast<const ProductionSite_Descr * const>
			 (tribe->get_building_descr(i)))
		{

			const char * const name = curProdSite->name().c_str();
      if (strcmp(name, "constructionsite") == 0) continue;
      if (strcmp(name, "headquarters")     == 0) continue;

			if
				(curProdSite->get_outputs()->find(selectedWare->name())
				 !=
				 curProdSite->get_outputs()->end())
			{
				prodSites.add
					(curProdSite->descname().c_str(), i, curProdSite->get_buildicon());
            found = true;
			}
		}
   if (found) prodSites.select(0);

}

void EncyclopediaWindow::prodSiteSelected(uint) {
	assert(prodSites.has_selection());
	condTable.clear();

	const ProductionSite_Descr::ProgramMap & program_map =
		static_cast<const ProductionSite_Descr &>
		(*tribe->get_building_descr(prodSites.get_selected()))
		.get_all_programs();

	//  FIXME This needs reworking. A program can indeed produce iron even if the
	//  FIXME program name is not any of produce_iron, smelt_iron, prog_iron or
	//  FIXME work. What matters is whether the program has a statement such as
	//  FIXME "produce iron" or "createitem iron". The program name is not
	//  FIXME supposed to have any meaning to the game logic except to uniquely
	//  FIXME identify the program.
	std::map<std::string, ProductionProgram*>::const_iterator programIt =
		program_map.find(std::string("produce_") + selectedWare->name());

   uint i;

	if (programIt == program_map.end())
		programIt =
			program_map.find(std::string("smelt_") + selectedWare->name());

	if (programIt == program_map.end())
		programIt = program_map.find(std::string("prog_") + selectedWare->name());

	if (programIt == program_map.end()) programIt = program_map.find("work");

	if (programIt != program_map.end()) {
      std::vector<ProductionAction> actions = programIt->second->get_all_actions();

      std::map<std::string, WareCondition> waresConsumed;
      std::map<std::string, WareCondition> waresChecked;

      int consumeGroup = 0;
      int checkGroup = 0;

      for (i=0; i<actions.size(); i++) {
         //some actions are noted as "consume ware1, ware2"
         std::vector<std::string> splitWares;
			split_string(actions[i].sparam1, splitWares, ",");

         bool isGrouped = false;

         if (splitWares.size() > 1) {
            isGrouped = true;
            if (actions[i].type == ProductionAction::actCheck)
               checkGroup++;
            else if (actions[i].type == ProductionAction::actConsume)
               consumeGroup++;
			}

         uint j;
         if (actions[i].type == ProductionAction::actConsume) {
            for (j=0;j<splitWares.size();j++) {
               WareCondition wc = {
                  actions[i].iparam1,
                  isGrouped,
                  consumeGroup
					};
               waresConsumed[splitWares[j]] = wc;
				}
			}
         else if (actions[i].type == ProductionAction::actCheck) {
            for (j=0;j<splitWares.size();j++) {
               WareCondition wc = {
                  actions[i].iparam1,
                  isGrouped,
                  checkGroup
					};
               waresChecked[splitWares[j]] = wc;
				}
			}
		}

      i = 0;
      for (std::map<std::string, WareCondition>::iterator waresCheckedIt=waresChecked.begin();
            waresCheckedIt!=waresChecked.end(); waresCheckedIt++) {

         std::map<std::string, WareCondition>::iterator waresConsumedIt = waresConsumed.find(waresCheckedIt->first);
         createCondTableEntry(i, waresCheckedIt->first.c_str(), (waresConsumedIt != waresConsumed.end()), &waresCheckedIt->second);

         if (waresConsumedIt != waresConsumed.end()) {
            waresConsumed.erase(waresConsumedIt);
			}
         i++;
		}

      for (std::map<std::string, WareCondition>::iterator waresConsumedIt=waresConsumed.begin();
            waresConsumedIt!=waresConsumed.end(); waresConsumedIt++) {

         createCondTableEntry(i, waresConsumedIt->first.c_str(), true, &waresConsumedIt->second);
         i++;
		}
	}

}

void EncyclopediaWindow::createCondTableEntry(int index, std::string wareName, bool consumed, WareCondition* wareCondition) {
   Item_Ware_Descr* curWare = tribe->get_ware_descr(tribe->get_safe_ware_index(wareName.c_str()));

	UI::Table<uintptr_t>::Entry_Record & tableEntry =
		condTable.add(index, curWare->get_icon());
   std::string rowText = curWare->descname();
   std::string consumeAmount = "0";
   std::string groupId = "";

	if (consumed) {
		char buffer[5];
		snprintf(buffer, sizeof(buffer), "%i", wareCondition->amount);
		consumeAmount = buffer;
	}

   if (wareCondition->isGrouped) {
      int k;
      for (k=0;k<wareCondition->groupId;k++) {
         groupId+="*";
		}
	}

	tableEntry.set_string(0, rowText);
	tableEntry.set_string(1, consumeAmount);
	tableEntry.set_string(2, groupId);
}
