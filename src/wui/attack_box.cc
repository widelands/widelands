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

#include "wui/attack_box.h"

#include <string>

#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "logic/soldier.h"

AttackBox::AttackBox
	(UI::Panel              * parent,
	 Widelands::Player      * player,
	 Widelands::FCoords     * target,
	 uint32_t  const              x,
	 uint32_t  const              y)
:
	UI::Box
		(parent, x, y, UI::Box::Vertical),
	m_pl(player),
	m_map(&m_pl->egbase().map()),
	m_node(target),
	m_slider_soldiers(nullptr),
	m_text_soldiers(nullptr),
	m_add_soldiers(nullptr)
{
	init();
}

AttackBox::~AttackBox() {
	delete m_slider_soldiers;
	delete m_text_soldiers;
	delete m_add_soldiers;
}

uint32_t AttackBox::get_max_attackers() {
	assert(m_map);
	assert(m_pl);

	if (upcast(Building, building, m_map->get_immovable(*m_node)))
		return m_pl->find_attack_soldiers(building->base_flag());
	return 0;
}

UI::Slider & AttackBox::add_slider
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
	parent.add(&result, UI::Box::AlignCenter);
	return result;
}

UI::Textarea & AttackBox::add_text
	(UI::Box           & parent,
	 std::string   str,
	 uint32_t      alignment,
	 const std::string & fontname,
	 uint32_t      fontsize)
{
	UI::Textarea & result = *new UI::Textarea(&parent, str.c_str());
	UI::TextStyle textstyle;
	textstyle.font = UI::Font::get(fontname, fontsize);
	textstyle.bold = true;
	textstyle.fg = UI_FONT_CLR_FG;
	result.set_textstyle(textstyle);
	parent.add(&result, alignment);
	return result;
}

UI::Button & AttackBox::add_button
	(UI::Box           & parent,
	 const std::string & text,
	 void         (AttackBox::*fn)(),
	 const std::string & tooltip_text)
{
	UI::Button * button =
		new UI::Button
			(&parent, text,
			 8, 8, 26, 26,
			 g_gr->images().get("pics/but2.png"),
			 text,
			 tooltip_text);
	button->sigclicked.connect(boost::bind(fn, boost::ref(*this)));
	parent.add(button, Box::AlignCenter);
	return *button;
}

void AttackBox::update_attack() {
	assert(m_slider_soldiers);
	assert(m_text_soldiers);
	assert(m_less_soldiers);
	assert(m_add_soldiers);

	int32_t max_attackers = get_max_attackers();

	if (m_slider_soldiers->get_max_value() != max_attackers)
		m_slider_soldiers->set_max_value(max_attackers);

	m_slider_soldiers->set_enabled(max_attackers > 0);
	m_add_soldiers->set_enabled(max_attackers > m_slider_soldiers->get_value());
	m_less_soldiers  ->set_enabled(m_slider_soldiers->get_value() > 0);

	/** TRANSLATORS: %1% of %2% soldiers. Used in Attack box. */
	m_text_soldiers->set_text((boost::format(_("%1% / %2%"))
									  % m_slider_soldiers->get_value()
									  % max_attackers).str());

	m_add_soldiers->set_title(std::to_string(max_attackers));
}

void AttackBox::init() {

	assert(m_node);

	uint32_t max_attackers = get_max_attackers();

	UI::Box & linebox = *new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(&linebox, UI::Box::AlignCenter);
	add_text(linebox, _("Soldiers:"));
	linebox.add_space(8);

	m_less_soldiers =
		&add_button
			(linebox,
			 "0",
			 &AttackBox::send_less_soldiers,
			 _("Send less soldiers"));

	//  Spliter of soldiers
	UI::Box & columnbox = *new UI::Box(&linebox, 0, 0, UI::Box::Vertical);
	linebox.add(&columnbox, UI::Box::AlignCenter);

	const std::string attack_string =
			(boost::format(_("%1% / %2%")) % (max_attackers > 0 ? 1 : 0) % max_attackers).str();

	m_text_soldiers =
		&add_text(columnbox, attack_string, UI::Box::AlignCenter,
					 UI::g_fh1->fontset().serif(),
					 UI_FONT_SIZE_ULTRASMALL);

	m_slider_soldiers =
		&add_slider
			(columnbox,
			 100, 10,
			 0, max_attackers, max_attackers > 0 ? 1 : 0,
			 "pics/but2.png",
			 _("Number of soldiers"));

	m_slider_soldiers->changed.connect(boost::bind(&AttackBox::update_attack, this));
	m_add_soldiers =
		&add_button
			(linebox,
			 std::to_string(max_attackers),
			 &AttackBox::send_more_soldiers,
			 _("Send more soldiers"));

	m_slider_soldiers->set_enabled(max_attackers > 0);
	m_add_soldiers   ->set_enabled(max_attackers > 0);
	m_less_soldiers  ->set_enabled(max_attackers > 0);
}

void AttackBox::send_less_soldiers() {
	assert(m_slider_soldiers);
	m_slider_soldiers->set_value(m_slider_soldiers->get_value() - 1);
}

void AttackBox::send_more_soldiers() {
	m_slider_soldiers->set_value(m_slider_soldiers->get_value() + 1);
}

uint32_t AttackBox::soldiers() const {
	assert(m_slider_soldiers);
	return m_slider_soldiers->get_value();
}
