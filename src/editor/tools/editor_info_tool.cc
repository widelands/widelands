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
		(&parent, 30, 30, 400, 200, _("Field Information").c_str());
	UI::Multiline_Textarea * const multiline_textarea =
		new UI::Multiline_Textarea
		(w, 0, 0, w->get_inner_w(), w->get_inner_h(), 0);

	Widelands::Field & f = map[center.node];

	std::string buf;
	char buf1[1024];

	sprintf(buf1, "%s\n", _("1) Field Infos").c_str());
	buf += buf1;
	sprintf
		(buf1,
		 " %s (%i/%i)\n",
		 _("Coordinates").c_str(),
		 center.node.x, center.node.y);
	buf += buf1;
	sprintf(buf1, " %s %i\n", _("Height").c_str(), f.get_height()); buf += buf1;
	buf += _(" Caps: ");
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
	buf += "\n";
	sprintf(buf1, " %s: %i\n", _("Owned by").c_str(), f.get_owned_by());
	buf += buf1;
	sprintf
		(buf1,
		 " %s: %s (TODO! more info)\n",
		 _("Has base immovable").c_str(), f.get_immovable() ? "Yes" : "No");
	buf+=buf1;
	sprintf
		(buf1,
		 " %s: %s (TODO: more informations)\n",
		 _("Has bobs").c_str(), f.get_first_bob() ? "Yes" : "No");
	buf += buf1;
	int32_t const res    = f.get_resources();
	int32_t const amount = f.get_resources_amount();
	if (res or amount)
		snprintf
			(buf1, sizeof(buf1),
			 " %s, %i %s '%s'\n",
			 _("Has resources: Yes").c_str(),
			 amount,
			 _("amount of").c_str(),
			 world.get_resource(res)->name().c_str());
	else
		snprintf
			(buf1, sizeof(buf1),
			 _(" Has resources: No\n").c_str());
	buf += buf1;

   sprintf
		(buf1,
		 " %s: %i\n", _("Start resources amount").c_str(),
		 f.get_starting_res_amount());
	buf += buf1;

	sprintf(buf1, _(" Roads: TODO!\n").c_str());
	buf += buf1;

	buf += "\n";
	sprintf(buf1, "%s\n", _("2) Terrain Info\n").c_str());
	buf += buf1;
	{
		Widelands::Field         const & tf  = map[center.triangle];
		Widelands::Terrain_Descr const & ter = world.terrain_descr
			(center.triangle.t == Widelands::TCoords<>::D ?
			 tf.terrain_d() : tf.terrain_r());
		snprintf
			(buf1, sizeof(buf1),
			 " %s: %s\n", _("Name").c_str(), ter.name().c_str());
		buf += buf1;
		snprintf
			(buf1, sizeof(buf1),
			 " %s: %i\n", _("Texture Number").c_str(), ter.get_texture());
		buf += buf1;
	}

	buf += "\n";
	sprintf(buf1, "%s\n", _("4) Map Info").c_str());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Name").c_str(), map.get_name());
	buf += buf1;
	sprintf
		(buf1,
		 " %s: %ix%i\n", _("Size").c_str(), map.get_width(), map.get_height());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Author").c_str(), map.get_author());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Descr").c_str(), map.get_description());
	buf += buf1;
	sprintf
		(buf1, " %s: %i\n", _("Number of Players").c_str(), map.get_nrplayers());
	buf += buf1;
	sprintf
		(buf1,
		 " %s\n",
		 _
		 (" TODO: more information (number of resources, number of terrains...)")
		 .c_str());
	buf += buf1;

	buf += "\n";
	sprintf(buf1, "%s\n", _("5) World Info").c_str());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Name").c_str(), world.get_name());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Author").c_str(), world.get_author());
	buf += buf1;
	sprintf(buf1, " %s: %s\n", _("Descr").c_str(), world.get_descr());
	buf += buf1;
	sprintf
		(buf1,
		 " %s\n",
		 _
		 (" TODO -- More information (Number of bobs/number of wares...)\n")
		 .c_str());
	buf += buf1;

	buf += "\n\n\n\n\n";

	multiline_textarea->set_text(buf.c_str());

	return 0;
}
