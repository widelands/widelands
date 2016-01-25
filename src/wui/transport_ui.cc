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

#include "economy/economy.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_gamebase.h"
#include "wui/waresdisplay.h"

#include <boost/lexical_cast.hpp>

using Widelands::Economy;
using Widelands::EditorGameBase;
using Widelands::Game;
using Widelands::WareDescr;
using Widelands::DescriptionIndex;
using Widelands::WorkerDescr;


static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";

struct EconomyOptionsWindow : public UI::UniqueWindow {
	EconomyOptionsWindow(InteractiveGameBase & parent, Economy & economy)
		:
		UI::UniqueWindow
			(&parent, "economy_options", &economy.optionswindow_registry(), 0, 0,
			 _("Economy options")),
		tabpanel_(this, 0, 0, g_gr->images().get("pics/but1.png"))
	{
		set_center_panel(&tabpanel_);

		tabpanel_.add
			("wares",
			 g_gr->images().get(pic_tab_wares),
			 new EconomyOptionsWarePanel(&tabpanel_, parent, economy),
			 _("Wares"));
		tabpanel_.add
			("workers",
			 g_gr->images().get(pic_tab_workers),
			 new EconomyOptionsWorkerPanel(&tabpanel_, parent, economy),
			 _("Workers"));
	}

private:
	UI::TabPanel tabpanel_;

	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay
			(UI::Panel * const parent,
			 int32_t const x, int32_t const y,
			 const Widelands::TribeDescr & tribe,
			 Widelands::WareWorker type,
			 bool selectable,
			 Economy & economy)
		:
			 AbstractWaresDisplay(parent, x, y, tribe, type, selectable),
			 economy_(economy)
		{
			const Widelands::TribeDescr& owner_tribe = economy_.owner().tribe();
			if (type == Widelands::wwWORKER) {
				for (const DescriptionIndex& worker_index : owner_tribe.workers()) {
					const WorkerDescr* worker_descr = owner_tribe.get_worker_descr(worker_index);
					if (!worker_descr->has_demand_check()) {
						hide_ware(worker_index);
					}
				}
			} else {
				for (const DescriptionIndex& ware_index : owner_tribe.wares()) {
					const WareDescr* ware_descr = owner_tribe.get_ware_descr(ware_index);
					if (!ware_descr->has_demand_check(owner_tribe.name())) {
						hide_ware(ware_index);
					}
				}
			}
		}
	protected:
		std::string info_for_ware(Widelands::DescriptionIndex const ware) override {
			return
				boost::lexical_cast<std::string>
				(get_type() == Widelands::wwWORKER ?
				 economy_.worker_target_quantity(ware).permanent :
				 economy_.ware_target_quantity(ware).permanent);
		}
	private:
		Economy & economy_;
	};


	/**
	 * Wraps the wares display together with some buttons
	 */
	struct EconomyOptionsWarePanel : UI::Box {
		bool can_act_;
		TargetWaresDisplay display_;
		Economy & economy_;

		EconomyOptionsWarePanel(UI::Panel * parent, InteractiveGameBase & igbase, Economy & economy) :
			UI::Box(parent, 0, 0, UI::Box::Vertical),
			can_act_(igbase.can_act(economy.owner().player_number())),
			display_(this, 0, 0, economy.owner().tribe(), Widelands::wwWARE, can_act_, economy),
			economy_(economy)
		{
			add(&display_, UI::Box::AlignLeft, true);

			UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
			add(buttons, UI::Box::AlignLeft);

			UI::Button * b = nullptr;

#define ADD_WARE_BUTTON(callback, text, tooltip)                  \
	b = new UI::Button                                    \
		 (buttons, #callback,                                       \
		  0, 0, 34, 34,                                             \
		  g_gr->images().get("pics/but4.png"),            \
		  text, tooltip, can_act_);                                \
	b->sigclicked.connect(boost::bind(&EconomyOptionsWarePanel::callback, this)); \
	buttons->add(b, UI::Box::AlignCenter);
			ADD_WARE_BUTTON(decrease_target, "-", _("Decrease target"))
			b->set_repeating(true);
			ADD_WARE_BUTTON(increase_target, "+", _("Increase target"))
			b->set_repeating(true);
			buttons->add_space(8);
			ADD_WARE_BUTTON(reset_target, "R", _("Reset to default"))
		}

		void decrease_target() {

			for (const DescriptionIndex& ware_index : economy_.owner().tribe().wares()) {
				if (display_.ware_selected(ware_index)) {
					const Economy::TargetQuantity & tq =
						economy_.ware_target_quantity(ware_index);
					if (0 < tq.permanent) {
						Widelands::Player & player = economy_.owner();
						Game & game = dynamic_cast<Game&>(player.egbase());
						game.send_player_command
							(*new Widelands::CmdSetWareTargetQuantity
								(game.get_gametime(), player.player_number(),
								 player.get_economy_number(&economy_), ware_index,
								 tq.permanent - 1));
					}
				}
			}
		}

		void increase_target() {
			for (const DescriptionIndex& ware_index : economy_.owner().tribe().wares()) {
				if (display_.ware_selected(ware_index)) {
					const Economy::TargetQuantity & tq =
						economy_.ware_target_quantity(ware_index);
					Widelands::Player & player = economy_.owner();
					Game & game = dynamic_cast<Game&>(player.egbase());
					game.send_player_command
						(*new Widelands::CmdSetWareTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&economy_), ware_index,
							 tq.permanent + 1));
				}
			}
		}

		void reset_target() {

			for (const DescriptionIndex& ware_index : economy_.owner().tribe().wares()) {
				if (display_.ware_selected(ware_index)) {
					Widelands::Player & player = economy_.owner();
					Game & game = dynamic_cast<Game&>(player.egbase());
					game.send_player_command
						(*new Widelands::CmdResetWareTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&economy_), ware_index));
				}
			}
		}
	};
	struct EconomyOptionsWorkerPanel : UI::Box {
		bool can_act_;
		TargetWaresDisplay display_;
		Economy & economy_;

		EconomyOptionsWorkerPanel(UI::Panel * parent, InteractiveGameBase & igbase, Economy & economy) :
			UI::Box(parent, 0, 0, UI::Box::Vertical),
			can_act_(igbase.can_act(economy.owner().player_number())),
			display_(this, 0, 0, economy.owner().tribe(), Widelands::wwWORKER, can_act_, economy),
			economy_(economy)
		{
			add(&display_, UI::Box::AlignLeft, true);

			UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
			add(buttons, UI::Box::AlignLeft);

			UI::Button * b = nullptr;
#define ADD_WORKER_BUTTON(callback, text, tooltip)                  \
	b = new UI::Button                                      \
		 (buttons, #callback,                                         \
		  0, 0, 34, 34,                                               \
		  g_gr->images().get("pics/but4.png"),              \
		  text, tooltip, can_act_);                                  \
	b->sigclicked.connect(boost::bind(&EconomyOptionsWorkerPanel::callback, this)); \
	buttons->add(b, UI::Box::AlignCenter);

			ADD_WORKER_BUTTON(decrease_target, "-", _("Decrease target"))
			b->set_repeating(true);
			ADD_WORKER_BUTTON(increase_target, "+", _("Increase target"))
			b->set_repeating(true);
			buttons->add_space(8);
			ADD_WORKER_BUTTON(reset_target, "R", _("Reset to default"))
		}


		void decrease_target() {
			for (const DescriptionIndex& worker_index :  economy_.owner().tribe().workers()) {
				if (display_.ware_selected(worker_index)) {
					const Economy::TargetQuantity & tq =
						economy_.worker_target_quantity(worker_index);
					if (0 < tq.permanent) {
						Widelands::Player & player = economy_.owner();
						Game & game = dynamic_cast<Game&>(player.egbase());
						game.send_player_command
							(*new Widelands::CmdSetWorkerTargetQuantity
								(game.get_gametime(), player.player_number(),
								 player.get_economy_number(&economy_), worker_index,
								 tq.permanent - 1));
					}
				}
			}
		}

		void increase_target() {
			for (const DescriptionIndex& worker_index :  economy_.owner().tribe().workers()) {
				if (display_.ware_selected(worker_index)) {
					const Economy::TargetQuantity & tq =
						economy_.worker_target_quantity(worker_index);
					Widelands::Player & player = economy_.owner();
					Game & game = dynamic_cast<Game&>(player.egbase());
					game.send_player_command
						(*new Widelands::CmdSetWorkerTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&economy_), worker_index,
							 tq.permanent + 1));
				}
			}
		}

		void reset_target() {
			for (const DescriptionIndex& worker_index :  economy_.owner().tribe().workers()) {
				if (display_.ware_selected(worker_index)) {
					Widelands::Player & player = economy_.owner();
					Game & game = dynamic_cast<Game&>(player.egbase());
					game.send_player_command
						(*new Widelands::CmdResetWorkerTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&economy_), worker_index));
				}
			}
		}
	};
};


// TODO(unknown): Neither this function nor the UI Registry should be part
// of Economy. Economy should be made an observerable class where
// users can register for change updates. The registry should be
// moved to InteractivePlayer or some other UI component.
void Economy::show_options_window() {
	if (m_optionswindow_registry.window) {
		m_optionswindow_registry.window->move_to_top();
	} else {
		new EconomyOptionsWindow(dynamic_cast<InteractiveGameBase&>
			 	(*owner().egbase().get_ibase()), *this);
	}
}
