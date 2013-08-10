/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "wui/garrisonpanel.h"

#include <boost/signal.hpp>

#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "graphic/font.h"
#include "graphic/graphic.h"
#include "upcast.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_gamebase.h"
#include "wui/soldiercapacitycontrol.h"
#include "wui/soldierlist.h"

using Widelands::Soldier;

namespace UI {

GarrisonPanel::GarrisonPanel
	(UI::Panel & parent,
	 Interactive_GameBase & igb,
	 Widelands::Garrison & garrison)
:
UI::Box(&parent, 0, 0, UI::Box::Vertical),

m_igb(igb),
m_garrison(garrison),
m_soldierpanel(*this, igb.egbase(), garrison),
m_infotext(this, _("Click soldier to send away"))
{
	add(&m_soldierpanel, UI::Box::AlignCenter);

	add_space(2);

	add(&m_infotext, UI::Box::AlignCenter);

	m_soldierpanel.set_mouseover(boost::bind(&GarrisonPanel::mouseover, this, _1));
	m_soldierpanel.set_click(boost::bind(&GarrisonPanel::eject, this, _1));

	const UI::TextStyle & style = TextStyle::ui_small();
	// Note the extra character in the HP: string below to fix bug 724169
	uint32_t maxtextwidth = std::max
		(style.calc_bare_width(_("Click soldier to send away")),
		 style.calc_bare_width("HP: 8/8  AT: 8/8  DE: 8/8  EV: 8/8_"));
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	if (!garrison.is_passive()) {
		m_soldier_preference.add_button
			(buttons, Point(0, 0), g_gr->images().get("pics/prefer_rookies.png"), _("Prefer Rookies"));
		m_soldier_preference.add_button
			(buttons, Point(32, 0), g_gr->images().get("pics/prefer_heroes.png"), _("Prefer Heroes"));
		UI::Radiobutton* button = m_soldier_preference.get_first_button();
		while (button) {
			buttons->add(button, AlignLeft);
			button = button->next_button();
		}

		m_soldier_preference.set_state(0);
		if (m_garrison.get_soldier_preference() == Widelands::Garrison::SoldierPref::Heroes) {
			m_soldier_preference.set_state(1);
		}
		m_soldier_preference.changedto.connect
			(boost::bind(&GarrisonPanel::set_soldier_preference, this, _1));
	}
	buttons->add_inf_space();
	buttons->add
		(new SoldierCapacityControl(buttons, igb, m_garrison),
		 UI::Box::AlignRight);

	add(buttons, UI::Box::AlignCenter, true);
}

void GarrisonPanel::mouseover(const Soldier * soldier)
{
	if (!soldier) {
		m_infotext.set_text(_("Click soldier to send away"));
		return;
	}

	uint32_t const  hl = soldier->get_hp_level         ();
	uint32_t const mhl = soldier->get_max_hp_level     ();
	uint32_t const  al = soldier->get_attack_level     ();
	uint32_t const mal = soldier->get_max_attack_level ();
	uint32_t const  dl = soldier->get_defense_level    ();
	uint32_t const mdl = soldier->get_max_defense_level();
	uint32_t const  el = soldier->get_evade_level      ();
	uint32_t const mel = soldier->get_max_evade_level  ();

	char buffer[5 * 30];
	snprintf
		(buffer, sizeof(buffer),
		 "HP: %u/%u  AT: %u/%u  DE: %u/%u  EV: %u/%u",
		 hl, mhl, al, mal, dl, mdl, el, mel);
	m_infotext.set_text(buffer);
}

void GarrisonPanel::eject(const Soldier * soldier)
{
	uint32_t const capacity_min = m_garrison.minSoldierCapacity();
	bool can_act = m_igb.can_act(m_garrison.owner().player_number());
	bool over_min = capacity_min < m_garrison.presentSoldiers().size();

	if (can_act && over_min)
		m_igb.game().send_player_drop_soldier(m_garrison.get_building(), soldier->serial());
}

void GarrisonPanel::set_soldier_preference(int32_t changed_to) {
	upcast(Widelands::MilitarySite, ms, &m_garrison.get_building());
	assert(ms);
	m_garrison.set_soldier_preference
		(changed_to == 0 ? Widelands::Garrison::SoldierPref::Rookies
			: Widelands::Garrison::SoldierPref::Heroes);
}

}
