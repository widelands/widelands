/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "soldierlist.h"

#include <boost/bind.hpp>

#include "container_iterate.h"
#include "graphic/rendertarget.h"
#include "interactive_gamebase.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/soldiercontrol.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "wlapplication.h"

using Widelands::Soldier;
using Widelands::SoldierControl;

//static char const * pic_drop_soldier = "pics/menu_drop_soldier.png";

/**
 * Iconic representation of soldiers, including their levels and current HP.
 */
struct SoldierPanel : UI::Panel {
	typedef boost::function<void (const Soldier *)> SoldierFn;

	SoldierPanel(UI::Panel & parent, Widelands::Editor_Game_Base & egbase, Widelands::Building & building);

	Widelands::Editor_Game_Base & egbase() const {return m_egbase;}

	virtual void think();
	virtual void draw(RenderTarget &);

	void set_mouseover(const SoldierFn & fn);
	void set_click(const SoldierFn & fn);

protected:
	virtual void handle_mousein(bool inside);
	virtual bool handle_mousemove(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
	virtual bool handle_mousepress(Uint8 btn, int32_t x, int32_t y);

private:
	Point calc_pos(uint32_t row, uint32_t col) const;
	const Soldier * find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Soldier> soldier;
		uint32_t row;
		uint32_t col;
		Point pos;
	};

	Widelands::Editor_Game_Base & m_egbase;
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

SoldierPanel::SoldierPanel(UI::Panel & parent, Widelands::Editor_Game_Base & egbase, Widelands::Building & building)
:
Panel(&parent, 0, 0, 0, 0),
m_egbase(egbase),
m_soldiers(*dynamic_cast<SoldierControl *>(&building)),
m_last_animate_time(0)
{
	Soldier::calc_info_icon_size(building.tribe(), m_icon_width, m_icon_height);
	m_icon_width += 2 * IconBorder;
	m_icon_height += 2 * IconBorder;

	uint32_t maxcapacity = m_soldiers.maxSoldierCapacity();
	if (maxcapacity <= MaxColumns) {
		m_cols = maxcapacity;
		m_rows = 1;
	} else {
		m_cols = MaxColumns;
		m_rows = (maxcapacity + m_cols - 1) / m_cols;
	}

	set_size(m_cols * m_icon_width, m_rows * m_icon_height);
	set_desired_size(m_cols * m_icon_width, m_rows * m_icon_height);
	set_think(true);

	// Initialize the icons
	std::vector<Soldier *> soldierlist = m_soldiers.presentSoldiers();
	uint32_t row = 0;
	uint32_t col = 0;
	container_iterate_const(std::vector<Soldier *>, soldierlist, sit) {
		Icon icon;
		icon.soldier = *sit.current;
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
	uint32_t capacity = m_soldiers.soldierCapacity();

	// Update soldier list and target row/col:
	std::vector<Soldier *> soldierlist = m_soldiers.presentSoldiers();
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
				soldier = 0;
		}

		if (!soldier) {
			m_icons.erase(m_icons.begin() + idx);
			idx--;
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
	while (soldierlist.size()) {
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
		container_iterate(std::vector<Icon>, m_icons, icon_it) {
			if (icon_it.current->row <= icon.row)
				insertpos = icon_it.current + 1;

			icon.pos.x = std::max(icon.pos.x, static_cast<int32_t>(icon_it.current->pos.x + m_icon_width));
		}

		m_icons.insert(insertpos, icon);
	}

	// Third pass: animate icons
	int32_t curtime = WLApplication::get()->get_time();
	int32_t dt = std::min(std::max(curtime - m_last_animate_time, 0), 1000);
	int32_t maxdist = dt * AnimateSpeed / 1000;
	m_last_animate_time = curtime;

	container_iterate(std::vector<Icon>, m_icons, icon_it) {
		Icon & icon = *icon_it.current;
		Point goal = calc_pos(icon.row, icon.col);
		Point dp = goal - icon.pos;

		dp.x = std::min(std::max(dp.x, -maxdist), maxdist);
		dp.y = std::min(std::max(dp.y, -maxdist), maxdist);

		icon.pos += dp;
	}
}

void SoldierPanel::draw(RenderTarget & dst)
{
	// Fill a region matching the current site capacity with black
	uint32_t capacity = m_soldiers.soldierCapacity();
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
	container_iterate_const(std::vector<Icon>, m_icons, icon_it) {
		const Icon & icon = *icon_it.current;
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
	container_iterate_const(std::vector<Icon>, m_icons, icon_it) {
		Rect r(icon_it.current->pos, m_icon_width, m_icon_height);
		if (r.contains(Point(x, y)))
			return icon_it.current->soldier.get(egbase());
	}

	return 0;
}

void SoldierPanel::handle_mousein(bool inside)
{
	if (!inside && m_mouseover_fn)
		m_mouseover_fn(0);
}

bool SoldierPanel::handle_mousemove(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff)
{
	if (m_mouseover_fn)
		m_mouseover_fn(find_soldier(x, y));
	return true;
}

bool SoldierPanel::handle_mousepress(Uint8 btn, int32_t x, int32_t y)
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
 * List of soldiers and a "drop soldiers" button suitable for
 * \ref MilitarySiteWindow and \ref TrainingSiteWindow
 */
struct SoldierList : UI::Box {
	SoldierList
		(UI::Panel & parent,
		 Interactive_GameBase & igb,
		 Widelands::Building & building);

	SoldierControl & soldiers() const;

private:
	void mouseover(const Soldier * soldier);
	void eject(const Soldier * soldier);

	Interactive_GameBase & m_igb;
	Widelands::Building & m_building;
	SoldierPanel m_soldierpanel;
	UI::Textarea m_infotext;
};

SoldierList::SoldierList
	(UI::Panel & parent,
	 Interactive_GameBase & igb,
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
}

SoldierControl & SoldierList::soldiers() const
{
	return *dynamic_cast<SoldierControl *>(&m_building);
}

void SoldierList::mouseover(const Soldier * soldier)
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

void SoldierList::eject(const Soldier * soldier)
{
	uint32_t const capacity_min = soldiers().minSoldierCapacity();
	bool can_act = m_igb.can_act(m_building.owner().player_number());
	bool over_min = capacity_min < soldiers().presentSoldiers().size();

	if (can_act && over_min)
		m_igb.game().send_player_drop_soldier(m_building, soldier->serial());
}

UI::Panel * create_soldier_list
	(UI::Panel & parent,
	 Interactive_GameBase & igb,
	 Widelands::Building & building)
{
	return new SoldierList(parent, igb, building);
}
