/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "editor/tools/info_tool.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

/// Show a window with information about the pointed at node and triangle.
int32_t EditorInfoTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                          EditorInteractive& parent,
                                          EditorActionArgs* /* args */,
                                          Widelands::Map* map) {

	constexpr UI::FontStyle font_style = UI::FontStyle::kWuiInfoPanelParagraph;

	parent.stop_painting();

	UI::Window* const w =
	   new UI::Window(&parent, "field_information", 30, 30, 400, 200, _("Field Information"));
	UI::MultilineTextarea* const multiline_textarea =
	   new UI::MultilineTextarea(w, 0, 0, w->get_inner_w(), w->get_inner_h(), UI::PanelStyle::kWui);

	Widelands::Field& f = (*map)[center.node];

	// *** Node info
	std::string buf = as_heading(_("Node"), UI::PanelStyle::kWui, true);
	buf += as_listitem(
	   (boost::format(_("Coordinates: (%1$i, %2$i)")) % center.node.x % center.node.y).str(),
	   font_style);

	std::vector<std::string> caps_strings;
	Widelands::NodeCaps const caps = f.nodecaps();
	switch (caps & Widelands::BUILDCAPS_SIZEMASK) {

	case Widelands::BUILDCAPS_SMALL: {
		/** TRANSLATORS: Editor terrain property: small building plot */
		caps_strings.push_back(_("small"));
	} break;

	case Widelands::BUILDCAPS_MEDIUM: {
		/** TRANSLATORS: Editor terrain property: medium building plot */
		caps_strings.push_back(_("medium"));
	} break;
	case Widelands::BUILDCAPS_BIG: {
		/** TRANSLATORS: Editor terrain property: big building plot */
		caps_strings.push_back(_("big"));
	} break;
	default:
		break;
	}

	if (caps & Widelands::BUILDCAPS_FLAG) {
		/** TRANSLATORS: Editor terrain property: space for a flag */
		caps_strings.push_back(_("flag"));
	}
	if (caps & Widelands::BUILDCAPS_MINE) {
		/** TRANSLATORS: Editor terrain property: mine building plot */
		caps_strings.push_back(_("mine"));
	}
	if (caps & Widelands::BUILDCAPS_PORT) {
		/** TRANSLATORS: Editor terrain property: port space */
		caps_strings.push_back(_("port"));
	}
	if (caps & Widelands::MOVECAPS_WALK) {
		/** TRANSLATORS: Editor terrain property: units can walk on this terrain */
		caps_strings.push_back(_("walkable"));
	}
	if (caps & Widelands::MOVECAPS_SWIM) {
		/** TRANSLATORS: Editor terrain property: units can swim on this terrain (fish, ships) */
		caps_strings.push_back(_("swimmable"));
	}

	buf += as_listitem((boost::format(_("Caps: %s")) %
	                    i18n::localize_list(caps_strings, i18n::ConcatenateWith::COMMA))
	                      .str(),
	                   font_style);

	if (f.get_owned_by() > 0) {
		buf += as_listitem(
		   (boost::format(_("Owned by: Player %u")) % static_cast<unsigned int>(f.get_owned_by()))
		      .str(),
		   font_style);
	} else {
		buf += as_listitem(_("Owned by: —"), font_style);
	}

	// *** Terrain info
	buf += as_heading(_("Terrain"), UI::PanelStyle::kWui);

	const Widelands::Field& tf = (*map)[center.triangle.node];
	const Widelands::TerrainDescription& ter = parent.egbase().world().terrain_descr(
	   center.triangle.t == Widelands::TriangleIndex::D ? tf.terrain_d() : tf.terrain_r());

	buf += as_listitem(
	   (boost::format(pgettext("terrain_name", "Name: %s")) % ter.descname()).str(), font_style);

	std::vector<std::string> terrain_is_strings;
	for (const Widelands::TerrainDescription::Type& terrain_type : ter.get_types()) {
		terrain_is_strings.push_back(terrain_type.descname);
	}

	buf += as_listitem(
	   /** TRANSLATORS: "Is" is a list of terrain properties, e.g. "arable, unreachable and
	    * unwalkable". You can also translate this as "Category: %s" or "Property: %s" */
	   (boost::format(_("Is: %s")) %
	    i18n::localize_list(terrain_is_strings, i18n::ConcatenateWith::AMPERSAND))
	      .str(),
	   font_style);

	// *** Map Object info
	const Widelands::BaseImmovable* immovable = f.get_immovable();
	Widelands::Bob* bob = f.get_first_bob();
	if (immovable || bob) {
		/** TRANSLATORS: Heading for immovables and animals in editor info tool */
		buf += as_heading(_("Objects"), UI::PanelStyle::kWui);
		if (immovable) {
			buf += as_listitem(
			   (boost::format(_("Immovable: %s")) % immovable->descr().descname()).str(), font_style);
		}

		if (bob) {
			// Collect bobs
			std::vector<std::string> critternames;
			std::vector<std::string> shipnames;
			std::vector<std::string> workernames;
			do {
				switch (bob->descr().type()) {
				case (Widelands::MapObjectType::CRITTER):
					critternames.push_back(bob->descr().descname());
					break;
				case (Widelands::MapObjectType::SHIP): {
					if (upcast(Widelands::Ship, ship, bob)) {
						shipnames.push_back(ship->get_shipname());
					}
				} break;
				case (Widelands::MapObjectType::WORKER):
				case (Widelands::MapObjectType::CARRIER):
				case (Widelands::MapObjectType::SOLDIER):
					workernames.push_back(bob->descr().descname());
					break;
				default:
					break;
				}
			} while ((bob = bob->get_next_bob()));

			// Add bobs
			if (!critternames.empty()) {
				buf += as_listitem((boost::format(_("Animals: %s")) %
				                    i18n::localize_list(critternames, i18n::ConcatenateWith::COMMA))
				                      .str(),
				                   font_style);
			}
			if (!workernames.empty()) {
				buf += as_listitem((boost::format(_("Workers: %s")) %
				                    i18n::localize_list(workernames, i18n::ConcatenateWith::COMMA))
				                      .str(),
				                   font_style);
			}
			if (!shipnames.empty()) {
				buf += as_listitem((boost::format(_("Ships: %s")) %
				                    i18n::localize_list(shipnames, i18n::ConcatenateWith::COMMA))
				                      .str(),
				                   font_style);
			}
		}
	}

	// *** Resources info
	Widelands::ResourceAmount ramount = f.get_resources_amount();
	if (ramount > 0) {
		buf += as_heading(_("Resources"), UI::PanelStyle::kWui);
		buf += as_listitem(
		   (boost::format(pgettext("resources", "%1%x %2%")) % static_cast<unsigned int>(ramount) %
		    parent.egbase().world().get_resource(f.get_resources())->descname())
		      .str(),
		   font_style);
	}

	// *** Map info
	buf += as_heading(_("Map"), UI::PanelStyle::kWui);
	buf += as_listitem(
	   (boost::format(pgettext("map_name", "Name: %s")) % richtext_escape(map->get_name())).str(),
	   font_style);
	buf += as_listitem(
	   (boost::format(_("Size: %1% x %2%")) % map->get_width() % map->get_height()).str(),
	   font_style);

	if (map->get_nrplayers() > 0) {
		buf += as_listitem(
		   (boost::format(_("Players: %u")) % static_cast<unsigned int>(map->get_nrplayers())).str(),
		   font_style);
	} else {
		buf += as_listitem(_("Players: –"), font_style);
	}

	buf += as_listitem(
	   (boost::format(_("Author: %s")) % richtext_escape(map->get_author())).str(), font_style);
	buf += as_listitem(
	   (boost::format(_("Description: %s")) % richtext_escape(map->get_description())).str(),
	   font_style);

	multiline_textarea->set_text(as_richtext(buf));

	return 0;
}
