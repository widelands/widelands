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

#include <set>
#include <map>
#include <vector>
#include <string>
#include <typeinfo>

#include "encyclopedia_window.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "ui_window.h"
#include "ui_unique_window.h"
#include "ui_table.h"
#include "tribe.h"
#include "ware.h"
#include "player.h"
#include "building.h"
#include "productionsite.h"
#include "production_program.h"
#include "helper.h"

#define WINDOW_WIDTH	600
#define WINDOW_HEIGHT 550

EncyclopediaWindow::EncyclopediaWindow (Interactive_Player& plr,UI::UniqueWindow::Registry& registry) :
UI::UniqueWindow(&plr, &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribe ware encyclopedia")), interactivePlayer(plr) {
   tribe = interactivePlayer.get_player()->get_tribe();
   
   waresTable=new UI::Table(this, 5, 5, WINDOW_WIDTH - 10, WINDOW_HEIGHT - 250, Align_Left, UI::Table::UP);
   waresTable->add_column(_("Ware").c_str(), UI::Table::STRING, WINDOW_WIDTH);
   waresTable->selected.set(this, &EncyclopediaWindow::wareSelected);

   descrTxt = new UI::Multiline_Textarea(this,5, WINDOW_HEIGHT - 240, WINDOW_WIDTH - 10, 80, "", Align_Left);

	prodSitesTable = new UI::Table(this, 5, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140, Align_Left, UI::Table::UP);
	prodSitesTable->add_column(_("Is Produced by").c_str(), UI::Table::STRING, 240);
	prodSitesTable->selected.set(this, &EncyclopediaWindow::prodSiteSelected);

	condTable = new UI::Table(this, WINDOW_WIDTH / 2, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140, Align_Left, UI::Table::UP);
   condTable->add_column(_("Needs Ware").c_str(), UI::Table::STRING, 140); //was 240
   condTable->add_column(_("Consumed").c_str(),UI::Table::STRING,80);
   condTable->add_column(_("Group").c_str(),UI::Table::STRING,70);
   
   fillWaresTable();

   if (get_usedefaultpos())
		center_to_parent();
}

EncyclopediaWindow::~EncyclopediaWindow() {
}

void EncyclopediaWindow::fillWaresTable() {
   int nrWares = tribe->get_nrwares();
   int i;
   for (i=0;i<nrWares;i++) {
      Item_Ware_Descr* ware = tribe->get_ware_descr(i);
      UI::Table_Entry* tableEntry = new UI::Table_Entry(waresTable,reinterpret_cast<void * const>(i),ware->get_menu_pic());
      tableEntry->set_string(0,ware->get_descname());
   }
}

void EncyclopediaWindow::wareSelected(int selectedRow) {
   this->tribe = interactivePlayer.get_player()->get_tribe();
	const uintptr_t index = reinterpret_cast<const uintptr_t>
		(waresTable->get_entry(selectedRow)->get_user_data());
   
   this->selectedWare = tribe->get_ware_descr(index);

   descrTxt->set_text(selectedWare->get_helptext());

   prodSitesTable->clear();
   condTable->clear();

   int i;
   bool found = false;

   for (i = 0; i < tribe->get_nrbuildings(); ++i) {
      Building_Descr* curBuilding = tribe->get_building_descr((uint)i);

      const char * const name = curBuilding->get_name();
      if (strcmp(name, "constructionsite") == 0) continue;
      if (strcmp(name, "headquarters")     == 0) continue;

      if (typeid(*curBuilding)==typeid(ProductionSite_Descr)) {
         ProductionSite_Descr* curProdSite = (ProductionSite_Descr*) curBuilding;
         
         std::set<std::string>::iterator it = curProdSite->get_outputs()->find(selectedWare->get_name());
         if (it != curProdSite->get_outputs()->end()) {
            UI::Table_Entry* tableEntry = new UI::Table_Entry(prodSitesTable,reinterpret_cast<void * const>(i),curProdSite->get_buildicon());
            tableEntry->set_string(0,curProdSite->get_descname());
            found = true;
         }
      }
   }
   if (found)
      prodSitesTable->select(0);

}

void EncyclopediaWindow::prodSiteSelected(int selectedRow) {
   condTable->clear();
	const uintptr_t index = reinterpret_cast<const uintptr_t>
		(prodSitesTable->get_entry(selectedRow)->get_user_data());
   ProductionSite_Descr* curProdSite = (ProductionSite_Descr*)tribe->get_building_descr(index);
   
   std::map<std::string, ProductionProgram*> map = curProdSite->get_all_programs();
  
   std::string programName = "produce_";
   programName += selectedWare->get_name();
   std::map<std::string, ProductionProgram*>::iterator programIt = map.find(programName);
   
   uint i;
   
   if (programIt == map.end()) {
      programIt = map.find("work");
   }
   
   if (programIt != map.end()) {
      std::vector<ProductionAction> actions = programIt->second->get_all_actions();
      
      std::map<std::string,WareCondition> waresConsumed;
      std::map<std::string,WareCondition> waresChecked;
      
      int group = 0;
     
      for (i=0; i<actions.size(); i++) {
         //some actions are noted as "consume ware1,ware2"
         std::vector<std::string> splitWares;
         split_string(actions[i].sparam1,&splitWares,",");
         
         bool isGrouped = false;
         
         if (splitWares.size() > 1 && actions[i].type == ProductionAction::actCheck) {
            isGrouped = true;
            group++;
         }
         
         uint j;
         if (actions[i].type == ProductionAction::actConsume) {
            for (j=0;j<splitWares.size();j++) {
               WareCondition wc = {
                  actions[i].iparam1,
                  isGrouped,
                  group
               };
               waresConsumed[splitWares[j]] = wc;
            }
         }
         else if (actions[i].type == ProductionAction::actCheck) {
            for (j=0;j<splitWares.size();j++) {
               WareCondition wc = {
                  actions[i].iparam1,
                  isGrouped,
                  group
               };
               waresChecked[splitWares[j]] = wc;
            }
         }
      }
      
      i = 0;
      for (std::map<std::string,WareCondition>::iterator waresCheckedIt=waresChecked.begin(); 
            waresCheckedIt!=waresChecked.end(); waresCheckedIt++) {
            
         std::map<std::string,WareCondition>::iterator waresConsumedIt = waresConsumed.find(waresCheckedIt->first);
         createCondTableEntry(i,waresCheckedIt->first.c_str(),(waresConsumedIt != waresConsumed.end()),&waresCheckedIt->second);
         
         if (waresConsumedIt != waresConsumed.end()) {
            waresConsumed.erase(waresConsumedIt);
         }
         i++;
      }
      
      for (std::map<std::string,WareCondition>::iterator waresConsumedIt=waresConsumed.begin(); 
            waresConsumedIt!=waresConsumed.end(); waresConsumedIt++) {
         
         createCondTableEntry(i,waresConsumedIt->first.c_str(),true,&waresConsumedIt->second);
         i++;
      }
   }

}

void EncyclopediaWindow::createCondTableEntry(int index, std::string wareName, bool consumed, WareCondition* wareCondition) {
   Item_Ware_Descr* curWare = tribe->get_ware_descr(tribe->get_safe_ware_index(wareName.c_str()));
   
   UI::Table_Entry* tableEntry = new UI::Table_Entry(condTable,reinterpret_cast<void * const>(index),curWare->get_menu_pic());
   std::string rowText = curWare->get_descname();
   std::string consumeAmount = "0";
   std::string groupId = "";
   
   if (consumed) {
      char buf[5];
      snprintf(buf, sizeof(buf), "%i", wareCondition->amount);
      consumeAmount = buf;
   }
   
   if (wareCondition->isGrouped) {
      int k;
      for (k=0;k<wareCondition->groupId;k++) {
         groupId+="*";
      }
   }
   
   tableEntry->set_string(0,rowText.c_str());
   tableEntry->set_string(1,consumeAmount.c_str());
   tableEntry->set_string(2,groupId.c_str());
}
