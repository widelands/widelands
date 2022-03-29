/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/soldier_statistics_menu.h"

#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/soldier.h"
#include "wui/waresdisplay.h"

SoldierStatisticsPanel::SoldierStatisticsPanel(UI::Panel& parent,
                                               const Widelands::Player& player,
                                               const CountingFn& cfn)
   : UI::Box(&parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical), counting_function_(cfn) {
	// To optimize the layout, we arrange Attack and Evade level gradients horizontally
	// and Health and Defense level gradients vertically
	const Widelands::SoldierDescr& soldier = dynamic_cast<const Widelands::SoldierDescr&>(
	   *player.tribe().get_worker_descr(player.tribe().soldier()));
	max_attack_ = soldier.get_max_attack_level();
	max_defense_ = soldier.get_max_defense_level();
	max_health_ = soldier.get_max_health_level();
	max_evade_ = soldier.get_max_evade_level();

	for (unsigned health = 0; health <= max_health_; ++health) {
		for (unsigned defense = 0; defense <= max_defense_; ++defense) {
			UI::Box* hbox1 = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox2 = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
			UI::Box* hbox3 = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
			for (unsigned attack = 0; attack <= max_attack_; ++attack) {
				for (unsigned evade = 0; evade <= max_evade_; ++evade) {
					if ((attack != 0u) || (evade != 0u)) {
						hbox1->add_space(8);
						hbox2->add_space(8);
						hbox3->add_space(8);
					}
					UI::Icon* icon1 =
					   new UI::Icon(hbox1, UI::PanelStyle::kWui, soldier.get_attack_level_pic(attack));
					UI::Icon* icon2 =
					   new UI::Icon(hbox1, UI::PanelStyle::kWui, soldier.get_defense_level_pic(defense));
					UI::Icon* icon3 =
					   new UI::Icon(hbox2, UI::PanelStyle::kWui, soldier.get_health_level_pic(health));
					UI::Icon* icon4 =
					   new UI::Icon(hbox2, UI::PanelStyle::kWui, soldier.get_evade_level_pic(evade));
					UI::Textarea* txt = new UI::Textarea(
					   hbox3, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kCenter);
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
					const std::string tt = format("%s<br>%s<br>%s<br>%s",             //
					                              format(_("Health: %u"), health),    //
					                              format(_("Attack: %u"), attack),    //
					                              format(_("Defense: %u"), defense),  //
					                              format(_("Evade: %u"), evade));
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
			if ((health != 0u) || (defense != 0u)) {
				add_space(8);
			}
			add(hbox1, UI::Box::Resizing::kFullSize);
			add(hbox2, UI::Box::Resizing::kFullSize);
			add(hbox3, UI::Box::Resizing::kFullSize);
		}
	}

	update();
}

void SoldierStatisticsPanel::think() {
	UI::Panel::think();
	update();
}

void SoldierStatisticsPanel::update() {
	unsigned index = 0;
	for (unsigned h = 0; h <= max_health_; ++h) {
		for (unsigned d = 0; d <= max_defense_; ++d) {
			for (unsigned a = 0; a <= max_attack_; ++a) {
				for (unsigned e = 0; e <= max_evade_; ++e) {
					const uint32_t nr = counting_function_(h, a, d, e);
					labels_all_[index]->set_text(nr != 0u ? get_amount_string(nr, true) : "");
					labels_all_[index]->set_tooltip(std::to_string(nr));
					for (uint8_t i = 0; i < 4; ++i) {
						icons_all_[index * 4 + i]->set_grey_out(nr == 0);
					}
					++index;
				}
			}
		}
	}
}

SoldierStatisticsMenu::SoldierStatisticsMenu(InteractivePlayer& parent,
                                             UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "soldier_statistics",
                      &registry,
                      100,
                      100,
                      _("Soldier Statistics")),
     player_(parent.player()),
     tabs_(this, UI::TabPanelStyle::kWuiDark) {

	tabs_.add("all", _("Overview"),
	          new SoldierStatisticsPanel(
	             tabs_, player_, [this](uint32_t h, uint32_t a, uint32_t d, uint32_t e) {
		             return player_.count_soldiers(h, a, d, e);
	             }));

	const Widelands::SoldierDescr& soldier = dynamic_cast<const Widelands::SoldierDescr&>(
	   *player_.tribe().get_worker_descr(player_.tribe().soldier()));
	max_attack_ = soldier.get_max_attack_level();
	max_defense_ = soldier.get_max_defense_level();
	max_health_ = soldier.get_max_health_level();
	max_evade_ = soldier.get_max_evade_level();

	UI::Box* vbox = new UI::Box(&tabs_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);

	UI::Box* hbox1 = new UI::Box(vbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox2 = new UI::Box(vbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox3 = new UI::Box(vbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	UI::Box* hbox4 = new UI::Box(vbox, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	for (unsigned h = 0; h <= max_health_; ++h) {
		UI::Icon* i = new UI::Icon(hbox1, UI::PanelStyle::kWui, soldier.get_health_level_pic(h));
		UI::Textarea* txt = new UI::Textarea(
		   hbox1, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox1->add(i, UI::Box::Resizing::kAlign);
		hbox1->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = format(_("Health: %u"), h);
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned a = 0; a <= max_attack_; ++a) {
		UI::Icon* i = new UI::Icon(hbox2, UI::PanelStyle::kWui, soldier.get_attack_level_pic(a));
		UI::Textarea* txt = new UI::Textarea(
		   hbox2, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox2->add(i, UI::Box::Resizing::kAlign);
		hbox2->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = format(_("Attack: %u"), a);
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned d = 0; d <= max_defense_; ++d) {
		UI::Icon* i = new UI::Icon(hbox3, UI::PanelStyle::kWui, soldier.get_defense_level_pic(d));
		UI::Textarea* txt = new UI::Textarea(
		   hbox3, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox3->add(i, UI::Box::Resizing::kAlign);
		hbox3->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = format(_("Defense: %u"), d);
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}
	for (unsigned e = 0; e <= max_evade_; ++e) {
		UI::Icon* i = new UI::Icon(hbox4, UI::PanelStyle::kWui, soldier.get_evade_level_pic(e));
		UI::Textarea* txt = new UI::Textarea(
		   hbox4, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, "", UI::Align::kLeft);
		txt->set_fixed_width(8 * i->get_w());
		hbox4->add(i, UI::Box::Resizing::kAlign);
		hbox4->add(txt, UI::Box::Resizing::kAlign);
		icons_detail_.push_back(i);
		labels_detail_.push_back(txt);
		const std::string tt = format(_("Evade: %u"), e);
		i->set_handle_mouse(true);
		txt->set_handle_mouse(true);
		i->set_tooltip(tt);
		txt->set_tooltip(tt);
	}

	vbox->add(hbox1, UI::Box::Resizing::kFullSize);
	vbox->add(hbox2, UI::Box::Resizing::kFullSize);
	vbox->add(hbox3, UI::Box::Resizing::kFullSize);
	vbox->add(hbox4, UI::Box::Resizing::kFullSize);
	tabs_.add("detail", _("By Attribute"), vbox);

	set_center_panel(&tabs_);
	update();

	initialization_complete();
}

void SoldierStatisticsMenu::think() {
	UI::UniqueWindow::think();
	update();
}

void SoldierStatisticsMenu::update() {
	unsigned index = 0;
	for (unsigned h = 0; h <= max_health_; ++h) {
		const uint32_t nr = player_.count_soldiers_h(h);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text(format(_("×%u"), nr));
		++index;
	}
	for (unsigned a = 0; a <= max_attack_; ++a) {
		const uint32_t nr = player_.count_soldiers_a(a);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text(format(_("×%u"), nr));
		++index;
	}
	for (unsigned d = 0; d <= max_defense_; ++d) {
		const uint32_t nr = player_.count_soldiers_d(d);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text(format(_("×%u"), nr));
		++index;
	}
	for (unsigned e = 0; e <= max_evade_; ++e) {
		const uint32_t nr = player_.count_soldiers_e(e);
		icons_detail_[index]->set_grey_out(nr == 0);
		labels_detail_[index]->set_text(format(_("×%u"), nr));
		++index;
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& SoldierStatisticsMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_soldiers;
			r.create();
			assert(r.window);
			SoldierStatisticsMenu& m = dynamic_cast<SoldierStatisticsMenu&>(*r.window);
			m.tabs_.activate(fr.unsigned_8());
			return m;
		}
		throw Widelands::UnhandledVersionError(
		   "Soldiers Statistics Menu", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("soldiers statistics menu: %s", e.what());
	}
}
void SoldierStatisticsMenu::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(tabs_.active());
}
