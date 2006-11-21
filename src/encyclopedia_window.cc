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

#define WINDOW_WIDTH	600
#define WINDOW_HEIGHT 550

EncyclopediaWindow::EncyclopediaWindow (Interactive_Player& plr,UI::UniqueWindow::Registry& registry) :
UI::UniqueWindow(&plr, &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Tribe ware encyclopedia")), interactivePlayer(plr) {

   waresTable=new UI::Table(this, 5, 5, WINDOW_WIDTH - 10, WINDOW_HEIGHT - 250, Align_Left, UI::Table::UP);
   waresTable->add_column(_("Ware").c_str(), UI::Table::STRING, WINDOW_WIDTH);
   waresTable->selected.set(this, &EncyclopediaWindow::wareSelected);

   descrTxt = new UI::Multiline_Textarea(this,5, WINDOW_HEIGHT - 240, WINDOW_WIDTH - 10, 80, "", Align_Left);

	prodSitesTable = new UI::Table(this, 5, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140, Align_Left, UI::Table::UP);
	prodSitesTable->add_column(_("Is Produced by").c_str(), UI::Table::STRING, 240);
	prodSitesTable->selected.set(this, &EncyclopediaWindow::prodSiteSelected);

	condTable = new UI::Table(this, WINDOW_WIDTH / 2, WINDOW_HEIGHT - 150, WINDOW_WIDTH / 2 - 5, 140, Align_Left, UI::Table::UP);
   condTable->add_column(_("Needs Ware").c_str(), UI::Table::STRING, 240);

   fillWaresTable();

   if (get_usedefaultpos())
		center_to_parent();
}

EncyclopediaWindow::~EncyclopediaWindow() {
}

void EncyclopediaWindow::fillWaresTable() {
   const Tribe_Descr* tribe = interactivePlayer.get_player()->get_tribe();
   int nrWares = tribe->get_nrwares();
   int i;
   for (i=0;i<nrWares;i++) {
      Item_Ware_Descr* ware = tribe->get_ware_descr(i);
      UI::Table_Entry* tableEntry = new UI::Table_Entry(waresTable,reinterpret_cast<void * const>(i),ware->get_menu_pic());
      tableEntry->set_string(0,ware->get_descname());
   }
}

void EncyclopediaWindow::wareSelected(int selectedRow) {
   const Tribe_Descr* tribe = interactivePlayer.get_player()->get_tribe();
   int index = reinterpret_cast<int>(waresTable->get_entry(selectedRow)->get_user_data());
   Item_Ware_Descr* ware = tribe->get_ware_descr(index);

   descrTxt->set_text(ware->get_helptext());

   prodSitesTable->clear();
   condTable->clear();

   int i;
   log("Encyclopedia: searching ware: %s\n",ware->get_name());
   bool found = false;

   for (i = 0; i < tribe->get_nrbuildings(); ++i) {
      Building_Descr* curBuilding = tribe->get_building_descr((uint)i);

      const char * const name = curBuilding->get_name();
      if (strcmp(name, "constructionsite") == 0) continue;
      if (strcmp(name, "headquarters")     == 0) continue;

      if (typeid(*curBuilding)==typeid(ProductionSite_Descr)) {
         ProductionSite_Descr* curProdSite = (ProductionSite_Descr*) curBuilding;

         std::set<std::string>::iterator it = curProdSite->get_outputs()->find(ware->get_name());
         if (it != curProdSite->get_outputs()->end()) {
            log("Encyclopedia: ware found ware at: %s\n",curProdSite->get_descname());
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
   const Tribe_Descr* tribe = interactivePlayer.get_player()->get_tribe();
   int index = reinterpret_cast<int>(prodSitesTable->get_entry(selectedRow)->get_user_data());
   ProductionSite_Descr* curProdSite = (ProductionSite_Descr*)tribe->get_building_descr(index);
   std::vector<Input> inputs = *curProdSite->get_inputs();
   uint i;
   for (i=0;i<inputs.size();i++) {
      UI::Table_Entry* tableEntry = new UI::Table_Entry(condTable,reinterpret_cast<void * const>(i),inputs[i].get_ware()->get_menu_pic());
      tableEntry->set_string(0,inputs[i].get_ware()->get_descname());
   }

}
