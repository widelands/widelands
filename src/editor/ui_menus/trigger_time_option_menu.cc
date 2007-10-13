/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "trigger_time_option_menu.h"

#include "graphic.h"
#include "i18n.h"
#include "trigger/trigger_time.h"
#include "editorinteractive.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"


Trigger_Time_Option_Menu::Trigger_Time_Option_Menu
(Editor_Interactive & parent, Trigger_Time & trigger)
:
UI::Window(&parent, 0, 0, 164, 180, _("Trigger Option Menu").c_str()),
m_trigger (trigger)
{
   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   const int32_t width=20;
   const int32_t height=20;
   int32_t posx=offsx;
   int32_t posy=offsy;

	int32_t wait_time = trigger.get_wait_time();
   m_values[0]=(wait_time/3600)/10; // hours
   m_values[1]=(wait_time/3600)%10;
   wait_time-=(wait_time/3600)*3600;
   m_values[2]=(wait_time/60)/10; // minutes
   m_values[3]=(wait_time/60)%10;
   wait_time-=(wait_time/60)*60;
   m_values[4]=(wait_time)/10; // seconds
   m_values[5]=(wait_time)%10;

   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+50, posy, get_inner_w()-50-2*spacing, 20, 0, 0);
	m_name->set_text(trigger.get_name());

   posy+=20+spacing;

   // hours, first cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 2);

   m_textareas[0]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 3);

   posx+=width+spacing/2;

   // hours second cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 4);

   m_textareas[1]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 5);

   posx+=width+spacing;

   // two points
	UI::Textarea * tt = new UI::Textarea(this, posx, posy+23, ":", Align_Left);
   posx+=spacing+tt->get_w();

   // minutes, first cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 6);

   m_textareas[2]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 7);

   posx+=width+spacing/2;

   // minutes second cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 8);

   m_textareas[3]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 9);

   posx+=width+spacing;

   // two points
   tt=new UI::Textarea(this, posx, posy+23, ":", Align_Left);
   posx+=spacing+tt->get_w();
   // seconds, first cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 10);

   m_textareas[4]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 11);

   posx+=width+spacing/2;
   // seconds, second cipher

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 12);

   m_textareas[5]=new UI::Textarea(this, posx, posy+20, width, height, "9", Align_Center);

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy+40, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, this, 13);

   posx+=width+spacing;

   posy+=60;

   posy+=2*spacing;

   // Buttons
   posx=(get_inner_w()/2)-60-spacing;

	new UI::Button<Trigger_Time_Option_Menu>
		(this,
		 posx, posy, 60, 20,
		 0,
		 &Trigger_Time_Option_Menu::clicked_ok, this,
		 _("Ok"));

   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Trigger_Time_Option_Menu::end_modal, this, 0,
		 _("Cancel"));

   center_to_parent();
   update();
}


/*
 * Handle mousepress/-release
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Trigger_Time_Option_Menu::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Trigger_Time_Option_Menu::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}


void Trigger_Time_Option_Menu::clicked_ok() {
	m_trigger.set_wait_time
		((m_values[0] * 10 + m_values[1]) * 3600
		 +
		 (m_values[2] * 10 + m_values[3]) * 60
		 +
		 (m_values[4] * 10 + m_values[5]));
	if (m_name->get_text())
		m_trigger.set_name(m_name->get_text());
	end_modal(1);
}


void Trigger_Time_Option_Menu::clicked(int32_t i) {
   int32_t id=(i-2)/2;
   m_values[id]= i%2 ?  m_values[id]-1 :  m_values[id]+1;
   if (m_values[id]<0) m_values[id]=0;
   if (!m_values[0] && !m_values[1] && !m_values[2] && !m_values[3] && !m_values[4] && !m_values[5]) m_values[5]=1;
   if (m_values[id]>9) m_values[id]=9;
   if ((id==2 || id==4)  && m_values[id]>=6) m_values[id]=5;
   update();
}

/*
 * Update the Textareas
 */
void Trigger_Time_Option_Menu::update() {
	for (size_t i = 0; i < 6; ++i) {
      std::string str;
      str.append(1, static_cast<char>(m_values[i]+0x30));
      m_textareas[i]->set_text(str);
	}
}
