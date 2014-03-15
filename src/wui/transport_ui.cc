/*
 * Copyright (C) 2008-2012 by the Widelands Development Team
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
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/tribe.h"
#include "logic/ware_descr.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_gamebase.h"
#include "wui/waresdisplay.h"

#include <boost/lexical_cast.hpp>

using Widelands::Economy;
using Widelands::Editor_Game_Base;
using Widelands::Game;
using Widelands::WareDescr;
using Widelands::Ware_Index;
using Widelands::Worker_Descr;


static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";

struct Economy_Options_Window : public UI::UniqueWindow {
	Economy_Options_Window(Interactive_GameBase & parent, Economy & economy)
		:
		UI::UniqueWindow
			(&parent, "economy_options", &economy.m_optionswindow_registry, 0, 0,
			 _("Economy options")),
		m_tabpanel(this, 0, 0, g_gr->images().get("pics/but1.png"))
	{
		set_center_panel(&m_tabpanel);

		m_tabpanel.add
			("wares",
			 g_gr->images().get(pic_tab_wares),
			 new Economy_Options_Ware_Panel(&m_tabpanel, parent, economy),
			 _("Wares"));
		m_tabpanel.add
			("workers",
			 g_gr->images().get(pic_tab_workers),
			 new Economy_Options_Worker_Panel(&m_tabpanel, parent, economy),
			 _("Workers"));

		// Until we can find a non-stupid way of automatically updating
		// the window when one of the target quantities changes,
		// simply disable caching.
		set_cache(false);
	}

private:
	UI::Tab_Panel m_tabpanel;

	struct TargetWaresDisplay : public AbstractWaresDisplay {
		TargetWaresDisplay
			(UI::Panel * const parent,
			 int32_t const x, int32_t const y,
			 const Widelands::Tribe_Descr & tribe,
			 Widelands::WareWorker type,
			 bool selectable,
			 Economy & economy)
		:
			 AbstractWaresDisplay(parent, x, y, tribe, type, selectable),
			 m_economy(economy)
		{
			if (type == Widelands::wwWORKER) {
				Ware_Index nr_wares = m_economy.owner().tribe().get_nrworkers();
				for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
					if (not m_economy.owner().tribe().get_worker_descr(i)->has_demand_check()) {
						hide_ware(i);
					}
				}
			} else {
				Ware_Index nr_wares = m_economy.owner().tribe().get_nrwares();
				for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
					if (not m_economy.owner().tribe().get_ware_descr(i)->has_demand_check()) {
						hide_ware(i);
					}
				}
			}
		}
	protected:
		std::string info_for_ware(Widelands::Ware_Index const ware) override {
			return
				boost::lexical_cast<std::string>
				(get_type() == Widelands::wwWORKER ?
				 m_economy.worker_target_quantity(ware).permanent :
				 m_economy.ware_target_quantity(ware).permanent);
		}
	private:
		Economy & m_economy;
	};


	/**
	 * Wraps the wares display together with some buttons
	 */
	struct Economy_Options_Ware_Panel : UI::Box {
		bool m_can_act;
		TargetWaresDisplay m_display;
		Economy & m_economy;

		Economy_Options_Ware_Panel(UI::Panel * parent, Interactive_GameBase & igbase, Economy & economy) :
			UI::Box(parent, 0, 0, UI::Box::Vertical),
			m_can_act(igbase.can_act(economy.owner().player_number())),
			m_display(this, 0, 0, economy.owner().tribe(), Widelands::wwWARE, m_can_act, economy),
			m_economy(economy)
		{
			add(&m_display, UI::Box::AlignLeft, true);

			UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
			add(buttons, UI::Box::AlignLeft);

			UI::Button * b = nullptr;

#define ADD_WARE_BUTTON(callback, text, tooltip)                  \
	b = new UI::Button                                    \
		 (buttons, #callback,                                       \
		  0, 0, 34, 34,                                             \
		  g_gr->images().get("pics/but4.png"),            \
		  text, tooltip, m_can_act);                                \
	b->sigclicked.connect(boost::bind(&Economy_Options_Ware_Panel::callback, this)); \
	buttons->add(b, UI::Box::AlignCenter);
			ADD_WARE_BUTTON(decrease_target, "-", _("Decrease target"))
			b->set_repeating(true);
			ADD_WARE_BUTTON(increase_target, "+", _("Increase target"))
			b->set_repeating(true);
			buttons->add_space(8);
			ADD_WARE_BUTTON(reset_target, "R", _("Reset to default"))
		}

		void decrease_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrwares();

			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					const Economy::Target_Quantity & tq =
						m_economy.ware_target_quantity(id);
					if (1 < tq.permanent) {
						Widelands::Player & player = m_economy.owner();
						Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
						game.send_player_command
							(*new Widelands::Cmd_SetWareTargetQuantity
								(game.get_gametime(), player.player_number(),
								 player.get_economy_number(&m_economy), id,
								 tq.permanent - 1));
					}
				}
			}
		}

		void increase_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrwares();

			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					const Economy::Target_Quantity & tq =
						m_economy.ware_target_quantity(id);
					Widelands::Player & player = m_economy.owner();
					Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_SetWareTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&m_economy), id,
							 tq.permanent + 1));
				}
			}
		}

		void reset_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrwares();

			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					Widelands::Player & player = m_economy.owner();
					Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_ResetWareTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&m_economy), id));
				}
			}
		}
	};
	struct Economy_Options_Worker_Panel : UI::Box {
		bool m_can_act;
		TargetWaresDisplay m_display;
		Economy & m_economy;

		Economy_Options_Worker_Panel(UI::Panel * parent, Interactive_GameBase & igbase, Economy & economy) :
			UI::Box(parent, 0, 0, UI::Box::Vertical),
			m_can_act(igbase.can_act(economy.owner().player_number())),
			m_display(this, 0, 0, economy.owner().tribe(), Widelands::wwWORKER, m_can_act, economy),
			m_economy(economy)
		{
			add(&m_display, UI::Box::AlignLeft, true);

			UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);
			add(buttons, UI::Box::AlignLeft);

			UI::Button * b = nullptr;
#define ADD_WORKER_BUTTON(callback, text, tooltip)                  \
	b = new UI::Button                                      \
		 (buttons, #callback,                                         \
		  0, 0, 34, 34,                                               \
		  g_gr->images().get("pics/but4.png"),              \
		  text, tooltip, m_can_act);                                  \
	b->sigclicked.connect(boost::bind(&Economy_Options_Worker_Panel::callback, this)); \
	buttons->add(b, UI::Box::AlignCenter);

			ADD_WORKER_BUTTON(decrease_target, "-", _("Decrease target"))
			b->set_repeating(true);
			ADD_WORKER_BUTTON(increase_target, "+", _("Increase target"))
			b->set_repeating(true);
			buttons->add_space(8);
			ADD_WORKER_BUTTON(reset_target, "R", _("Reset to default"))
		}


		void decrease_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrworkers();

			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					const Economy::Target_Quantity & tq =
						m_economy.worker_target_quantity(id);
					if (1 < tq.permanent) {
						Widelands::Player & player = m_economy.owner();
						Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
						game.send_player_command
							(*new Widelands::Cmd_SetWorkerTargetQuantity
								(game.get_gametime(), player.player_number(),
								 player.get_economy_number(&m_economy), id,
								 tq.permanent - 1));
					}
				}
			}
		}

		void increase_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrworkers();

			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					const Economy::Target_Quantity & tq =
						m_economy.worker_target_quantity(id);
					Widelands::Player & player = m_economy.owner();
					Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_SetWorkerTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&m_economy), id,
							 tq.permanent + 1));
				}
			}
		}

		void reset_target() {
			Widelands::Ware_Index nritems = m_economy.owner().tribe().get_nrworkers();
			for
				(Widelands::Ware_Index id = Widelands::Ware_Index::First();
				 id < nritems; ++id)
			{
				if (m_display.ware_selected(id)) {
					Widelands::Player & player = m_economy.owner();
					Game & game = ref_cast<Game, Editor_Game_Base>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_ResetWorkerTargetQuantity
							(game.get_gametime(), player.player_number(),
							 player.get_economy_number(&m_economy), id));
				}
			}
		}
	};
};


/**
 * \todo: Neither this function nor the UI Registry should be part
 * of Economy. Economy should be made an observerable class where
 * users can register for change updates. The registry should be
 * moved to InteractivePlayer or some other UI component.
 */
void Economy::show_options_window() {
	if (m_optionswindow_registry.window)
		m_optionswindow_registry.window->move_to_top();
	else
		new Economy_Options_Window
			(ref_cast<Interactive_GameBase, Interactive_Base>
			 	(*owner().egbase().get_ibase()),
			 *this);
}
