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

#include <memory>

#include "economy/economy.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "notifications/notifications.h"
#include "ui_basic/box.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/waresdisplay.h"

struct EconomyOptionsWindow : public UI::Window {
	EconomyOptionsWindow(UI::Panel* parent, Widelands::Economy* economy, bool can_act);
	~EconomyOptionsWindow();

private:
	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay(UI::Panel* const parent,
		                   int32_t const x,
		                   int32_t const y,
		                   Widelands::WareWorker type,
		                   bool selectable,
		                   Widelands::Economy* economy);

		void set_economy(Widelands::Economy*);

	protected:
		std::string info_for_ware(Widelands::DescriptionIndex const ware) override;

	private:
		Widelands::Economy* economy_;
	};

	/**
	 * Wraps the wares/workers display together with some buttons
	 */
	struct EconomyOptionsPanel : UI::Box {
		EconomyOptionsPanel(UI::Panel* parent,
		                    bool can_act,
		                    Widelands::WareWorker type,
		                    Widelands::Economy* economy);

		void set_economy(Widelands::Economy*);
		void change_target(int amount);
		void reset_target();

	private:
		Widelands::WareWorker type_;
		Widelands::Economy* economy_;
		bool can_act_;
		TargetWaresDisplay display_;
	};

	/// Actions performed when a NoteEconomyWindow is received.
	void on_economy_note(const Widelands::NoteEconomy& note);

	Widelands::Economy* economy_;
	UI::TabPanel tabpanel_;
	EconomyOptionsPanel* ware_panel_;
	EconomyOptionsPanel* worker_panel_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteEconomy>> economynotes_subscriber_;
};

#endif  // end of include guard: WL_WUI_ECONOMY_OPTIONS_WINDOW_H
