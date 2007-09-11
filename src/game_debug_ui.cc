/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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
// UI classes for real-time game debugging

#include "bob.h"
#include "building.h"
#include "field.h"
#include "graphic.h"
#include "i18n.h"
#include "instances.h"
#include "interactive_base.h"
#include "map.h"

#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_panel.h"
#include "ui_tabpanel.h"
#include "ui_window.h"

#include <stdio.h>


struct MapObjectDebugPanel : public UI::Panel, public Map_Object::LogSink {
	MapObjectDebugPanel(UI::Panel* parent, Editor_Game_Base* egbase, Map_Object* obj);
	~MapObjectDebugPanel();

	virtual void log(std::string str);

private:
	Editor_Game_Base       * m_egbase;
	Object_Ptr               m_object;

	UI::Multiline_Textarea * m_log;
};


/*
===============
MapObjectDebugPanel::MapObjectDebugPanel

Initialize logging.
===============
*/
MapObjectDebugPanel::MapObjectDebugPanel(UI::Panel* parent, Editor_Game_Base* egbase, Map_Object* obj)
	: UI::Panel(parent, 0, 0, 280, 150)
{
	m_egbase = egbase;
	m_object = obj;

	m_log = new UI::Multiline_Textarea(this, 0, 0, 280, 150, "");
	m_log->set_scrollmode(UI::Multiline_Textarea::ScrollLog);

	obj->set_logsink(this);
}


/*
===============
MapObjectDebugPanel::~MapObjectDebugPanel

Unregister logger.
===============
*/
MapObjectDebugPanel::~MapObjectDebugPanel()
{
	Map_Object* obj = m_object.get(m_egbase);

	if (obj && obj->get_logsink() == this)
		obj->set_logsink(0);
}


/*
===============
MapObjectDebugPanel::log

Append the string to the log textarea.
===============
*/
void MapObjectDebugPanel::log(std::string str)
{
	m_log->set_text((m_log->get_text() + str).c_str());
}


/*
===============
Map_Object::create_debug_panels

Create tabs for the debugging UI.

This is separated out of instances.cc here, so we don't have to include
UI headers in the game logic code (same reason why we have a separate
building_ui.cc).
===============
*/
void Map_Object::create_debug_panels(Editor_Game_Base* egbase, UI::Tab_Panel* tabs)
{
	tabs->add(g_gr->get_picture(PicMod_Game,  "pics/menu_debug.png"),
			new MapObjectDebugPanel(tabs, egbase, this));
}


/*
==============================================================================

MapObjectDebugWindow

==============================================================================
*/

/*
MapObjectDebugWindow
--------------------
The map object debug window is basically just a simple container for tabs
that are provided by the map object itself via the virtual function
collect_debug_tabs().
*/
class MapObjectDebugWindow : public UI::Window {
public:
	MapObjectDebugWindow(Interactive_Base* parent, Map_Object* obj);

	Interactive_Base* get_iabase() {return (Interactive_Base*)get_parent();}

	virtual void think();

private:
   bool           m_log_general_info;
	Object_Ptr      m_object;
	uint            m_serial;
	UI::Tab_Panel * m_tabs;
};



/*
===============
MapObjectDebugWindow::MapObjectDebugWindow

Create the window
===============
*/
MapObjectDebugWindow::MapObjectDebugWindow(Interactive_Base* parent, Map_Object* obj)
	: UI::Window(parent, 0, 0, 100, 100, "")
{
	char buffer[128];

	m_object = obj;

	m_serial = obj->get_serial();
	snprintf(buffer, sizeof(buffer), "%u", m_serial);
	set_title(buffer);

	m_tabs = new UI::Tab_Panel(this, 0, 0, 1);

	obj->create_debug_panels(&parent->egbase(), m_tabs);

	m_tabs->set_snapparent(true);
	m_tabs->resize();

   m_log_general_info = true;
}


/*
===============
MapObjectDebugWindow::think

Remove self when the object disappears.
===============
*/
void MapObjectDebugWindow::think()
{
	Editor_Game_Base & egbase = get_iabase()->egbase();
	Map_Object * const obj = m_object.get(&egbase);

	if (obj) {
		if (m_log_general_info)  {
      obj->log_general_info(&egbase);
      m_log_general_info = false;
		}
		UI::Window::think();
	} else {
		char buffer[128];

		snprintf(buffer, sizeof(buffer), "DEAD: %u", m_serial);
		set_title(buffer);
	}

}


/*
===============
show_mapobject_debug

Show debug window for a Map_Object
===============
*/
void show_mapobject_debug(Interactive_Base* parent, Map_Object* obj)
{
	new MapObjectDebugWindow(parent, obj);
}


/*
==============================================================================

FieldDebugWindow

==============================================================================
*/

class FieldDebugWindow : public UI::Window {
public:
	FieldDebugWindow(Interactive_Base & parent, const Coords);
	~FieldDebugWindow();

	Interactive_Base* get_iabase() {return (Interactive_Base*)get_parent();}

	virtual void think();

	void open_immovable();
	void open_bob(const uint index);

private:
	Map &                        m_map;
	const FCoords                m_coords;

	UI::Multiline_Textarea       m_ui_field;
	UI::Button<FieldDebugWindow> m_ui_immovable;
	UI::Listselect<uintptr_t>    m_ui_bobs;
};


/*
===============
FieldDebugWindow::FieldDebugWindow

Initialize the field debug window.
===============
*/
FieldDebugWindow::FieldDebugWindow
(Interactive_Base & parent, const Coords coords)
:
UI::Window(&parent, 0, 60, 200, 400, _("Debug Field").c_str()),
m_map     (parent.egbase().map()),
m_coords  (m_map.get_fcoords(coords)),

	// Setup child panels
m_ui_field(this, 0, 0, 200, 280, ""),

m_ui_immovable
(this,
 0, 280, 200, 24,
 0,
 &FieldDebugWindow::open_immovable, this,
 ""),

m_ui_bobs(this, 0, 304, 200, 96)

{
	assert(0 <= m_coords.x);
	assert(m_coords.x < m_map.get_width());
	assert(0 <= m_coords.y);
	assert(m_coords.y < m_map.get_height());
	assert(&m_map[0] <= m_coords.field);
	assert             (m_coords.field < &m_map[0] + m_map.max_index());
	m_ui_bobs.selected.set(this, &FieldDebugWindow::open_bob);
}


/*
===============
FieldDebugWindow::~FieldDebugWindow
===============
*/
FieldDebugWindow::~FieldDebugWindow()
{
}


/*
===============
FieldDebugWindow::think

Gather information about the field and update the UI elements.
This is done every frame in order to have up to date information all the time.
===============
*/
void FieldDebugWindow::think()
{
	std::string str;
	char buffer[512];

	UI::Window::think();

	// Select information about the field itself
	str = "";
	snprintf
		(buffer, sizeof(buffer),
		 "%i, %i - %s %i\n",
		 m_coords.x, m_coords.y,
		 _("height:").c_str(), m_coords.field->get_height());
	str += buffer;
	const Map::Index i = m_coords.field - &m_map[0];
	const Editor_Game_Base & egbase =
		dynamic_cast<const Interactive_Base &>(*get_parent()).egbase();
	const Player_Number nr_players = m_map.get_nrplayers();
	for (Player_Number plnum = 1; plnum <= nr_players; ++plnum)
		if (const Player * const player = egbase.get_player(plnum)) {
			const Player::Field & player_field = player->fields()[i];
			snprintf(buffer, sizeof(buffer), "Player %u:\n", plnum);
			str += buffer;
			snprintf
				(buffer, sizeof(buffer),
				 "  military influence: %u\n", player_field.military_influence);
			str += buffer;
			Vision vision = player_field.vision;
			snprintf(buffer, sizeof(buffer), "  vision: %u\n", vision);
			str += buffer;
			{
				const Time time_last_surveyed =
					player_field.time_triangle_last_surveyed[TCoords<>::D];
				if (time_last_surveyed != Editor_Game_Base::Never()) {
					snprintf
						(buffer, sizeof(buffer),
						 "  D triangle last surveyed at %u: amount %u\n",
						 time_last_surveyed, player_field.resource_amounts.d);
					str += buffer;
				} else str += "  D triangle never surveyed\n";
			}
			{
				const Time time_last_surveyed =
					player_field.time_triangle_last_surveyed[TCoords<>::R];
				if (time_last_surveyed != Editor_Game_Base::Never()) {
					snprintf
						(buffer, sizeof(buffer),
						 "  R triangle last surveyed at %u: amount %u\n",
						 time_last_surveyed, player_field.resource_amounts.r);
					str += buffer;
				} else str += "  R triangle never surveyed\n";
		}
			switch (vision) {
			case 0: str += "  never seen\n"; break;
			case 1:
				snprintf
					(buffer, sizeof(buffer),
					 "  last seen at %u:\n"
					 "    owner: %u\n"
					 "    immovable animation:\n%s\n"
					 "      ",
					 player_field.time_node_last_unseen,
					 player_field.owner,
					 player_field.map_object_descr[TCoords<>::None] ?
					 g_anim.get_animation
					 (player_field.map_object_descr[TCoords<>::None]
					  ->main_animation())
					 ->picnametempl.c_str()
					 :
					 "(none)");
				str += buffer;
				break;
			default:
				snprintf(buffer, sizeof(buffer), "  seen %u times\n", vision - 1);
				str +=  buffer;
			}
		}

	m_ui_field.set_text(str.c_str());

	// Immovable information
	BaseImmovable* imm = m_coords.field->get_immovable();

	if (imm) {
		std::string name;

		switch (imm->get_type()) {
		case Map_Object::IMMOVABLE: name = ((Immovable*)imm)->name(); break;
		case Map_Object::BUILDING: name = ((Building*)imm)->name(); break;
		case Map_Object::FLAG: name = "flag"; break;
		case Map_Object::ROAD: name = "road"; break;
		default: name = "unknown"; break;
		}

		snprintf
			(buffer, sizeof(buffer), "%s (%u)", name.c_str(), imm->get_serial());
		m_ui_immovable.set_title(buffer);
		m_ui_immovable.set_enabled(true);
	} else {
		m_ui_immovable.set_title("no immovable");
		m_ui_immovable.set_enabled(false);
	}

	// Bobs information
	std::vector<Bob*> bobs;

	m_ui_bobs.clear();

	m_map.find_bobs(Area<FCoords>(m_coords, 0), &bobs);
	for (std::vector<Bob*>::iterator it = bobs.begin(); it != bobs.end(); ++it) {
		snprintf
			(buffer, sizeof(buffer),
			 "%s (%u)", (*it)->name().c_str(), (*it)->get_serial());
		m_ui_bobs.add(buffer, (*it)->get_serial());
	}
}


/*
===============
FieldDebugWindow::open_immovable

Open the debug window for the immovable on our position.
===============
*/
void FieldDebugWindow::open_immovable()
{
	BaseImmovable* imm = m_coords.field->get_immovable();

	if (imm)
		show_mapobject_debug(get_iabase(), imm);
}


/*
===============
FieldDebugWindow::open_bob

Open the bob debug window for the bob of the given index in the list
===============
*/
void FieldDebugWindow::open_bob(const uint index) {
	if (index != UI::Listselect<uintptr_t>::no_selection_index())
		if
			(Map_Object * const object =
			 get_iabase()->egbase().objects().get_object(m_ui_bobs.get_selected()))
			show_mapobject_debug(get_iabase(), object);
}


/*
===============
show_field_debug

Open a debug window for the given field.
===============
*/
void show_field_debug(Interactive_Base *parent, Coords coords)
{new FieldDebugWindow(*parent, coords);}
