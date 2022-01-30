/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */
// UI classes for real-time game debugging

#include "wui/game_debug_ui.h"

#include "base/i18n.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/player.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/interactive_base.h"

MapObjectDebugPanel::MapObjectDebugPanel(UI::Panel& parent,
                                         const Widelands::EditorGameBase& egbase,
                                         Widelands::MapObject& obj)
   : UI::Panel(&parent, UI::PanelStyle::kWui, 0, 0, 350, 200),
     egbase_(egbase),
     object_(&obj),
     log_(this,
          0,
          0,
          350,
          200,
          UI::PanelStyle::kWui,
          "",
          UI::Align::kLeft,
          UI::MultilineTextarea::ScrollMode::kScrollLog) {
	obj.set_logsink(this);
}

MapObjectDebugPanel::~MapObjectDebugPanel() {
	if (Widelands::MapObject* const obj = object_.get(egbase_)) {
		if (obj->get_logsink() == this) {
			obj->set_logsink(nullptr);
		}
	}
}

/*
===============
Append the string to the log textarea.
===============
*/
void MapObjectDebugPanel::log(const std::string& str) {
	log_.set_text((log_.get_text() + str));
}

/*
==============================================================================

MapObjectDebugWindow

==============================================================================
*/
MapObjectDebugWindow::MapObjectDebugWindow(InteractiveBase& parent, Widelands::MapObject& obj)
   : UI::Window(&parent, UI::WindowStyle::kWui, "map_object_debug", 0, 0, 100, 100, ""),
     log_general_info_(true),
     object_(&obj),
     tabs_(this, UI::TabPanelStyle::kWuiLight) {
	serial_ = obj.serial();
	set_title(std::to_string(serial_));

	tabs_.add("debug", g_image_cache->get("images/wui/fieldaction/menu_debug.png"),
	          new MapObjectDebugPanel(tabs_, parent.egbase(), obj));

	set_center_panel(&tabs_);

	initialization_complete();
}

/*
===============
Remove self when the object disappears.
===============
*/
void MapObjectDebugWindow::think() {
	const Widelands::EditorGameBase& egbase = ibase().egbase();
	if (Widelands::MapObject* const obj = object_.get(egbase)) {
		if (log_general_info_) {
			obj->log_general_info(egbase);
			log_general_info_ = false;
		}
		UI::Window::think();
	} else {
		set_title(format("DEAD: %u", serial_));
	}
}

/*
===============
show_mapobject_debug

Show debug window for a MapObject
===============
*/
void show_mapobject_debug(InteractiveBase& parent, Widelands::MapObject& obj) {
	new MapObjectDebugWindow(parent, obj);
}

/*
==============================================================================

FieldDebugWindow

==============================================================================
*/
FieldDebugWindow::FieldDebugWindow(InteractiveBase& parent, Widelands::Coords const coords)
   : /** TRANSLATORS: Title for a window that shows debug information for a field on the map */
     UI::Window(&parent, UI::WindowStyle::kWui, "field_debug", 0, 60, 300, 400, _("Debug Field")),
     map_(parent.egbase().map()),
     coords_(map_.get_fcoords(coords)),

     //  setup child panels
     ui_field_(this, 0, 0, 300, 280, UI::PanelStyle::kWui, ""),

     ui_immovable_(this, "immovable", 0, 280, 300, 24, UI::ButtonStyle::kWuiMenu, ""),

     ui_bobs_(this, 0, 304, 300, 96, UI::PanelStyle::kWui) {
	ui_immovable_.sigclicked.connect([this]() { open_immovable(); });

	assert(0 <= coords_.x);
	assert(coords_.x < map_.get_width());
	assert(0 <= coords_.y);
	assert(coords_.y < map_.get_height());
	assert(&map_[0] <= coords_.field);
	assert(coords_.field < &map_[0] + map_.max_index());
	ui_bobs_.double_clicked.connect([this](uint32_t a) { open_bob(a); });

	initialization_complete();
}

/*
===============
Gather information about the field and update the UI elements.
This is done every frame in order to have up to date information all the time.
===============
*/
void FieldDebugWindow::think() {
	std::string str;

	UI::Window::think();

	// Select information about the field itself
	const Widelands::EditorGameBase& egbase =
	   dynamic_cast<const InteractiveBase&>(*get_parent()).egbase();
	{
		Widelands::PlayerNumber const owner = coords_.field->get_owned_by();
		str += format("(%i, %i)\nheight: %u\nowner: %u\n", coords_.x, coords_.y,
		              static_cast<unsigned int>(coords_.field->get_height()),
		              static_cast<unsigned int>(owner));

		if (owner) {
			Widelands::NodeCaps const buildcaps = egbase.player(owner).get_buildcaps(coords_);
			if (buildcaps & Widelands::BUILDCAPS_BIG) {
				str += "  can build big building\n";
			} else if (buildcaps & Widelands::BUILDCAPS_MEDIUM) {
				str += "  can build medium building\n";
			} else if (buildcaps & Widelands::BUILDCAPS_SMALL) {
				str += "  can build small building\n";
			}
			if (buildcaps & Widelands::BUILDCAPS_FLAG) {
				str += "  can place flag\n";
			}
			if (buildcaps & Widelands::BUILDCAPS_MINE) {
				str += "  can build mine\n";
			}
			if (buildcaps & Widelands::BUILDCAPS_PORT) {
				str += "  can build port\n";
			}
		}
	}
	if (coords_.field->nodecaps() & Widelands::MOVECAPS_WALK) {
		str += "is walkable\n";
	}
	if (coords_.field->nodecaps() & Widelands::MOVECAPS_SWIM) {
		str += "is swimmable\n";
	}
	Widelands::MapIndex const i = coords_.field - &map_[0];
	Widelands::PlayerNumber const nr_players = map_.get_nrplayers();
	iterate_players_existing_const(plnum, nr_players, egbase, player) {
		const Widelands::Player::Field& player_field = player->fields()[i];
		str += format("Player %u:\n", static_cast<unsigned int>(plnum));
		str += format("  military influence: %u\n", player_field.military_influence);

		Widelands::Vision const vision = player_field.vision;
		str += format("  vision: %u\n", vision.value());
		{
			Time const time_last_surveyed =
			   player_field.time_triangle_last_surveyed[static_cast<int>(Widelands::TriangleIndex::D)];

			if (time_last_surveyed.is_valid()) {
				str += format("  D triangle last surveyed at %u: amount %u\n", time_last_surveyed.get(),
				              static_cast<unsigned int>(player_field.resource_amounts.d));

			} else {
				str += "  D triangle never surveyed\n";
			}
		}
		{
			Time const time_last_surveyed =
			   player_field.time_triangle_last_surveyed[static_cast<int>(Widelands::TriangleIndex::R)];

			if (time_last_surveyed.is_valid()) {
				str += format("  R triangle last surveyed at %u: amount %u\n", time_last_surveyed.get(),
				              static_cast<unsigned int>(player_field.resource_amounts.r));

			} else {
				str += "  R triangle never surveyed\n";
			}
		}

		if (!vision.is_explored()) {
			str += "  never seen\n";
		} else if (!vision.is_visible()) {
			std::string animation_name = "(no animation)";
			if (player_field.map_object_descr) {
				animation_name = "(seen an animation)";
			}
			str += format("  last seen at %u:\n"
			              "    owner: %u\n"
			              "    immovable animation:\n%s\n"
			              "      ",
			              player_field.time_node_last_unseen.get(),
			              static_cast<unsigned int>(player_field.owner), animation_name.c_str());
		} else if (!vision.is_seen_by_us()) {
			str += "  seen only by teammate(s)\n";
		} else {
			if (vision.is_revealed()) {
				str += "  permanently revealed\n";
			} else if (vision.is_hidden()) {
				str += "  permanently hidden\n";
			}
			str += format("  seen %u times\n", vision.seers());
		}
	}
	{
		const Widelands::DescriptionIndex ridx = coords_.field->get_resources();

		if (ridx == Widelands::kNoResource) {
			str += "Resource: None\n";
		} else {
			const Widelands::ResourceAmount ramount = coords_.field->get_resources_amount();
			const Widelands::ResourceAmount initial_amount = coords_.field->get_initial_res_amount();

			str += format("Resource: %s\n",
			              ibase().egbase().descriptions().get_resource_descr(ridx)->name().c_str());

			str += format("  Amount: %i/%i\n", static_cast<unsigned int>(ramount),
			              static_cast<unsigned int>(initial_amount));
		}
	}

	if (str != text_) {
		// Field properties changed -> update text_
		ui_field_.set_text(str);
		text_ = std::move(str);
	}

	// Immovable information
	if (Widelands::BaseImmovable* const imm = coords_.field->get_immovable()) {
		ui_immovable_.set_title(format("%s (%u)", imm->descr().name(), imm->serial()));
		ui_immovable_.set_enabled(true);
	} else {
		ui_immovable_.set_title("no immovable");
		ui_immovable_.set_enabled(false);
	}

	// Bobs information
	std::vector<Widelands::Bob*> bobs;
	map_.find_bobs(egbase, Widelands::Area<Widelands::FCoords>(coords_, 0), &bobs);

	// Do not clear the list. Instead parse all bobs and sync lists
	for (uint32_t idx = 0; idx < ui_bobs_.size(); idx++) {
		Widelands::MapObject* mo = ibase().egbase().objects().get_object(ui_bobs_[idx]);
		bool toremove = false;
		std::vector<Widelands::Bob*>::iterator removeIt;
		// Nested loop :(

		for (std::vector<Widelands::Bob*>::iterator bob_iter = bobs.begin(); bob_iter != bobs.end();
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
		ui_bobs_.remove(idx);
		idx--;  // reiter the same index
	}
	// Add remaining
	for (const Widelands::Bob* temp_bob : bobs) {
		ui_bobs_.add(
		   format("%s (%u)", temp_bob->descr().name(), temp_bob->serial()), temp_bob->serial());
	}
}

/*
===============
Open the debug window for the immovable on our position.
===============
*/
void FieldDebugWindow::open_immovable() {
	if (Widelands::BaseImmovable* const imm = coords_.field->get_immovable()) {
		show_mapobject_debug(ibase(), *imm);
	}
}

/*
===============
Open the bob debug window for the bob of the given index in the list
===============
*/
void FieldDebugWindow::open_bob(const uint32_t index) {
	if (index != UI::Listselect<intptr_t>::no_selection_index()) {
		if (Widelands::MapObject* const object =
		       ibase().egbase().objects().get_object(ui_bobs_.get_selected())) {
			show_mapobject_debug(ibase(), *object);
		}
	}
}

/*
===============
show_field_debug

Open a debug window for the given field.
===============
*/
void show_field_debug(InteractiveBase& parent, const Widelands::Coords& coords) {
	new FieldDebugWindow(parent, coords);
}
