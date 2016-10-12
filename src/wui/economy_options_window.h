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
#include "ui_basic/unique_window.h"
#include "wui/interactive_gamebase.h"
#include "wui/waresdisplay.h"

struct EconomyOptionsWindow : public UI::UniqueWindow {
	EconomyOptionsWindow(InteractiveGameBase& parent, Widelands::Economy& economy);

private:
	UI::TabPanel tabpanel_;

	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay(UI::Panel* const parent,
		                   int32_t const x,
		                   int32_t const y,
		                   const Widelands::TribeDescr& tribe,
		                   Widelands::WareWorker type,
		                   bool selectable,
								 Widelands::Economy& economy);

	protected:
		std::string info_for_ware(Widelands::DescriptionIndex const ware) override;

	private:
		Widelands::Economy& economy_;
	};

	/**
	 * Wraps the wares display together with some buttons
	 */
	struct EconomyOptionsWarePanel : UI::Box {
		EconomyOptionsWarePanel(UI::Panel* parent,
		                        InteractiveGameBase& igbase,
		                        Widelands::Economy& economy);
		void decrease_target();
		void increase_target();
		void reset_target();

		bool can_act_;
		TargetWaresDisplay display_;
		Widelands::Economy& economy_;
	};
	struct EconomyOptionsWorkerPanel : UI::Box {
		EconomyOptionsWorkerPanel(UI::Panel* parent,
		                          InteractiveGameBase& igbase,
		                          Widelands::Economy& economy);
		void decrease_target();
		void increase_target();
		void reset_target();

		bool can_act_;
		TargetWaresDisplay display_;
		Widelands::Economy& economy_;
	};
};

#endif  // end of include guard: WL_WUI_ECONOMY_OPTIONS_WINDOW_H
