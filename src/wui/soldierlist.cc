/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/soldierlist.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "wlapplication.h"
#include "wui/interactive_gamebase.h"
#include "wui/soldiercapacitycontrol.h"

using Widelands::Soldier;
using Widelands::SoldierControl;

/**
 * Iconic representation of soldiers, including their levels and current HP.
 */
struct SoldierPanel : UI::Panel {
	using SoldierFn = boost::function<void (const Soldier *)>;

	SoldierPanel(UI::Panel & parent, Widelands::EditorGameBase & egbase, Widelands::Building & building);

	Widelands::EditorGameBase & egbase() const {return m_egbase;}

	void think() override;
	void draw(RenderTarget &) override;

	void set_mouseover(const SoldierFn & fn);
	void set_click(const SoldierFn & fn);

protected:
	void handle_mousein(bool inside) override;
	bool handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

private:
	Point calc_pos(uint32_t row, uint32_t col) const;
	const Soldier * find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Soldier> soldier;
		uint32_t row;
		uint32_t col;
		Point pos;

		/**
		 * Keep track of how we last rendered this soldier,
		 * so that we can update when its status changes.
		 */
		/*@{*/
		uint32_t cache_level;
		uint32_t cache_health;
		/*@}*/
	};

	Widelands::EditorGameBase & m_egbase;
	SoldierControl & m_soldiers;

	SoldierFn m_mouseover_fn;
	SoldierFn m_click_fn;

	std::vector<Icon> m_icons;

	uint32_t m_rows;
	uint32_t m_cols;

	uint32_t m_icon_width;
	uint32_t m_icon_height;

	int32_t m_last_animate_time;

	static const uint32_t MaxColumns = 6;
	static const uint32_t AnimateSpeed = 300; ///< in pixels per second
	static const uint32_t IconBorder = 2;
};

SoldierPanel::SoldierPanel
	(UI::Panel & parent,
	 Widelands::EditorGameBase & gegbase,
	 Widelands::Building & building)
:
Panel(&parent, 0, 0, 0, 0),
m_egbase(gegbase),
m_soldiers(*dynamic_cast<SoldierControl *>(&building)),
m_last_animate_time(0)
{
	Soldier::calc_info_icon_size(building.owner().tribe(), m_icon_width, m_icon_height);
	m_icon_width += 2 * IconBorder;
	m_icon_height += 2 * IconBorder;

	uint32_t maxcapacity = m_soldiers.max_soldier_capacity();
	if (maxcapacity <= MaxColumns) {
		m_cols = maxcapacity;
		m_rows = 1;
	} else {
		m_cols = MaxColumns;
		m_rows = (maxcapacity + m_cols - 1) / m_cols;
	}

	set_size(m_cols * m_icon_width, m_rows * m_icon_height);
	set_desired_size(m_cols * m_icon_width, m_rows * m_icon_height);
	set_thinks(true);

	// Initialize the icons
	uint32_t row = 0;
	uint32_t col = 0;
	for (Soldier * soldier : m_soldiers.present_soldiers()) {
		Icon icon;
		icon.soldier = soldier;
		icon.row = row;
		icon.col = col;
		icon.pos = calc_pos(row, col);
		m_icons.push_back(icon);

		if (++col >= m_cols) {
			col = 0;
			row++;
		}
	}
}

/**
 * Set the callback function that indicates which soldier the mouse is over.
 */
void SoldierPanel::set_mouseover(const SoldierPanel::SoldierFn & fn)
{
	m_mouseover_fn = fn;
}

/**
 * Set the callback function that is called when a soldier is clicked.
 */
void SoldierPanel::set_click(const SoldierPanel::SoldierFn & fn)
{
	m_click_fn = fn;
}

void SoldierPanel::think()
{
	bool changes = false;
	uint32_t capacity = m_soldiers.soldier_capacity();

	// Update soldier list and target row/col:
	std::vector<Soldier *> soldierlist = m_soldiers.present_soldiers();
	std::vector<uint32_t> row_occupancy;
	row_occupancy.resize(m_rows);

	// First pass: check whether existing icons are still valid, and compact them
	for (uint32_t idx = 0; idx < m_icons.size(); ++idx) {
		Icon & icon = m_icons[idx];
		Soldier * soldier = icon.soldier.get(egbase());
		if (soldier) {
			std::vector<Soldier *>::iterator it = std::find(soldierlist.begin(), soldierlist.end(), soldier);
			if (it != soldierlist.end())
				soldierlist.erase(it);
			else
				soldier = nullptr;
		}

		if (!soldier) {
			m_icons.erase(m_icons.begin() + idx);
			idx--;
			changes = true;
			continue;
		}

		while
			(icon.row &&
			 (row_occupancy[icon.row] >= MaxColumns ||
			  icon.row * MaxColumns + row_occupancy[icon.row] >= capacity))
			icon.row--;

		icon.col = row_occupancy[icon.row]++;
	}

	// Second pass: add new soldiers
	while (!soldierlist.empty()) {
		Icon icon;
		icon.soldier = soldierlist.back();
		soldierlist.pop_back();
		icon.row = 0;
		while (row_occupancy[icon.row] >= MaxColumns)
			icon.row++;
		icon.col = row_occupancy[icon.row]++;
		icon.pos = calc_pos(icon.row, icon.col);

		// Let soldiers slide in from the right border
		icon.pos.x = get_w();

		std::vector<Icon>::iterator insertpos = m_icons.begin();

		for (std::vector<Icon>::iterator icon_iter = m_icons.begin();
			  icon_iter != m_icons.end();
			  ++icon_iter) {

			if (icon_iter->row <= icon.row)
				insertpos = icon_iter + 1;

			icon.pos.x = std::max<int32_t>(icon.pos.x, icon_iter->pos.x + m_icon_width);
		}

		icon.cache_health = 0;
		icon.cache_level = 0;

		m_icons.insert(insertpos, icon);
		changes = true;
	}

	// Third pass: animate icons
	int32_t curtime = SDL_GetTicks();
	int32_t dt = std::min(std::max(curtime - m_last_animate_time, 0), 1000);
	int32_t maxdist = dt * AnimateSpeed / 1000;
	m_last_animate_time = curtime;

	for (Icon& icon : m_icons) {
		Point goal = calc_pos(icon.row, icon.col);
		Point dp = goal - icon.pos;

		dp.x = std::min(std::max(dp.x, -maxdist), maxdist);
		dp.y = std::min(std::max(dp.y, -maxdist), maxdist);

		if (dp.x != 0 || dp.y != 0)
			changes = true;

		icon.pos += dp;

		// Check whether health and/or level of the soldier has changed
		Soldier * soldier = icon.soldier.get(egbase());
		uint32_t level = soldier->get_attack_level();
		level = level * (soldier->descr().get_max_defense_level() + 1) + soldier->get_defense_level();
		level = level * (soldier->descr().get_max_evade_level() + 1) + soldier->get_evade_level();
		level = level * (soldier->descr().get_max_hp_level() + 1) + soldier->get_hp_level();

		uint32_t health = soldier->get_current_hitpoints();

		if (health != icon.cache_health || level != icon.cache_level) {
			icon.cache_level = level;
			icon.cache_health = health;
			changes = true;
		}
	}

	if (changes) {
		Point mousepos = get_mouse_position();
		m_mouseover_fn(find_soldier(mousepos.x, mousepos.y));
		update();
	}
}

void SoldierPanel::draw(RenderTarget & dst)
{
	// Fill a region matching the current site capacity with black
	uint32_t capacity = m_soldiers.soldier_capacity();
	uint32_t fullrows = capacity / MaxColumns;

	if (fullrows)
		dst.fill_rect
			(Rect(Point(0, 0), get_w(), m_icon_height * fullrows),
			 RGBAColor(0, 0, 0, 0));
	if (capacity % MaxColumns)
		dst.fill_rect
			(Rect
				(Point(0, m_icon_height * fullrows),
				 m_icon_width * (capacity % MaxColumns),
				 m_icon_height),
			 RGBAColor(0, 0, 0, 0));

	// Draw icons
	for (const Icon& icon : m_icons) {
		const Soldier * soldier = icon.soldier.get(egbase());
		if (!soldier)
			continue;

		soldier->draw_info_icon(dst, icon.pos + Point(IconBorder, IconBorder), false);
	}
}

Point SoldierPanel::calc_pos(uint32_t row, uint32_t col) const
{
	return Point(col * m_icon_width, row * m_icon_height);
}

/**
 * Return the soldier (if any) at the given coordinates.
 */
const Soldier * SoldierPanel::find_soldier(int32_t x, int32_t y) const
{
	for (const Icon& icon : m_icons) {
		Rect r(icon.pos, m_icon_width, m_icon_height);
		if (r.contains(Point(x, y))) {
			return icon.soldier.get(egbase());
		}
	}

	return nullptr;
}

void SoldierPanel::handle_mousein(bool inside)
{
	if (!inside && m_mouseover_fn)
		m_mouseover_fn(nullptr);
}

bool SoldierPanel::handle_mousemove
	(uint8_t /* state */,
	 int32_t x,
	 int32_t y,
	 int32_t /* xdiff */,
	 int32_t /* ydiff */)
{
	if (m_mouseover_fn)
		m_mouseover_fn(find_soldier(x, y));
	return true;
}

bool SoldierPanel::handle_mousepress(uint8_t btn, int32_t x, int32_t y)
{
	if (btn == SDL_BUTTON_LEFT) {
		if (m_click_fn) {
			if (const Soldier * soldier = find_soldier(x, y))
				m_click_fn(soldier);
		}
		return true;
	}

	return false;
}

/**
 * List of soldiers \ref MilitarySiteWindow and \ref TrainingSiteWindow
 */
struct SoldierList : UI::Box {
	SoldierList
		(UI::Panel & parent,
		 InteractiveGameBase & igb,
		 Widelands::Building & building);

	SoldierControl & soldiers() const;

private:
	void mouseover(const Soldier * soldier);
	void eject(const Soldier * soldier);
	void set_soldier_preference(int32_t changed_to);
	void think() override;

	InteractiveGameBase & m_igb;
	Widelands::Building & m_building;
	SoldierPanel m_soldierpanel;
	UI::Radiogroup m_soldier_preference;
	UI::Textarea m_infotext;
};

SoldierList::SoldierList
	(UI::Panel & parent,
	 InteractiveGameBase & igb,
	 Widelands::Building & building)
:
UI::Box(&parent, 0, 0, UI::Box::Vertical),

m_igb(igb),
m_building(building),
m_soldierpanel(*this, igb.egbase(), building),
m_infotext(this, _("Click soldier to send away"))
{
	add(&m_soldierpanel, UI::Box::AlignCenter);

	add_space(2);

	add(&m_infotext, UI::Box::AlignCenter);

	m_soldierpanel.set_mouseover(boost::bind(&SoldierList::mouseover, this, _1));
	m_soldierpanel.set_click(boost::bind(&SoldierList::eject, this, _1));

	// We don't want translators to translate this twice, so it's a bit involved.
	int w = UI::g_fh1->render(
				  as_uifont((boost::format("%s ") // We need some extra space to fix bug 724169
								 /** TRANSLATORS: Health, Attack, Defense, Evade */
								 % (boost::format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"))
									 % 8 % 8
									 % 8 % 8
									 % 8 % 8
									 % 8 % 8)).str()))->width();
	uint32_t maxtextwidth = std::max(w,
												UI::g_fh1->render(as_uifont(_("Click soldier to send away")))->width());
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	bool can_act = m_igb.can_act(m_building.owner().player_number());
	if (upcast(Widelands::MilitarySite, ms, &building)) {
		m_soldier_preference.add_button
			(buttons, Point(0, 0),
			 g_gr->images().get("images/wui/buildings/prefer_rookies.png"),
			 _("Prefer Rookies"));
		m_soldier_preference.add_button
			(buttons, Point(32, 0),
			 g_gr->images().get("images/wui/buildings/prefer_heroes.png"),
			 _("Prefer Heroes"));
		UI::Radiobutton* button = m_soldier_preference.get_first_button();
		while (button) {
			buttons->add(button, AlignLeft);
			button = button->next_button();
		}

		m_soldier_preference.set_state(0);
		if (ms->get_soldier_preference() == Widelands::MilitarySite::kPrefersHeroes) {
			m_soldier_preference.set_state(1);
		}
		if (can_act) {
			m_soldier_preference.changedto.connect
				(boost::bind(&SoldierList::set_soldier_preference, this, _1));
		} else {
			m_soldier_preference.set_enabled(false);
		}
	}
	buttons->add_inf_space();
	buttons->add
		(create_soldier_capacity_control(*buttons, igb, building),
		 UI::Box::AlignRight);

	add(buttons, UI::Box::AlignCenter, true);
}

SoldierControl & SoldierList::soldiers() const
{
	return *dynamic_cast<SoldierControl *>(&m_building);
}

void SoldierList::think()
{
	// Only update the soldiers pref radio if player is spectator
	if (m_igb.can_act(m_building.owner().player_number())) {
		return;
	}
	if (upcast(Widelands::MilitarySite, ms, &m_building)) {
		switch (ms->get_soldier_preference()) {
			case Widelands::MilitarySite::kPrefersRookies:
				m_soldier_preference.set_state(0);
				break;
			case Widelands::MilitarySite::kPrefersHeroes:
				m_soldier_preference.set_state(1);
				break;
			case Widelands::MilitarySite::kNoPreference:
				m_soldier_preference.set_state(-1);
				break;
		}
	}
}


void SoldierList::mouseover(const Soldier * soldier)
{
	if (!soldier) {
		m_infotext.set_text(_("Click soldier to send away"));
		return;
	}

	m_infotext.set_text(
		(boost::format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"))
			% soldier->get_hp_level() % soldier->descr().get_max_hp_level()
			% soldier->get_attack_level() % soldier->descr().get_max_attack_level()
			% soldier->get_defense_level() % soldier->descr().get_max_defense_level()
			% soldier->get_evade_level() % soldier->descr().get_max_evade_level()
		).str()
	);
}

void SoldierList::eject(const Soldier * soldier)
{
	uint32_t const capacity_min = soldiers().min_soldier_capacity();
	bool can_act = m_igb.can_act(m_building.owner().player_number());
	bool over_min = capacity_min < soldiers().present_soldiers().size();

	if (can_act && over_min)
		m_igb.game().send_player_drop_soldier(m_building, soldier->serial());
}

void SoldierList::set_soldier_preference(int32_t changed_to) {
#ifndef NDEBUG
	upcast(Widelands::MilitarySite, ms, &m_building);
	assert(ms);
#endif
	m_igb.game().send_player_militarysite_set_soldier_preference
		(m_building, changed_to == 0 ?
			Widelands::MilitarySite::kPrefersRookies:
			Widelands::MilitarySite::kPrefersHeroes);
}

UI::Panel * create_soldier_list
	(UI::Panel & parent,
	 InteractiveGameBase & igb,
	 Widelands::Building & building)
{
	return new SoldierList(parent, igb, building);
}
