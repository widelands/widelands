/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "wui/soldier_statistics_menu.h"

#include "logic/map_objects/tribes/soldier.h"
#include "ui_basic/box.h"

SoldierStatisticsMenu::SoldierStatisticsMenu(InteractivePlayer& parent,
                                             UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "soldier_statistics", &registry, 100, 100, _("Soldier Statistics")),
     player_(parent.player()) {
	UI::Box* vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	// To optimize the layout, we arrange Attack and Evade level gradients horizontally
	// and Health and Defense level gradients vertically
	const Widelands::SoldierDescr& soldier = dynamic_cast<const Widelands::SoldierDescr&>(
	   *player_.tribe().get_worker_descr(player_.tribe().soldier()));
	ma_ = soldier.get_max_attack_level();
	md_ = soldier.get_max_defense_level();
	mh_ = soldier.get_max_health_level();
	me_ = soldier.get_max_evade_level();

	for (unsigned health = 0; health <= mh_; ++health) {
		for (unsigned defense = 0; defense <= md_; ++defense) {
			UI::Box* hbox1 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox2 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox3 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			for (unsigned attack = 0; attack <= ma_; ++attack) {
				for (unsigned evade = 0; evade <= me_; ++evade) {
					if (attack || evade) {
						hbox1->add_space(8);
						hbox2->add_space(8);
						hbox3->add_inf_space();
					}
					UI::Icon* icon1 = new UI::Icon(hbox1, soldier.get_attack_level_pic(attack));
					UI::Icon* icon2 = new UI::Icon(hbox1, soldier.get_defense_level_pic(defense));
					UI::Icon* icon3 = new UI::Icon(hbox2, soldier.get_health_level_pic(health));
					UI::Icon* icon4 = new UI::Icon(hbox2, soldier.get_evade_level_pic(evade));
					UI::Textarea* txt = new UI::Textarea(hbox3, "", UI::Align::kCenter);
					txt->set_fixed_width(2 * icon1->get_w());
					hbox1->add(icon1, UI::Box::Resizing::kAlign);
					hbox1->add(icon2, UI::Box::Resizing::kAlign);
					hbox2->add(icon3, UI::Box::Resizing::kAlign);
					hbox2->add(icon4, UI::Box::Resizing::kAlign);
					hbox3->add(txt, UI::Box::Resizing::kAlign);
					icons_.push_back(icon1);
					icons_.push_back(icon2);
					icons_.push_back(icon3);
					icons_.push_back(icon4);
					labels_.push_back(txt);
				}
			}
			if (health || defense) {
				vbox->add_space(8);
			}
			vbox->add(hbox1, UI::Box::Resizing::kFullSize);
			vbox->add(hbox2, UI::Box::Resizing::kFullSize);
			vbox->add(hbox3, UI::Box::Resizing::kFullSize);
		}
	}

	set_center_panel(vbox);
	update();
}

void SoldierStatisticsMenu::think() {
	UI::UniqueWindow::think();
	update();
}

void SoldierStatisticsMenu::update() {
	unsigned index = 0;
	for (unsigned h = 0; h <= mh_; ++h) {
		for (unsigned d = 0; d <= md_; ++d) {
			for (unsigned a = 0; a <= ma_; ++a) {
				for (unsigned e = 0; e <= me_; ++e) {
					const uint32_t nr = player_.count_soldiers(h, a, d, e);
					labels_[index]->set_text(std::to_string(nr));
					for (uint8_t i = 0; i < 4; ++i) {
						icons_[index * 4 + i]->set_grey_out(nr == 0);
					}
					++index;
				}
			}
		}
	}
}
