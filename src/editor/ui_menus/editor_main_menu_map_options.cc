/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_map_options.h"

#include <cstdio>

#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "profile/profile.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"


inline Editor_Interactive & Main_Menu_Map_Options::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


/**
 * Create all the buttons etc...
*/
Main_Menu_Map_Options::Main_Menu_Map_Options(Editor_Interactive & parent)
	:
	UI::Window
		(&parent, "map_options",
		 (parent.get_w() - 200) / 2, (parent.get_h() - 300) / 2, 200, 305,
		 _("Map Options"))
{

	int32_t const offsx   =  5;
	int32_t const offsy   =  5;
	int32_t const spacing =  3;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;
	UI::Textarea * ta = new UI::Textarea(this, posx, posy - 2, _("Map Name:"));
	m_name =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_name->changed.connect(boost::bind(&Main_Menu_Map_Options::changed, this, 0));
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Size:"));
	m_size =
		new UI::Textarea
			(this, posx + ta->get_w() + spacing, posy - 2, "512x512");
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Nr Players:"));
	m_nrplayers =
		new UI::Textarea(this, posx + ta->get_w() + spacing, posy - 2, "1");
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("World:"));
	m_world =
		new UI::Textarea
			(this, posx + ta->get_w() + spacing, posy - 2, "\"Greenland\"");
	posy += height + spacing;
	ta = new UI::Textarea(this, posx, posy - 2, _("Author:"));
	m_author =
		new UI::EditBox
			(this,
			 posx + ta->get_w() + spacing, posy,
			 get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 20,
			 g_gr->images().get("pics/but1.png"));
	m_author->changed.connect(boost::bind(&Main_Menu_Map_Options::changed, this, 1));
	posy += height + spacing;
	m_descr =
		new UI::Multiline_Editbox
			(this,
			 posx, posy,
			 get_inner_w() - spacing - posx, get_inner_h() - 25 - spacing - posy,
			 parent.egbase().map().get_description());
	m_descr->changed.connect(boost::bind(&Main_Menu_Map_Options::editbox_changed, this));

	UI::Button * btn =
		new UI::Button
			(this, "set_origin",
			 5, get_inner_h() - 25, get_inner_w() - 10, 20,
			 g_gr->images().get("pics/but0.png"),
			 _("Set origin"),
			 _
				("Set the position that will have the coordinates (0, 0). This will "
				 "be the top-left corner of a generated minimap."));
	btn->sigclicked.connect
		(boost::bind
		 (&Editor_Interactive::select_tool, &parent,
		  boost::ref(parent.tools.set_origin), Editor_Tool::First));

	update();
}

/**
 * Updates all UI::Textareas in the UI::Window to represent currently
 * set values
*/
void Main_Menu_Map_Options::update() {
	const Widelands::Map & map = eia().egbase().map();

	char buf[200];
	sprintf(buf, "%ix%i", map.get_width(), map.get_height());
	m_size->set_text(buf);
	m_author->setText(map.get_author());
	m_name  ->setText(map.get_name());
	sprintf(buf, "%i", map.get_nrplayers());
	m_nrplayers->set_text(buf);
	m_world ->set_text(map.world().get_name());
	m_descr ->set_text(map.get_description());
}


/**
 * Called when one of the editboxes are changed
*/
void Main_Menu_Map_Options::changed(int32_t const id) {
	if        (id == 0) {
		eia().egbase().map().set_name(m_name->text().c_str());
	} else if (id == 1) {
		eia().egbase().map().set_author(m_author->text().c_str());
		g_options.pull_section("global").set_string
			("realname", m_author->text());
	}
	update();
}

/**
 * Called when the editbox has changed
 */
void Main_Menu_Map_Options::editbox_changed() {
	eia().egbase().map().set_description(m_descr->get_text().c_str());
}
