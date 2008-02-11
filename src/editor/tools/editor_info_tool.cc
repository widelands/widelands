/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "editor_info_tool.h"

#include "editorinteractive.h"
#include "i18n.h"
#include "map.h"
#include "world.h"

#include "ui_multilinetextarea.h"
#include "ui_window.h"

#include <stdio.h>


/*
===========
Editor_Info_Tool::handle_click()

show a simple info dialog with infos about this field
===========
*/
int32_t Editor_Info_Tool::handle_click_impl
(Widelands::Map                     & map,
 Widelands::Node_and_Triangle<> const center,
 Editor_Interactive                 & parent)
{
	Widelands::World const & world = map.world();
	UI::Window * w =
		new UI::Window
		(&parent, 30, 30, 400, 200, _("Field Information"));
	UI::Multiline_Textarea * const multiline_textarea =
		new UI::Multiline_Textarea
		(w, 0, 0, w->get_inner_w(), w->get_inner_h(), 0);

	Widelands::Field & f = map[center.node];

	std::string buf;
	char buf1[1024];

	snprintf
		(buf1, sizeof(buf1),
		 _("1) Node infos\n Coordinates: (%i, %i)\n Height: %u\n Caps: "),
		 center.node.x, center.node.y, f.get_height());
	buf += buf1;
	{
		Widelands::FieldCaps const caps = f.get_caps();
		switch (caps & Widelands::BUILDCAPS_SIZEMASK) {
		case Widelands::BUILDCAPS_SMALL:  buf += _("small");  break;
		case Widelands::BUILDCAPS_MEDIUM: buf += _("medium"); break;
		case Widelands::BUILDCAPS_BIG:    buf += _("big");    break;
		};
		if (caps & Widelands::BUILDCAPS_FLAG) buf += _(" flag");
		if (caps & Widelands::BUILDCAPS_MINE) buf += _(" mine");
		if (caps & Widelands::BUILDCAPS_PORT) buf += _(" port");
		if (caps & Widelands::MOVECAPS_WALK)  buf += _(" walk");
		if (caps & Widelands::MOVECAPS_SWIM)  buf += _(" swim");
	}
	snprintf
		(buf1, sizeof(buf1),
		 _("\n Owned by %i\n Has base immovable: %s\n Has bobs: %s\n"),
		 f.get_owned_by(),
		 f.get_immovable() ? _("Yes") : _("No"),
		 f.get_first_bob() ? _("Yes") : _("No"));
	buf += buf1;

	buf += _("2) Terrain Info\n Name: ");
	{
		Widelands::Field         const & tf  = map[center.triangle];
		Widelands::Terrain_Descr const & ter = world.terrain_descr
			(center.triangle.t == Widelands::TCoords<>::D ?
			 tf.terrain_d() : tf.terrain_r());
		buf += ter.name();
		snprintf
			(buf1, sizeof(buf1), _("\n Texture Number: %i\n"), ter.get_texture());
		buf += buf1;
	}

	buf += _("4) Map Info\n Name: ");
	buf += map.get_name();
	snprintf
		(buf1, sizeof(buf1),
		 _("Size: %ix%i\n Author: "), map.get_width(), map.get_height());
	buf += buf1;
	buf += map.get_author     ();
	buf += _("\n Descr: ");
	buf += map.get_description();
	snprintf
		(buf1, sizeof(buf1),
		 _("\n Number of Players: %i\n"), map.get_nrplayers());
	buf += buf1;

	buf += _("5) World Info\n Name: ");
	buf += world.get_name  ();
	buf += _("\n Author: ");
	buf += world.get_author();
	buf += _("\n Descr: ");
	buf += world.get_descr ();

	multiline_textarea->set_text(buf.c_str());

	return 0;
}
