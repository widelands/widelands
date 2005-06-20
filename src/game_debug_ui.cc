/*
 * Copyright (C) 2004 by Widelands Development Team
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

#include <stdio.h>
#include "bob.h"
#include "field.h"
#include "graphic.h"
#include "instances.h"
#include "interactive_base.h"
#include "map.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_panel.h"
#include "ui_tabpanel.h"
#include "ui_window.h"

/*
==============================================================================

MapObjectDebugPanel

==============================================================================
*/

class MapObjectDebugPanel : public UIPanel, public Map_Object::LogSink {
public:
	MapObjectDebugPanel(UIPanel* parent, Editor_Game_Base* egbase, Map_Object* obj);
	~MapObjectDebugPanel();

	virtual void log(std::string str);

private:
	Editor_Game_Base*		m_egbase;
	Object_Ptr				m_object;

	UIMultiline_Textarea*	m_log;
};


/*
===============
MapObjectDebugPanel::MapObjectDebugPanel

Initialize logging.
===============
*/
MapObjectDebugPanel::MapObjectDebugPanel(UIPanel* parent, Editor_Game_Base* egbase, Map_Object* obj)
	: UIPanel(parent, 0, 0, 280, 150)
{
	m_egbase = egbase;
	m_object = obj;

	m_log = new UIMultiline_Textarea(this, 0, 0, 280, 150, "");
	m_log->set_scrollmode(UIMultiline_Textarea::ScrollLog);

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
void Map_Object::create_debug_panels(Editor_Game_Base* egbase, UITab_Panel* tabs)
{
	tabs->add(g_gr->get_picture( PicMod_Game,  "pics/menu_debug.png" ),
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
class MapObjectDebugWindow : public UIWindow {
public:
	MapObjectDebugWindow(Interactive_Base* parent, Map_Object* obj);

	Interactive_Base* get_iabase() { return (Interactive_Base*)get_parent(); }

	virtual void think();

private:
   bool           m_log_general_info;
	Object_Ptr		m_object;
	uint				m_serial;
	UITab_Panel*	m_tabs;
};



/*
===============
MapObjectDebugWindow::MapObjectDebugWindow

Create the window
===============
*/
MapObjectDebugWindow::MapObjectDebugWindow(Interactive_Base* parent, Map_Object* obj)
	: UIWindow(parent, 0, 0, 100, 100, "")
{
	char buf[128];

	m_object = obj;

	m_serial = obj->get_serial();
	snprintf(buf, sizeof(buf), "%u", m_serial);
	set_title(buf);

	m_tabs = new UITab_Panel(this, 0, 0, 1);

	obj->create_debug_panels(parent->get_egbase(), m_tabs);

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
	Map_Object* obj = m_object.get(get_iabase()->get_egbase());

   if(obj && m_log_general_info)  {
      obj->log_general_info(get_iabase()->get_egbase());
      m_log_general_info = false;
   }

	if (!obj) {
		char buf[128];

		snprintf(buf, sizeof(buf), "DEAD: %u", m_serial);
		set_title(buf);
		return;
	}

	UIWindow::think();
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

class FieldDebugWindow : public UIWindow {
public:
	FieldDebugWindow(Interactive_Base* parent, Coords coords);
	~FieldDebugWindow();

	Interactive_Base* get_iabase() { return (Interactive_Base*)get_parent(); }

	virtual void think();

	void open_immovable();
	void open_bob(int idx);

private:
	Map*			m_map;
	FCoords		m_coords;

	UIMultiline_Textarea*	m_ui_field;
	UIButton*					m_ui_immovable;
	UIListselect*				m_ui_bobs;
};


/*
===============
FieldDebugWindow::FieldDebugWindow

Initialize the field debug window.
===============
*/
FieldDebugWindow::FieldDebugWindow(Interactive_Base* parent, Coords coords)
	: UIWindow(parent, 0, 0, 200, 200, "Debug Field")
{
	m_map = parent->get_map();
	m_coords = m_map->get_fcoords(coords);

	// Setup child panels
	m_ui_field = new UIMultiline_Textarea(this, 0, 0, 200, 80, "");

	m_ui_immovable = new UIButton(this, 0, 80, 200, 24, 0);
	m_ui_immovable->clicked.set(this, &FieldDebugWindow::open_immovable);

	m_ui_bobs = new UIListselect(this, 0, 104, 200, 96);
	m_ui_bobs->selected.set(this, &FieldDebugWindow::open_bob);
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
	char buf[512];

	UIWindow::think();

	// Select information about the field itself
	str = "";
	snprintf(buf, sizeof(buf), "%i, %i - height: %i\n",
			m_coords.x, m_coords.y, m_coords.field->get_height());
	str += buf;

	m_ui_field->set_text(str.c_str());

	// Immovable information
	BaseImmovable* imm = m_coords.field->get_immovable();

	if (imm) {
		std::string name;

		switch(imm->get_type()) {
		case Map_Object::IMMOVABLE: name = ((Immovable*)imm)->get_name(); break;
		case Map_Object::BUILDING: name = ((Building*)imm)->get_name(); break;
		case Map_Object::FLAG: name = "flag"; break;
		case Map_Object::ROAD: name = "road"; break;
		default: name = "unknown"; break;
		}

		snprintf(buf, sizeof(buf), "%s (%u)", name.c_str(), imm->get_serial());
		m_ui_immovable->set_title(buf);
		m_ui_immovable->set_enabled(true);
	} else {
		m_ui_immovable->set_title("no immovable");
		m_ui_immovable->set_enabled(false);
	}

	// Bobs information
	std::vector<Bob*> bobs;

	m_ui_bobs->clear();

	m_map->find_bobs(m_coords, 0, &bobs);
	for(std::vector<Bob*>::iterator it = bobs.begin(); it != bobs.end(); ++it) {
		snprintf(buf, sizeof(buf), "%s (%u)", (*it)->get_name().c_str(), (*it)->get_serial());
		m_ui_bobs->add_entry(buf, (void*)(*it)->get_serial());
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
void FieldDebugWindow::open_bob(int idx)
{
	unsigned long serial = (unsigned long)m_ui_bobs->get_selection();

	if (!serial)
		return;

	Object_Manager* objmgr = get_iabase()->get_egbase()->get_objects();
	Map_Object* object = objmgr->get_object(serial);

	if (object)
		show_mapobject_debug(get_iabase(), object);
}


/*
===============
show_field_debug

Open a debug window for the given field.
===============
*/
void show_field_debug(Interactive_Base *parent, Coords coords)
{
	new FieldDebugWindow(parent, coords);
}
