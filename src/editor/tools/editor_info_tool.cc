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

#include "editor/editorinteractive.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/world.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

/// Show a window with information about the pointed at node and triangle.
int32_t Editor_Info_Tool::handle_click_impl
	(Widelands::Map            &         map,
	Widelands::Node_and_Triangle<> const center,
	Editor_Interactive         &         parent,
	Editor_Action_Args         &         /* args */)
{
	const Widelands::World & world = map.world();
	UI::Window * const w =
	    new UI::Window
	(&parent, "field_information", 30, 30, 400, 200,
	 _("Field Information"));
	UI::Multiline_Textarea * const multiline_textarea =
	    new UI::Multiline_Textarea
	(w, 0, 0, w->get_inner_w(), w->get_inner_h());

	Widelands::Field & f = map[center.node];

	// *** Node info
	char buf1[1024];
	std::string buf = _("Node:");
	buf += "\n";
	buf += std::string("• ") + (boost::format(_("Coordinates: (%1$i, %2$i)"))
					 % center.node.x % center.node.y).str() + "\n";

	buf += std::string("• ");
	buf += buf1;
	buf += std::string("\n");

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

	buf += std::string("• ");
	snprintf(buf1, sizeof(buf1), _("Owned by %i"), f.get_owned_by());
	buf += std::string(buf1) + "\n";

	temp = f.get_immovable() ? _("Has base immovable") : _("No base immovable");
	buf += std::string("• ") + temp + "\n";

	temp = f.get_first_bob() ? _("Has bobs") : _("No bobs");
	buf += std::string("• ") + temp + "\n";

	// *** Terrain info
	buf += std::string("\n") + _("Terrain:") + "\n";

	const Widelands::Field         & tf  = map[center.triangle];
	const Widelands::Terrain_Descr & ter = world.terrain_descr
	                                       (center.triangle.t == Widelands::TCoords<>::D ?
	                                        tf.terrain_d() : tf.terrain_r());

	buf += std::string("• ") + (boost::format(_("Name: %s")) % ter.descname()).str() + "\n";
	buf += std::string("• ") + (boost::format(_("Texture Number: %i")) % ter.get_texture()).str() + "\n";

	// *** Resources info
	buf += std::string("\n") + _("Resources:") + "\n";

	Widelands::Resource_Index ridx = f.get_resources();
	int ramount = f.get_resources_amount();

	if (ramount > 0) {
		buf += std::string("• ") + (boost::format(
				_("Resource name: %s")) % world.get_resource(ridx)->name().c_str()
			).str() + "\n";
		buf += std::string("• ") + (boost::format(_("Resource amount: %i")) % ramount).str() + "\n";
	}
	else
	{
		buf += std::string("• ") + std::string(_("No resources")) + "\n";
	}

	// *** Map info
	buf += std::string("\n") + _("Map:") + "\n";
	buf += std::string("• ") + (boost::format(_("Name: %s")) % map.get_name()).str() + "\n";
	buf += std::string("• ") + (boost::format(_("Size: %1$ix%2$i"))
					 % map.get_width() % map.get_height()).str() + "\n";

	buf += std::string("• ");
	snprintf(buf1, sizeof(buf1), _("Players: %i"), map.get_nrplayers());

	buf += std::string(buf1) + "\n";
	buf += std::string("• ") + (boost::format(_("Author: %s")) % map.get_author()).str() + "\n";
	buf += std::string("• ") + (boost::format(_("Descr: %s")) % map.get_description()).str() + "\n";

	// *** World info
	buf += std::string("\n") + _("World:") + "\n";
	buf += std::string("• ") + (boost::format(_("Name: %s")) % world.get_name()).str() + "\n";
	buf += std::string("• ") + (boost::format(_("Author: %s")) % world.get_author()).str() + "\n";
	buf += std::string("• ") + (boost::format(_("Descr: %s")) % world.get_descr()).str() + "\n";

	multiline_textarea->set_text(buf.c_str());

	return 0;
}
