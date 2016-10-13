/*
 * Copyright (C) 2008-2016 by the Widelands Development Team
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

#ifndef WL_WUI_ECONOMY_OPTIONS_WINDOW_H
#define WL_WUI_ECONOMY_OPTIONS_WINDOW_H

#include "economy/economy.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/interactive_gamebase.h"
#include "wui/waresdisplay.h"

struct EconomyOptionsWindow : public UI::Window {
	EconomyOptionsWindow(InteractiveGameBase& parent, Widelands::Economy& economy);
	~EconomyOptionsWindow();

private:
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteEconomyWindow>>
	   economynotes_subscriber_;

	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay(UI::Panel* const parent,
		                   int32_t const x,
		                   int32_t const y,
		                   const Widelands::TribeDescr& tribe,
		                   Widelands::WareWorker type,
		                   bool selectable,
		                   size_t economy_number,
		                   Widelands::Player& owner);

		void set_economy_number(size_t economy_number);

	protected:
		std::string info_for_ware(Widelands::DescriptionIndex const ware) override;

	private:
		size_t economy_number_;
		Widelands::Player& owner_;
	};

	/**
	 * Wraps the wares/workers display together with some buttons
	 */
	struct EconomyOptionsPanel : UI::Box {
		EconomyOptionsPanel(UI::Panel* parent,
		                    InteractiveGameBase& igbase,
		                    Widelands::WareWorker type,
		                    size_t economy_number,
		                    Widelands::Player& owner);

		void set_economy_number(size_t economy_number);
		void change_target(int amount);
		void reset_target();

	private:
		Widelands::WareWorker type_;
		size_t economy_number_;
		Widelands::Player& owner_;
		bool can_act_;
		TargetWaresDisplay display_;
	};

	size_t economy_number_;
	Widelands::Player& owner_;
	UI::TabPanel tabpanel_;
	EconomyOptionsPanel* ware_panel_;
	EconomyOptionsPanel* worker_panel_;
};

#endif  // end of include guard: WL_WUI_ECONOMY_OPTIONS_WINDOW_H
