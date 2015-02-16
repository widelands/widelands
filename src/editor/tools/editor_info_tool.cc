/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "editor/tools/editor_info_tool.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "logic/map.h"
#include "logic/world/editor_category.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

/// Show a window with information about the pointed at node and triangle.
int32_t EditorInfoTool::handle_click_impl(Widelands::Map& map,
					    const Widelands::World& world,
					    Widelands::NodeAndTriangle<> center,
					    EditorInteractive& parent,
					    EditorActionArgs& /* args */) {
	UI::Window * const w =
	    new UI::Window
	(&parent, "field_information", 30, 30, 400, 200,
	 _("Field Information"));
	UI::MultilineTextarea * const multiline_textarea =
	    new UI::MultilineTextarea
	(w, 0, 0, w->get_inner_w(), w->get_inner_h());

	Widelands::Field & f = map[center.node];

	// *** Node info
	std::string buf = _("Node:");
	buf += "\n";
	buf += "• " + (boost::format(_("Coordinates: (%1$i, %2$i)"))
					 % center.node.x % center.node.y).str() + "\n";

	std::string temp = "";
	Widelands::NodeCaps const caps = f.nodecaps();
	switch (caps & Widelands::BUILDCAPS_SIZEMASK) {
		/** TRANSLATORS: This is part of a list, e.g. Caps: medium flag walk */
		case Widelands::BUILDCAPS_SMALL:  temp += _(" small");  break;
		/** TRANSLATORS: This is part of a list, e.g. Caps: medium flag walk */
		case Widelands::BUILDCAPS_MEDIUM: temp += _(" medium"); break;
		/** TRANSLATORS: This is part of a list, e.g. Caps: medium flag walk */
		case Widelands::BUILDCAPS_BIG:    temp += _(" big");    break;
		default: break;
	};
	/** TRANSLATORS: This is part of a list, e.g. Caps: medium flag walk */
	if (caps & Widelands::BUILDCAPS_FLAG) temp += _(" flag");
	/** TRANSLATORS: This is part of a list, e.g. Caps: flag mine walk */
	if (caps & Widelands::BUILDCAPS_MINE) temp += _(" mine");
	/** TRANSLATORS: This is part of a list, e.g. Caps: big flag port walk */
	if (caps & Widelands::BUILDCAPS_PORT) temp += _(" port");
	/** TRANSLATORS: This is part of a list, e.g. Caps: medium flag walk */
	if (caps & Widelands::MOVECAPS_WALK)  temp += _(" walk");
	/** TRANSLATORS: This is part of a list, e.g. Caps: swim */
	if (caps & Widelands::MOVECAPS_SWIM)  temp += _(" swim");

	buf += std::string("• ") + (boost::format(_("Caps:%s")) % temp).str() + "\n";

	if (f.get_owned_by() > 0) {
		buf += std::string("• ") +
				 (boost::format(_("Owned by: Player %u"))
				  % static_cast<unsigned int>(f.get_owned_by())).str() + "\n";
	} else {
		buf += std::string("• ") + _("Owned by: —") + "\n";
	}

	temp = f.get_immovable() ? _("Has base immovable") : _("No base immovable");
	buf += "• " + temp + "\n";

	temp = f.get_first_bob() ? _("Has animals") : _("No animals");
	buf += "• " + temp + "\n";

	// *** Terrain info
	buf += std::string("\n") + _("Terrain:") + "\n";

	const Widelands::Field         & tf  = map[center.triangle];
	const Widelands::TerrainDescription& ter = world.terrain_descr(
	   center.triangle.t == Widelands::TCoords<>::D ? tf.terrain_d() : tf.terrain_r());

	buf += "• " + (boost::format(_("Name: %s")) % ter.descname()).str() + "\n";

	Widelands::TerrainDescription::Type terrain_is = ter.get_is();
	std::vector<std::string> terrain_is_strings;

	if (terrain_is == Widelands::TerrainDescription::Type::kGreen) {
		terrain_is_strings.push_back(_("arable"));
	}
	if (terrain_is & Widelands::TerrainDescription::Type::kDry) {
		terrain_is_strings.push_back(_("treeless"));
	}
	if (terrain_is & Widelands::TerrainDescription::Type::kWater) {
		terrain_is_strings.push_back(_("aquatic"));
	}
	if (terrain_is & Widelands::TerrainDescription::Type::kDead) {
		terrain_is_strings.push_back(_("dead"));
	}
	if (terrain_is & Widelands::TerrainDescription::Type::kMountain) {
		terrain_is_strings.push_back(_("mountainous"));
	}
	if (terrain_is & Widelands::TerrainDescription::Type::kUnpassable) {
		terrain_is_strings.push_back(_("unpassable"));
	}
	buf += "• " + (boost::format(_("Category: %s"))
						% i18n::localize_list(terrain_is_strings, i18n::ConcatenateWith::AMPERSAND)).str() + "\n";
	buf += "• " + (boost::format(_("Editor Category: %s")) % ter.editor_category().descname()).str() + "\n";

	// *** Resources info
	buf += std::string("\n") + _("Resources:") + "\n";

	Widelands::ResourceIndex ridx = f.get_resources();
	int ramount = f.get_resources_amount();

	if (ramount > 0) {
		buf += "• " + (boost::format(
				_("Resource name: %s")) % world.get_resource(ridx)->name().c_str()
			).str() + "\n";
		buf += "• " + (boost::format(_("Resource amount: %i")) % ramount).str() + "\n";
	}
	else {
		buf += "• " + std::string(_("No resources")) + "\n";
	}

	// *** Map info
	buf += std::string("\n") + _("Map:") + "\n";
	buf += "• " + (boost::format(_("Name: %s")) % map.get_name().c_str()).str() + "\n";
	buf += "• " + (boost::format(_("Size: %1$ix%2$i"))
					 % map.get_width() % map.get_height()).str() + "\n";

	if (map.get_nrplayers() > 0) {
		buf += "• " +
		       (boost::format(_("Players: %d")) % static_cast<int>(map.get_nrplayers())).str() + "\n";
	}
	else {
		buf += "• " + std::string(_("Players: -")) + "\n";
	}

	buf += "• " + (boost::format(_("Author: %s")) % map.get_author()).str() + "\n";
	buf += "• " + (boost::format(_("Descr: %s")) % map.get_description().c_str()).str() + "\n";

	multiline_textarea->set_text(buf.c_str());

	return 0;
}
