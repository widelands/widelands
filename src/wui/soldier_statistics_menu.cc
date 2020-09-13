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
#include "ui_basic/tabpanel.h"
#include "wui/waresdisplay.h"

SoldierStatisticsMenu::SoldierStatisticsMenu(InteractivePlayer& parent,
                                             UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, UI::WindowStyle::kWui, "soldier_statistics", &registry, 100, 100, _("Soldier Statistics")),
     player_(parent.player()) {
	UI::TabPanel* tabs = new UI::TabPanel(this, UI::TabPanelStyle::kWuiDark);

	UI::Box* vbox = new UI::Box(tabs, 0, 0, UI::Box::Vertical);

	// To optimize the layout, we arrange Attack and Evade level gradients horizontally
	// and Health and Defense level gradients vertically
	const Widelands::SoldierDescr& soldier = dynamic_cast<const Widelands::SoldierDescr&>(
	   *player_.tribe().get_worker_descr(player_.tribe().soldier()));
	max_attack_ = soldier.get_max_attack_level();
	max_defense_ = soldier.get_max_defense_level();
	max_health_ = soldier.get_max_health_level();
	max_evade_ = soldier.get_max_evade_level();

	for (unsigned health = 0; health <= max_health_; ++health) {
		for (unsigned defense = 0; defense <= max_defense_; ++defense) {
			UI::Box* hbox1 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox2 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox3 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
			for (unsigned attack = 0; attack <= max_attack_; ++attack) {
				for (unsigned evade = 0; evade <= max_evade_; ++evade) {
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
					icons_all_.push_back(icon1);
					icons_all_.push_back(icon2);
					icons_all_.push_back(icon3);
					icons_all_.push_back(icon4);
					labels_all_.push_back(txt);
					const std::string tt = (boost::format("%s<br>%s<br>%s<br>%s") %
					                        (boost::format(_("Health: %u")) % health).str() %
					                        (boost::format(_("Attack: %u")) % attack).str() %
					                        (boost::format(_("Defense: %u")) % defense).str() %
					                        (boost::format(_("Evade: %u")) % evade).str())
					                          .str();
					txt->set_handle_mouse(true);
					icon1->set_tooltip(tt);
					icon2->set_tooltip(tt);
					icon3->set_tooltip(tt);
					icon4->set_tooltip(tt);
					icon1->set_handle_mouse(true);
					icon2->set_handle_mouse(true);
					icon3->set_handle_mouse(true);
					icon4->set_handle_mouse(true);
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

	tabs->add("all", _("Overview"), vbox);

	vbox = new UI::Box(tabs, 0, 0, UI::Box::Vertical);

	UI::Box* hbox1 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox2 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox3 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox4 = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);

	for (unsigned h = 0; h <= max_health_; ++h) {
		UI::Icon* i = new UI::Icon(hbox1, soldier.get_health_level_pic(h));
		UI::Textarea* txt = new UI::Textarea(hbox1, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox1->add(i, UI::Box::Resizing::kAlign);
		hbox1->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = (boost::format(_("Health: %u")) % h).str();
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned a = 0; a <= max_attack_; ++a) {
		UI::Icon* i = new UI::Icon(hbox2, soldier.get_attack_level_pic(a));
		UI::Textarea* txt = new UI::Textarea(hbox2, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox2->add(i, UI::Box::Resizing::kAlign);
		hbox2->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = (boost::format(_("Attack: %u")) % a).str();
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned d = 0; d <= max_defense_; ++d) {
		UI::Icon* i = new UI::Icon(hbox3, soldier.get_defense_level_pic(d));
		UI::Textarea* txt = new UI::Textarea(hbox3, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox3->add(i, UI::Box::Resizing::kAlign);
		hbox3->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = (boost::format(_("Defense: %u")) % d).str();
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned e = 0; e <= max_evade_; ++e) {
		UI::Icon* i = new UI::Icon(hbox4, soldier.get_evade_level_pic(e));
		UI::Textarea* txt = new UI::Textarea(hbox4, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox4->add(i, UI::Box::Resizing::kAlign);
		hbox4->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = (boost::format(_("Evade: %u")) % e).str();
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}

	vbox->add(hbox1, UI::Box::Resizing::kFullSize);
	vbox->add(hbox2, UI::Box::Resizing::kFullSize);
	vbox->add(hbox3, UI::Box::Resizing::kFullSize);
	vbox->add(hbox4, UI::Box::Resizing::kFullSize);
	tabs->add("detail", _("By Attribute"), vbox);

	set_center_panel(tabs);
	update();
}

void SoldierStatisticsMenu::think() {
	UI::UniqueWindow::think();
	update();
}

void SoldierStatisticsMenu::update() {
	unsigned index = 0;
	for (unsigned h = 0; h <= max_health_; ++h) {
		for (unsigned d = 0; d <= max_defense_; ++d) {
			for (unsigned a = 0; a <= max_attack_; ++a) {
				for (unsigned e = 0; e <= max_evade_; ++e) {
					const uint32_t nr = player_.count_soldiers(h, a, d, e);
					labels_all_[index]->set_text(nr ? get_amount_string(nr, true) : "");
					labels_all_[index]->set_tooltip(std::to_string(nr));
					for (uint8_t i = 0; i < 4; ++i) {
						icons_all_[index * 4 + i]->set_grey_out(nr == 0);
					}
					++index;
				}
			}
		}
	}

	index = 0;
	for (unsigned h = 0; h <= max_health_; ++h) {
		const uint32_t nr = player_.count_soldiers_h(h);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text((boost::format(_("×%u")) % nr).str());
		++index;
	}
	for (unsigned a = 0; a <= max_attack_; ++a) {
		const uint32_t nr = player_.count_soldiers_a(a);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text((boost::format(_("×%u")) % nr).str());
		++index;
	}
	for (unsigned d = 0; d <= max_defense_; ++d) {
		const uint32_t nr = player_.count_soldiers_d(d);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text((boost::format(_("×%u")) % nr).str());
		++index;
	}
	for (unsigned e = 0; e <= max_evade_; ++e) {
		const uint32_t nr = player_.count_soldiers_e(e);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text((boost::format(_("×%u")) % nr).str());
		++index;
	}
}
