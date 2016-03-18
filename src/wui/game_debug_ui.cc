/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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
// UI classes for real-time game debugging

#include "wui/game_debug_ui.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/interactive_base.h"

struct MapObjectDebugPanel
: public UI::Panel, public Widelands::MapObject::LogSink
{
	MapObjectDebugPanel
		(UI::Panel                   & parent,
		 const Widelands::EditorGameBase &,
		 Widelands::MapObject       &);
	~MapObjectDebugPanel();

	void log(std::string str) override;

private:
	const Widelands::EditorGameBase & m_egbase;
	Widelands::ObjectPointer         m_object;

	UI::MultilineTextarea        m_log;
};


MapObjectDebugPanel::MapObjectDebugPanel
	(UI::Panel                   & parent,
	 const Widelands::EditorGameBase & egbase,
	 Widelands::MapObject       & obj)
:
UI::Panel(&parent, 0, 0, 350, 200),
m_egbase (egbase),
m_object (&obj),
m_log    (this, 0, 0, 350, 200, "", UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kScrollLog)
{
	obj.set_logsink(this);
}


MapObjectDebugPanel::~MapObjectDebugPanel()
{
	if (Widelands::MapObject * const obj = m_object.get(m_egbase))
		if (obj->get_logsink() == this)
			obj->set_logsink(nullptr);
}


/*
===============
Append the string to the log textarea.
===============
*/
void MapObjectDebugPanel::log(std::string str)
{
	m_log.set_text((m_log.get_text() + str).c_str());
}


/*
===============
Create tabs for the debugging UI.

This is separated out of instances.cc here, so we don't have to include
UI headers in the game logic code (same reason why we have a separate
building_ui.cc).
===============
*/
void Widelands::MapObject::create_debug_panels
	(const Widelands::EditorGameBase & egbase, UI::TabPanel & tabs)
{
	tabs.add
		("debug", g_gr->images().get("images/wui/fieldaction/menu_debug.png"),
		 new MapObjectDebugPanel(tabs, egbase, *this));
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
struct MapObjectDebugWindow : public UI::Window {
	MapObjectDebugWindow(InteractiveBase & parent, Widelands::MapObject &);

	InteractiveBase & ibase() {
		return dynamic_cast<InteractiveBase&>(*get_parent());
	}

	void think() override;

private:
	bool                  m_log_general_info;
	Widelands::ObjectPointer m_object;
	uint32_t              m_serial;
	UI::TabPanel         m_tabs;
};


MapObjectDebugWindow::MapObjectDebugWindow
	(InteractiveBase & parent, Widelands::MapObject & obj)
	:
	UI::Window        (&parent, "map_object_debug", 0, 0, 100, 100, ""),
	m_log_general_info(true),
	m_object          (&obj),
	m_tabs
		(this, 0, 0,
		 g_gr->images().get("images/ui_basic/but4.png"))
{
	m_serial = obj.serial();
	set_title(std::to_string(m_serial));

	obj.create_debug_panels(parent.egbase(), m_tabs);

	set_center_panel(&m_tabs);
}


/*
===============
Remove self when the object disappears.
===============
*/
void MapObjectDebugWindow::think()
{
	Widelands::EditorGameBase & egbase = ibase().egbase();
	if (Widelands::MapObject * const obj = m_object.get(egbase)) {
		if (m_log_general_info)  {
			obj->log_general_info(egbase);
			m_log_general_info = false;
		}
		UI::Window::think();
	} else {
		set_title((boost::format("DEAD: %u") % m_serial).str());
	}

}


/*
===============
show_mapobject_debug

Show debug window for a MapObject
===============
*/
void show_mapobject_debug
	(InteractiveBase & parent, Widelands::MapObject & obj)
{
	new MapObjectDebugWindow(parent, obj);
}


/*
==============================================================================

FieldDebugWindow

==============================================================================
*/

struct FieldDebugWindow : public UI::Window {
	FieldDebugWindow(InteractiveBase & parent, Widelands::Coords);

	InteractiveBase & ibase() {
		return dynamic_cast<InteractiveBase&>(*get_parent());
	}

	void think() override;

	void open_immovable();
	void open_bob(uint32_t);

private:
	Widelands::Map             & m_map;
	Widelands::FCoords const     m_coords;

	UI::MultilineTextarea       m_ui_field;
	UI::Button m_ui_immovable;
	UI::Listselect<intptr_t>    m_ui_bobs;
};


FieldDebugWindow::FieldDebugWindow
	(InteractiveBase & parent, Widelands::Coords const coords)
:
	UI::Window(&parent, "field_debug", 0, 60, 300, 400, _("Debug Field")),
	m_map     (parent.egbase().map()),
	m_coords  (m_map.get_fcoords(coords)),

	//  setup child panels
	m_ui_field(this, 0, 0, 300, 280, ""),

	m_ui_immovable
		(this, "immovable",
		 0, 280, 300, 24,
		 g_gr->images().get("images/ui_basic/but4.png"),
		 ""),

	m_ui_bobs(this, 0, 304, 300, 96)
{
	m_ui_immovable.sigclicked.connect(boost::bind(&FieldDebugWindow::open_immovable, this));

	assert(0 <= m_coords.x);
	assert(m_coords.x < m_map.get_width());
	assert(0 <= m_coords.y);
	assert(m_coords.y < m_map.get_height());
	assert(&m_map[0] <= m_coords.field);
	assert             (m_coords.field < &m_map[0] + m_map.max_index());
	m_ui_bobs.selected.connect(boost::bind(&FieldDebugWindow::open_bob, this, _1));
}


/*
===============
Gather information about the field and update the UI elements.
This is done every frame in order to have up to date information all the time.
===============
*/
void FieldDebugWindow::think()
{
	std::string str;

	UI::Window::think();

	// Select information about the field itself
	const Widelands::EditorGameBase & egbase =
		dynamic_cast<const InteractiveBase&>(*get_parent())
		.egbase();
	{
		Widelands::PlayerNumber const owner = m_coords.field->get_owned_by();
		str += (boost::format("(%i, %i)\nheight: %u\nowner: %u\n")
				  % m_coords.x % m_coords.y
				  % static_cast<unsigned int>(m_coords.field->get_height())
				  % static_cast<unsigned int>(owner)).str();

		if (owner) {
			Widelands::NodeCaps const buildcaps =
				egbase.player(owner).get_buildcaps(m_coords);
			if      (buildcaps & Widelands::BUILDCAPS_BIG)
				str += "  can build big building\n";
			else if (buildcaps & Widelands::BUILDCAPS_MEDIUM)
				str += "  can build medium building\n";
			else if (buildcaps & Widelands::BUILDCAPS_SMALL)
				str += "  can build small building\n";
			if      (buildcaps & Widelands::BUILDCAPS_FLAG)
				str += "  can place flag\n";
			if      (buildcaps & Widelands::BUILDCAPS_MINE)
				str += "  can build mine\n";
			if      (buildcaps & Widelands::BUILDCAPS_PORT)
				str += "  can build port\n";
		}
	}
	if (m_coords.field->nodecaps() & Widelands::MOVECAPS_WALK)
		str += "is walkable\n";
	if (m_coords.field->nodecaps() & Widelands::MOVECAPS_SWIM)
		str += "is swimable\n";
	Widelands::MapIndex const i = m_coords.field - &m_map[0];
	Widelands::PlayerNumber const nr_players = m_map.get_nrplayers();
	iterate_players_existing_const(plnum, nr_players, egbase, player) {
		const Widelands::Player::Field & player_field = player->fields()[i];
		str += (boost::format("Player %u:\n") % static_cast<unsigned int>(plnum)).str();
		str += (boost::format("  military influence: %u\n") % player_field.military_influence).str();

		Widelands::Vision const vision = player_field.vision;
		str += (boost::format("  vision: %u\n") % vision).str();
		{
			Widelands::Time const time_last_surveyed =
				player_field.time_triangle_last_surveyed[Widelands::TCoords<>::D];

			if (time_last_surveyed != Widelands::never()) {
				str += (boost::format("  D triangle last surveyed at %u: amount %u\n")
						  % time_last_surveyed
						  % static_cast<unsigned int>(player_field.resource_amounts.d)).str();

			} else str += "  D triangle never surveyed\n";
		}
		{
			Widelands::Time const time_last_surveyed =
				player_field.time_triangle_last_surveyed[Widelands::TCoords<>::R];

			if (time_last_surveyed != Widelands::never()) {
				str += (boost::format("  R triangle last surveyed at %u: amount %u\n")
						  % time_last_surveyed
						  % static_cast<unsigned int>(player_field.resource_amounts.r)).str();

			} else str += "  R triangle never surveyed\n";
		}
		switch (vision) {
		case 0: str += "  never seen\n"; break;
		case 1: {
			std::string animation_name = "(no animation)";
			if (player_field.map_object_descr[Widelands::TCoords<>::None]) {
				animation_name = "(seen an animation)";
			}

			str += (boost::format("  last seen at %u:\n"
										"    owner: %u\n"
										"    immovable animation:\n%s\n"
										"      ")
					  % player_field.time_node_last_unseen
					  % static_cast<unsigned int>(player_field.owner)
					  % animation_name.c_str()).str();
			break;
		}
		default:
			str += (boost::format("  seen %u times\n") % (vision - 1)).str();
			break;
		}
	}
	{
		const Widelands::DescriptionIndex ridx = m_coords.field->get_resources();

		if (ridx == Widelands::kNoResource) {
			str += "Resource: None\n";
		} else {
			const int ramount = m_coords.field->get_resources_amount();
			const int initial_amount = m_coords.field->get_initial_res_amount();

			str += (boost::format("Resource: %s\n")
					  % ibase().egbase().world().get_resource(ridx)->name().c_str()).str();

			str += (boost::format("  Amount: %i/%i\n") % ramount % initial_amount).str();
		}
	}

	m_ui_field.set_text(str.c_str());

	// Immovable information
	if (Widelands::BaseImmovable * const imm = m_coords.field->get_immovable())
	{
		m_ui_immovable.set_title((boost::format("%s (%u)")
										  % imm->descr().name().c_str()
										  % imm->serial()).str());
		m_ui_immovable.set_enabled(true);
	} else {
		m_ui_immovable.set_title("no immovable");
		m_ui_immovable.set_enabled(false);
	}

	// Bobs information
	std::vector<Widelands::Bob *> bobs;
	m_map.find_bobs(Widelands::Area<Widelands::FCoords>(m_coords, 0), &bobs);

	// Do not clear the list. Instead parse all bobs and sync lists
	for (uint32_t idx = 0; idx < m_ui_bobs.size(); idx++) {
		Widelands::MapObject* mo =
			ibase().egbase().objects().get_object(m_ui_bobs[idx]);
		bool toremove = false;
		std::vector<Widelands::Bob *>::iterator removeIt;
		// Nested loop :(

		for (std::vector<Widelands::Bob *>::iterator bob_iter = bobs.begin();
			  bob_iter != bobs.end();
			  ++bob_iter) {

			if ((*bob_iter) && mo && (*bob_iter)->serial() == mo->serial()) {
				// Remove from the bob list if we already
				// have it in our list
				toremove = true;
				removeIt = bob_iter;
				break;
			}
		}
		if (toremove) {
			bobs.erase(removeIt);
			continue;
		}
		// Remove from our list if its not in the bobs
		// list, or if it doesn't seem to exist anymore
		m_ui_bobs.remove(idx);
		idx--; // reiter the same index
	}
	// Add remaining
	for (const Widelands::Bob * temp_bob : bobs) {
		m_ui_bobs.add(
			(boost::format("%s (%u)")
				% temp_bob->descr().name()
				% temp_bob->serial()).str(),
			temp_bob->serial());
	}
}


/*
===============
Open the debug window for the immovable on our position.
===============
*/
void FieldDebugWindow::open_immovable()
{
	if (Widelands::BaseImmovable * const imm = m_coords.field->get_immovable())
		show_mapobject_debug(ibase(), *imm);
}


/*
===============
Open the bob debug window for the bob of the given index in the list
===============
*/
void FieldDebugWindow::open_bob(const uint32_t index) {
	if (index != UI::Listselect<intptr_t>::no_selection_index())
		if
			(Widelands::MapObject * const object =
			 	ibase().egbase().objects().get_object(m_ui_bobs.get_selected()))
			show_mapobject_debug(ibase(), *object);
}


/*
===============
show_field_debug

Open a debug window for the given field.
===============
*/
void show_field_debug
	(InteractiveBase & parent, Widelands::Coords const coords)
{
	new FieldDebugWindow(parent, coords);
}
