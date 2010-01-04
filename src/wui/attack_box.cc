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

#include "attack_box.h"

#include "editor/editorinteractive.h"
#include "logic/soldier.h"

#include "upcast.h"

#if 0
static char const * const pic_more_soldiers    = "pics/attack_add_soldier.png";
static char const * const pic_less_soldiers    = "pics/attack_sub_soldier.png";
static char const * const pic_more_brave       = "pics/attack_add_soldier.png";
static char const * const pic_less_brave       = "pics/attack_sub_soldier.png";
#endif

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
	m_slider_retreat(0),
	m_slider_soldiers(0),
	m_text_soldiers(0),
	m_text_retreat(0),
	m_add_soldiers(0)
{
	init();
}

AttackBox::~AttackBox() {
	delete m_slider_retreat;
	delete m_slider_soldiers;
	delete m_text_soldiers;
	delete m_text_retreat;
	delete m_add_soldiers;
}

uint32_t AttackBox::get_max_attackers() {
	assert(m_map);
	assert(m_pl);

	if (upcast(Building, building, m_map->get_immovable(*m_node)))
		return m_pl->findAttackSoldiers(building->base_flag());
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
			 g_gr->get_picture(PicMod_Game, picname),
			 hint);
	parent.add(&result, UI::Box::AlignTop);
	return result;
}

UI::Textarea & AttackBox::add_text
	(UI::Box           & parent,
	 std::string   str,
	 uint32_t      alignment,
	 std::string const & fontname,
	 uint32_t      fontsize)
{
	UI::Textarea & result = *new UI::Textarea(&parent, 0, 0, str.c_str());
	result.set_font(fontname, fontsize, UI_FONT_CLR_FG);
	parent.add(&result, alignment);
	return result;
}

UI::Callback_Button<AttackBox> & AttackBox::add_button
	(UI::Box           & parent,
	 char        const * const text,
	 void         (AttackBox::*fn)(),
	 std::string const & tooltip_text)
{
	UI::Callback_Button<AttackBox> & button =
		*new UI::Callback_Button<AttackBox>
			(&parent,
			 8, 8, 26, 26,
			 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
			 fn,
			 *this,
			 text,
			 tooltip_text);
	parent.add(&button, Box::AlignTop);
	return button;
}

void AttackBox::update_attack() {
	assert(m_slider_soldiers);
	assert(m_text_soldiers);

	char buf[20];
	int32_t max_attackers = get_max_attackers();

	if (m_slider_soldiers->get_max_value() != max_attackers)
		m_slider_soldiers->set_max_value(max_attackers);

	sprintf(buf, "%u / %u", m_slider_soldiers->get_value(), max_attackers);
	m_text_soldiers->set_text(buf);

	sprintf(buf, "%u", max_attackers);
	m_add_soldiers->set_title(buf);


#if 0
	assert(m_slider_retreat);
	sprintf(buf, "%u%%", m_slider_retreat->get_value());
	m_text_retreat->set_text(buf);
#endif
}

void AttackBox::init() {
	char buf[10];
	assert(m_node);

	uint32_t max_attackers = get_max_attackers();


	{ //  Soldiers line
		UI::Box & linebox = *new UI::Box(this, 0, 0, UI::Box::Horizontal);
		add(&linebox, UI::Box::AlignTop);
		add_text(linebox, _("Soldiers:"));

		add_button
			(linebox,
			 "0",
			 &AttackBox::send_less_soldiers,
			 _("Send less soldiers"));

		//  Spliter of soldiers
		UI::Box & columnbox = *new UI::Box(&linebox, 0, 0, UI::Box::Vertical);
		linebox.add(&columnbox, UI::Box::AlignTop);

		sprintf(buf, "%u / %u", max_attackers > 0 ? 1 : 0, max_attackers);

		m_text_soldiers =
			&add_text(columnbox, buf, UI::Box::AlignCenter, UI_FONT_ULTRASMALL);

		m_slider_soldiers =
			&add_slider
				(columnbox,
				 100, 10,
				 0, max_attackers, max_attackers > 0 ? 1 : 0,
				 "slider.png",
				 _("Number of soldiers"));

		m_slider_soldiers->changed.set(this, &AttackBox::update_attack);

		sprintf(buf, "%u", max_attackers);
		m_add_soldiers =
			&add_button
				(linebox,
				 buf,
				 &AttackBox::send_more_soldiers,
				 _("Send more soldiers"));
	}

	{ //  Retreat line
		UI::Box & linebox = *new UI::Box(this, 0, 0, UI::Box::Horizontal);
		add(&linebox, UI::Box::AlignTop);

		add_text(linebox, _("Retreat: Never!"));
		m_slider_retreat =
			&add_slider
				(linebox,
				 100, 10,
				 0, 100, m_pl->get_retreat_percentage(),
				 "slider.png",
				 _("Supported damage before retreat"));
		m_slider_retreat->changed.set(this, &AttackBox::update_attack);
		add_text(linebox, _("Once injured"));
		m_slider_retreat->set_enabled(m_pl->is_retreat_change_allowed());
		linebox.set_visible(m_pl->is_retreat_change_allowed());
	}

	//  Add here another attack configuration parameters.
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

uint8_t AttackBox::retreat() const {
	assert(m_slider_retreat);
	return m_slider_retreat->get_value();
}
