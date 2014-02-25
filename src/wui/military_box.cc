/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "wui/military_box.h"

#include "graphic/graphic.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/playercommand.h"
#include "upcast.h"

using Widelands::Editor_Game_Base;
using Widelands::Game;

MilitaryBox::MilitaryBox
	(UI::Panel              * parent,
	 Widelands::Player      * player,
	 uint32_t  const              x,
	 uint32_t  const              y)
:
	UI::Box
		(parent, x, y, UI::Box::Vertical),
	m_pl(player),
	m_map(&m_pl->egbase().map()),
	m_allowed_change(false),
	m_slider_retreat(nullptr),
	m_text_retreat(nullptr)
{
	init();
}

MilitaryBox::~MilitaryBox() {
	delete m_slider_retreat;
	delete m_text_retreat;
}

UI::Slider & MilitaryBox::add_slider
	(UI::Box         & parent,
	 uint32_t          width,
	 uint32_t          height,
	 uint32_t          min, uint32_t max, uint32_t initial,
	 char      const * picname,
	 char      const * hint)
{
	UI::HorizontalSlider & result =
		*new UI::HorizontalSlider
			(&parent,
			 0, 0,
			 width, height,
			 min, max, initial,
			 g_gr->images().get(picname),
			 hint);
	parent.add(&result, UI::Box::AlignTop);
	return result;
}

UI::Textarea & MilitaryBox::add_text
	(UI::Box           & parent,
	 std::string   str,
	 uint32_t      alignment,
	 const std::string & fontname,
	 uint32_t      fontsize)
{
	UI::Textarea & result = *new UI::Textarea(&parent, 0, 0, str.c_str());
	result.set_textstyle(UI::TextStyle::makebold(UI::Font::get(fontname, fontsize), UI_FONT_CLR_FG));
	parent.add(&result, alignment);
	return result;
}

UI::Button & MilitaryBox::add_button
	(UI::Box           & parent,
	 char        const * const name,
	 char        const * const text,
	 void         (MilitaryBox::*fn)(),
	 const std::string & tooltip_text)
{
	UI::Button * button =
		new UI::Button
			(&parent, name,
			 8, 8, 26, 26,
			 g_gr->images().get("pics/but2.png"),
			 text,
			 tooltip_text);
	button->sigclicked.connect(boost::bind(fn, boost::ref(*this)));
	parent.add(button, Box::AlignTop);
	return *button;
}

void MilitaryBox::update() {
	Game & game = ref_cast<Game, Editor_Game_Base>(m_pl->egbase());

	if (m_pl->is_retreat_change_allowed()) {
		char buf[20];
		assert(m_slider_retreat);
		assert(m_text_retreat);
		/// Send change to player
		game.send_player_changemilitaryconfig
			(m_pl->player_number(), m_slider_retreat->get_value());
		/// Update UI
		sprintf(buf, "%u %%", m_slider_retreat->get_value());
		m_text_retreat->set_text(buf);
	}
}

void MilitaryBox::init() {
	char buf[10];

	{ //  Retreat line
		UI::Box & linebox = *new UI::Box(this, 0, 0, UI::Box::Horizontal);
		add(&linebox, UI::Box::AlignTop);

		add_text(linebox, _("Retreat: Never!"));

		//  Spliter of retreat
		UI::Box & columnbox = *new UI::Box(&linebox, 0, 0, UI::Box::Vertical);
		linebox.add(&columnbox, UI::Box::AlignTop);

		sprintf(buf, "%u %%", m_pl->get_retreat_percentage());

		m_text_retreat =
			&add_text(columnbox, buf, UI::Box::AlignCenter, UI_FONT_ULTRASMALL);

		m_slider_retreat =
			&add_slider
				(columnbox,
				 100, 10,
				 m_pl->tribe().get_military_data().get_min_retreat(),
				 m_pl->tribe().get_military_data().get_max_retreat(),
				 m_pl->get_retreat_percentage(),
				 "pics/caret.png",
				 _("Supported damage before retreat"));
		m_slider_retreat->changed.connect(boost::bind(&MilitaryBox::update, this));
		add_text(linebox, _("Once injured"));
		m_slider_retreat->set_enabled(m_pl->is_retreat_change_allowed());
		linebox.set_visible(m_pl->is_retreat_change_allowed());
	}

	//  Add here another attack configuration parameters.

	// Specify allow to change
	m_allowed_change = m_pl->is_retreat_change_allowed();
}

