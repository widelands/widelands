/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_event_menu_edit_trigger_conditional.h"

#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "trigger/trigger.h"
#include "trigger/trigger_conditional.h"

Editor_Event_Menu_Edit_TriggerConditional::Editor_Event_Menu_Edit_TriggerConditional
(Editor_Interactive* parent, TriggerConditional* cond, EventChain* chain)
:
UI::Window(parent, 0, 0, 465, 340, _("Edit Trigger Conditional").c_str()),
m_parent(parent),
m_given_cond(cond),
m_event_chain(chain)
{

   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;
   const int32_t ls_width = 180;

   // Trigger List
   new UI::Textarea(this, posx, offsy, _("Trigger Conditional: "), Align_Left);
	m_construction= new UI::Listselect<TriggerConditional_Factory::Token &>(this, spacing, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_construction->selected.set(this, &Editor_Event_Menu_Edit_TriggerConditional::cs_selected);
   m_construction->double_clicked.set(this, &Editor_Event_Menu_Edit_TriggerConditional::cs_double_clicked);
   posx += ls_width + spacing;

   posy = 35;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::LPAREN,
		 _("("));

   posy += 20 + spacing;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::RPAREN,
		 _(")"));

   posy += 20 + spacing;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::AND,
		 _("AND"));

	posy += 20 + spacing;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::OR,
		 _("OR"));

   posy += 20 + spacing;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::XOR,
		 _("XOR"));

   posy += 20 + spacing;

	new UI::IDButton
		<Editor_Event_Menu_Edit_TriggerConditional,
		TriggerConditional_Factory::TokenNames>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_operator,
		 this, TriggerConditional_Factory::NOT,
		 _("NOT"));

   posy += 20 + spacing + spacing;

	m_insert_btn = new UI::Button<Editor_Event_Menu_Edit_TriggerConditional>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_ins_trigger, this,
		 _("<-"),
		 _("Insert"),
		 false);

   posy += 20 + spacing + spacing;

	m_delete_btn = new UI::Button<Editor_Event_Menu_Edit_TriggerConditional>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_del_trigger, this,
		 _("Delete"),
		 std::string(),
		 false);

   posy += 20 + spacing + spacing + spacing;

	m_mvup_btn = new UI::Button<Editor_Event_Menu_Edit_TriggerConditional>
		(this,
		 posx + 5, posy, 24, 24,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_move_up, this,
		 std::string(),
		 false);

	m_mvdown_btn = new UI::Button<Editor_Event_Menu_Edit_TriggerConditional>
		(this,
		 posx + 51, posy, 24, 24,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_move_down, this,
		 std::string(),
		 false);

   posy += 24 + spacing + spacing;

   posx += 80 + spacing;
   new UI::Textarea(this, posx, offsy, _("Available Triggers: "), Align_Left);
   m_trigger_list=new UI::Listselect<Trigger &>(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu_Edit_TriggerConditional::tl_selected);
   m_trigger_list->double_clicked.set(this, &Editor_Event_Menu_Edit_TriggerConditional::tl_double_clicked);
	const MapTriggerManager & mtm = parent->egbase().map().get_mtm();
	const MapTriggerManager::Index nr_triggers = mtm.get_nr_triggers();
	for (MapTriggerManager::Index i = 0; i < nr_triggers; ++i) {
		Trigger & tr = mtm.get_trigger_by_nr(i);
		m_trigger_list->add(tr.get_name(), tr);
	}
   m_trigger_list->sort();

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;

	new UI::Button<Editor_Event_Menu_Edit_TriggerConditional>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_Edit_TriggerConditional::clicked_ok, this,
		 _("Ok"));

   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Editor_Event_Menu_Edit_TriggerConditional, int32_t>
		(this,
		 posx, posy, 80, 20,
		 1,
		 &Editor_Event_Menu_Edit_TriggerConditional::end_modal, this, 0,
		 _("Cancel"));

   // Add conditional
   if (cond) {
      std::vector< TriggerConditional_Factory::Token >* tokens = cond->get_infix_tokenlist();
      for (uint32_t i = 0; i < tokens->size(); i++) {
         TriggerConditional_Factory::Token & t = *new TriggerConditional_Factory::Token((*tokens)[i]);
	      assert(t.token <= TriggerConditional_Factory::TRIGGER);
			m_construction->add
		      (t.token == TriggerConditional_Factory::TRIGGER ?
		       static_cast<Trigger *>(t.data)->get_name()
		       :
		       TriggerConditional_Factory::operators[t.token],
		       t,
				 -1,
		       true);
		}
	}

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_Edit_TriggerConditional::~Editor_Event_Menu_Edit_TriggerConditional() {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Editor_Event_Menu_Edit_TriggerConditional::handle_mousepress
(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Editor_Event_Menu_Edit_TriggerConditional::handle_mouserelease
(const Uint8, int32_t, int32_t)
{return false;}


void Editor_Event_Menu_Edit_TriggerConditional::clicked_ok() {
      // construct token list
      std::vector<TriggerConditional_Factory::Token> tok;

	const uint32_t construction_size = m_construction->size();
	for (uint32_t i = 0; i < construction_size; ++i)
		tok.push_back((*m_construction)[i]);

      try {
         if (!tok.size()) throw TriggerConditional_Factory::SyntaxError();
         TriggerConditional* cond = TriggerConditional_Factory::create_from_infix(m_event_chain, tok);
         assert(cond);
         m_given_cond = cond;
         end_modal(1);
		} catch (TriggerConditional_Factory::SyntaxError err) {
			UI::Modal_Message_Box mb
				(m_parent,
				 _("Syntax Error"),
				 _("Your conditional contains at least one syntax error. Please "
				   "correct!\n"),
				 UI::Modal_Message_Box::OK);
			mb.run();
         return;
		}
}


void Editor_Event_Menu_Edit_TriggerConditional::clicked_operator
(const TriggerConditional_Factory::TokenNames i)
{
	assert(i <= TriggerConditional_Factory::TRIGGER);
      TriggerConditional_Factory::Token & t = *new TriggerConditional_Factory::Token();
      t.data = 0;
	m_construction->add(TriggerConditional_Factory::operators[i], t, -1, true);
}


void Editor_Event_Menu_Edit_TriggerConditional::clicked_ins_trigger() {
	Trigger & trigger = m_trigger_list->get_selected();
      TriggerConditional_Factory::Token & t = *new TriggerConditional_Factory::Token();
      t.data = &trigger;
      t.token = TriggerConditional_Factory::TRIGGER;
	m_construction->add(trigger.get_name(), t, -1, true);
	}


void Editor_Event_Menu_Edit_TriggerConditional::clicked_del_trigger() {
	delete &m_construction->get_selected();

	m_construction->remove_selected();
      m_mvup_btn->set_enabled(false);
      m_mvdown_btn->set_enabled(false);
      m_delete_btn->set_enabled(false);
}


void Editor_Event_Menu_Edit_TriggerConditional::clicked_move_up() {
	assert(m_construction->has_selection()); //  Button should be disabled.
	const int32_t n = m_construction->selection_index();
	assert(n > 0); //  Button should be disabled.
	m_construction->switch_entries(n, n - 1);
}
void Editor_Event_Menu_Edit_TriggerConditional::clicked_move_down() {
	assert(m_construction->has_selection()); //  Button should be disabled.
	const uint32_t n = m_construction->selection_index();
	assert(n + 1 < m_construction->size()); //  Button should be disabled.
	m_construction->switch_entries(n, n + 1);
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_Edit_TriggerConditional::tl_selected(uint32_t) {
   m_insert_btn->set_enabled(true);
}
void Editor_Event_Menu_Edit_TriggerConditional::cs_selected(uint32_t i) {
	m_mvdown_btn->set_enabled(i + 1 < m_construction->size());
	m_mvup_btn->set_enabled(i > 0);
   m_delete_btn->set_enabled(true);
}

/*
 * listbox got double clicked
 */
void Editor_Event_Menu_Edit_TriggerConditional::tl_double_clicked(uint32_t)
{clicked_ins_trigger();}
void Editor_Event_Menu_Edit_TriggerConditional::cs_double_clicked(uint32_t)
{clicked_del_trigger();}
