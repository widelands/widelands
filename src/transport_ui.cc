/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#include "economy/economy.h"
#include "graphic/graphic.h"
#include "interactive_player.h"
#include "item_ware_descr.h"
#include "player.h"
#include "playercommand.h"
#include "graphic/rendertarget.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_unique_window.h"

using Widelands::Economy;
using Widelands::Game;
using Widelands::Item_Ware_Descr;
using Widelands::Ware_Index;

struct Economy_Options_Window : public UI::UniqueWindow {
	Economy_Options_Window(Interactive_Player & parent, Economy & _economy)

		:
		UI::UniqueWindow
			(&parent, &_economy.m_optionswindow_registry, 0, 0,
			 _("Target quantities")),
		m_box
			(this, 0, 0, UI::Box::Vertical,
			 g_gr->get_xres() - 80, g_gr->get_yres() - 80),
		m_economy (_economy)
	{
		Widelands::Tribe_Descr const & tribe = parent.player().tribe();
		Ware_Index const nr_wares = tribe.get_nrwares();
		for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
			Item_Ware_Descr const & descr = *tribe.get_ware_descr(i);
			if (descr.has_demand_check())
				m_box.add(new Ware_Type_Box(m_box, i, descr), UI::Box::AlignTop);
		}
		m_box.set_scrolling(true);
		fit_inner(m_box);
	}

	virtual void think() {
		for
			(Ware_Type_Box * b =
			 	dynamic_cast<Ware_Type_Box *>(m_box.get_first_child());
			 b;
			 b = dynamic_cast<Ware_Type_Box *>(b->get_next_sibling()))
		{
			Ware_Index const i = b->ware_type;
			Economy::Target_Quantity const & tq = economy().target_quantity(i);
			b->decrease_permanent.set_enabled(1 < tq.permanent);
			b->decrease_temporary.set_enabled(1 < tq.temporary);
			b->reset             .set_enabled(tq.last_modified);
		}
	}

	Economy & economy() {return m_economy;}

private:
	struct Ware_Type_Box : public UI::Panel {
		Ware_Type_Box
			(UI::Box               &       parent,
			 Ware_Index              const _ware_type,
			 Item_Ware_Descr const &       _descr)
			:
			UI::Panel  (&parent, 0, 0, 420, 24),
			decrease_permanent(*this),
			increase_permanent(*this),
			decrease_temporary(*this),
			increase_temporary(*this),
			reset             (*this),
			ware_type         (_ware_type),
			descr             (_descr)
		{}

		virtual void draw(RenderTarget & dst) {
			dst.blit(Point(0, 0), descr.icon());
			g_fh->draw_string
				(dst,
				 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
				 Point(26, 12),
				 descr.descname().c_str(),
				 Align_CenterLeft,
				 -1);
			Economy::Target_Quantity const & tq =
				economy().target_quantity(ware_type);
			char buffer[32];
			sprintf(buffer, "%u", tq.permanent);
			g_fh->draw_string
				(dst,
				 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
				 Point(188, 12),
				 buffer,
				 Align_CenterRight,
				 -1);
			sprintf(buffer, "%u", tq.temporary);
			g_fh->draw_string
				(dst,
				 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
				 Point(278, 12),
				 buffer,
				 Align_CenterRight,
				 -1);
			UI::Panel::draw(dst);
		}

		Economy & economy() const {
			return
				dynamic_cast<Economy_Options_Window &>
					(*dynamic_cast<UI::Box &>(*get_parent()).get_parent())
				.economy();
		}

		struct Decrease_Permanent : public UI::Button {
			Decrease_Permanent(Ware_Type_Box & parent) :
				UI::Button
					(&parent, 190, 0, 24, 24, 0,
					 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
					 _("Decrease permanent target quantity"),
					 true)
			{
				set_repeating(true);
			}
			virtual void clicked() const {
				Ware_Type_Box const & parent =
					dynamic_cast<Ware_Type_Box &>(*get_parent());
				Economy & e = parent.economy();
				Ware_Index const ware_type = parent.ware_type;
				Economy::Target_Quantity const & tq = e.target_quantity(ware_type);
				assert(tq.permanent <= tq.temporary);
				if (1 < tq.permanent) {
					Widelands::Player & player = e.owner();
					Game & game = dynamic_cast<Game &>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_SetTargetQuantity
						 	(game.get_gametime(), player.get_player_number(),
						 	 player.get_economy_number(&e), ware_type,
						 	 tq.permanent - 1, tq.temporary));
				}
			}
		} decrease_permanent;

		struct Increase_Permanent : public UI::Button {
			Increase_Permanent(Ware_Type_Box & parent) :
				UI::Button
					(&parent, 214, 0, 24, 24, 0,
					 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
					 _("Increase permanent target quantity"))
			{
				set_repeating(true);
			}
			virtual void clicked() const {
				Ware_Type_Box const & parent =
					dynamic_cast<Ware_Type_Box &>(*get_parent());
				Economy & e = parent.economy();
				Ware_Index const ware_type = parent.ware_type;
				Economy::Target_Quantity const & tq = e.target_quantity(ware_type);
				assert(tq.permanent <= tq.temporary);
				uint32_t const new_permanent = tq.permanent + 1;
				Widelands::Player & player = e.owner();
				Game & game = dynamic_cast<Game &>(player.egbase());
				game.send_player_command
					(*new Widelands::Cmd_SetTargetQuantity
					 	(game.get_gametime(), player.get_player_number(),
					 	 player.get_economy_number(&e), ware_type,
					 	 new_permanent, std::max(new_permanent, tq.temporary)));
			}
		} increase_permanent;

		struct Decrease_Temporary : public UI::Button {
			Decrease_Temporary(Ware_Type_Box & parent) :
				UI::Button
					(&parent, 280, 0, 24, 24, 0,
					 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
					 _("Decrease temporary target quantity"))
			{
				set_repeating(true);
			}
			virtual void clicked() const {
				Ware_Type_Box const & parent =
					dynamic_cast<Ware_Type_Box &>(*get_parent());
				Economy & e = parent.economy();
				Ware_Index const ware_type = parent.ware_type;
				Economy::Target_Quantity const & tq = e.target_quantity(ware_type);
				assert(tq.permanent <= tq.temporary);
				if (1 < tq.temporary) {
					uint32_t const new_temporary = tq.temporary - 1;
					Widelands::Player & player = e.owner();
					Game   & game   = dynamic_cast<Game &>(player.egbase());
					game.send_player_command
						(*new Widelands::Cmd_SetTargetQuantity
						 	(game.get_gametime(), player.get_player_number(),
						 	 player.get_economy_number(&e), ware_type,
						 	 std::min(tq.permanent, new_temporary), new_temporary));
				}
			}
		} decrease_temporary;

		struct Increase_Temporary : public UI::Button {
			Increase_Temporary(Ware_Type_Box & parent) :
				UI::Button
					(&parent, 304, 0, 24, 24, 0,
					 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
					 _("Increase temporary target quantity"))
			{
				set_repeating(true);
			}
			virtual void clicked() const {
				Ware_Type_Box const & parent =
					dynamic_cast<Ware_Type_Box &>(*get_parent());
				Economy & e = parent.economy();
				Ware_Index const ware_type = parent.ware_type;
				Economy::Target_Quantity const & tq = e.target_quantity(ware_type);
				assert(tq.permanent <= tq.temporary);
				Widelands::Player & player = e.owner();
				Game & game = dynamic_cast<Game &>(player.egbase());
				game.send_player_command
					(*new Widelands::Cmd_SetTargetQuantity
					 	(game.get_gametime(), player.get_player_number(),
					 	 player.get_economy_number(&e), ware_type,
					 	 tq.permanent, tq.temporary + 1));
			}
		} increase_temporary;

		struct Reset : UI::Button {
			Reset(Ware_Type_Box & parent) :
				UI::Button
					(&parent, 330, 0, 90, 24, 0,
					 _("Reset"), _("Reset target quantity to default value"))
			{}
			virtual void clicked() const {
				Ware_Type_Box const & parent =
					dynamic_cast<Ware_Type_Box &>(*get_parent());
				Economy & e = parent.economy();
				Widelands::Player & player = e.owner();
				Game & game = dynamic_cast<Game &>(player.egbase());
				game.send_player_command
					(*new Widelands::Cmd_ResetTargetQuantity
					 	(game.get_gametime(), player.get_player_number(),
					 	 player.get_economy_number(&e), parent.ware_type));
			}
		} reset;

		Ware_Index              const ware_type;
		Item_Ware_Descr const &       descr;
	};

	UI::Box           m_box;
	Economy         & m_economy;
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
			(dynamic_cast<Interactive_Player &>(*owner().egbase().get_ibase()),
			 *this);
}
