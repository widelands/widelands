/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <cstdio>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "logic/map.h"
#include "logic/map_objects/world/editor_category.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

/// Show a window with information about the pointed at node and triangle.
int32_t EditorInfoTool::handle_click_impl(const Widelands::World& world,
                                          const Widelands::NodeAndTriangle<>& center,
                                          EditorInteractive& parent,
                                          EditorActionArgs* /* args */,
                                          Widelands::Map* map) {
	parent.stop_painting();

	UI::Window* const w =
	   new UI::Window(&parent, "field_information", 30, 30, 400, 200, _("Field Information"));
	UI::MultilineTextarea* const multiline_textarea =
	   new UI::MultilineTextarea(w, 0, 0, w->get_inner_w(), w->get_inner_h(), UI::Panel::Style::kWui);

	Widelands::Field& f = (*map)[center.node];

	// *** Node info
	std::string buf = _("Node:");
	buf += "\n";
	buf += "• " +
	       (boost::format(_("Coordinates: (%1$i, %2$i)")) % center.node.x % center.node.y).str() +
	       "\n";

	std::vector<std::string> caps_strings;
	Widelands::NodeCaps const caps = f.nodecaps();
	switch (caps & Widelands::BUILDCAPS_SIZEMASK) {
	/** TRANSLATORS: Editor terrain property: small building plot */
	case Widelands::BUILDCAPS_SMALL:
		caps_strings.push_back(_("small"));
		break;
	/** TRANSLATORS: Editor terrain property: medium building plot */
	case Widelands::BUILDCAPS_MEDIUM:
		caps_strings.push_back(_("medium"));
		break;
	/** TRANSLATORS: Editor terrain property: big building plot */
	case Widelands::BUILDCAPS_BIG:
		caps_strings.push_back(_("big"));
		break;
	default:
		break;
	};
	/** TRANSLATORS: Editor terrain property: space for a flag */
	if (caps & Widelands::BUILDCAPS_FLAG)
		caps_strings.push_back(_("flag"));
	/** TRANSLATORS: Editor terrain property: mine building plot */
	if (caps & Widelands::BUILDCAPS_MINE)
		caps_strings.push_back(_("mine"));
	/** TRANSLATORS: Editor terrain property: port space */
	if (caps & Widelands::BUILDCAPS_PORT)
		caps_strings.push_back(_("port"));
	/** TRANSLATORS: Editor terrain property: units can walk on this terrain */
	if (caps & Widelands::MOVECAPS_WALK)
		caps_strings.push_back(_("walkable"));
	/** TRANSLATORS: Editor terrain property: units can swim on this terrain (fish, ships) */
	if (caps & Widelands::MOVECAPS_SWIM)
		caps_strings.push_back(_("swimmable"));

	buf += std::string("• ") +
	       (boost::format(_("Caps: %s")) %
	        i18n::localize_list(caps_strings, i18n::ConcatenateWith::COMMA))
	          .str() +
	       "\n";

	if (f.get_owned_by() > 0) {
		buf += std::string("• ") +
		       (boost::format(_("Owned by: Player %u")) % static_cast<unsigned int>(f.get_owned_by()))
		          .str() +
		       "\n";
	} else {
		buf += std::string("• ") + _("Owned by: —") + "\n";
	}

	std::string temp = "";
	temp = f.get_immovable() ? _("Has immovable") : _("No immovable");
	buf += "• " + temp + "\n";

	temp = f.get_first_bob() ? _("Has animals") : _("No animals");
	buf += "• " + temp + "\n";

	// *** Terrain info
	buf += std::string("\n") + _("Terrain:") + "\n";

	const Widelands::Field& tf = (*map)[center.triangle];
	const Widelands::TerrainDescription& ter = world.terrain_descr(
	   center.triangle.t == Widelands::TCoords<>::D ? tf.terrain_d() : tf.terrain_r());

	buf +=
	   "• " + (boost::format(pgettext("terrain_name", "Name: %s")) % ter.descname()).str() + "\n";

	std::vector<std::string> terrain_is_strings;
	for (const Widelands::TerrainDescription::Type& terrain_type : ter.get_types()) {
		terrain_is_strings.push_back(terrain_type.descname);
	}
	/** TRANSLATORS: "Is" is a list of terrain properties, e.g. "arable", "unreachable and
	 * unwalkable" */
	/** TRANSLATORS: You can also translate this as "Category: %s" or "Property: %s" */
	buf += "• " +
	       (boost::format(_("Is: %s")) %
	        i18n::localize_list(terrain_is_strings, i18n::ConcatenateWith::AMPERSAND))
	          .str() +
	       "\n";
	buf += "• " +
	       (boost::format(_("Editor Category: %s")) % ter.editor_category()->descname()).str() +
	       "\n";

	// *** Resources info
	buf += std::string("\n") + _("Resources:") + "\n";

	Widelands::DescriptionIndex ridx = f.get_resources();
	Widelands::ResourceAmount ramount = f.get_resources_amount();

	if (ramount > 0) {
		buf +=
		   "• " +
		   (boost::format(_("Resource name: %s")) % world.get_resource(ridx)->name().c_str()).str() +
		   "\n";
		buf += "• " +
		       (boost::format(_("Resource amount: %i")) % static_cast<unsigned int>(ramount)).str() +
		       "\n";
	} else {
		buf += "• " + std::string(_("No resources")) + "\n";
	}

	// *** Map info
	buf += std::string("\n") + _("Map:") + "\n";
	buf += "• " + (boost::format(pgettext("map_name", "Name: %s")) % map->get_name().c_str()).str() +
	       "\n";
	buf += "• " +
	       (boost::format(_("Size: %1$ix%2$i")) % map->get_width() % map->get_height()).str() + "\n";

	if (map->get_nrplayers() > 0) {
		buf +=
		   "• " +
		   (boost::format(_("Players: %u")) % static_cast<unsigned int>(map->get_nrplayers())).str() +
		   "\n";
	} else {
		buf += "• " + std::string(_("Players: –")) + "\n";
	}

	buf += "• " + (boost::format(_("Author: %s")) % map->get_author()).str() + "\n";
	buf += "• " + (boost::format(_("Descr: %s")) % map->get_description().c_str()).str() + "\n";

	multiline_textarea->set_text(buf.c_str());

	return 0;
}
