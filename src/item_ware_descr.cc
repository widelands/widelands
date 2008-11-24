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

#include "graphic.h"
#include "i18n.h"
#include "item_ware_descr.h"
#include "profile.h"

namespace Widelands {

/**
 * Loads a ware from conf stored in a given directory
 *
 * \param directory directory to read conffile from
 * \param prof
 */
void Item_Ware_Descr::parse(const char *directory, Profile *prof)
{
	char buffer[256];
	Section & global = prof->get_safe_section("global");

	m_descname = global.get_string("descname", name().c_str());
	m_helptext = global.get_string
		("help", _("Doh... someone forgot the help text!"));

	snprintf(buffer, sizeof(buffer), "%s_menu.png", name().c_str());
	snprintf
		(buffer, sizeof(buffer),
		 "%s/%s", directory, global.get_string("menu_pic", buffer));
	m_icon_fname = buffer;

	m_default_target_quantity =
		global.get_positive
			("default_target_quantity", std::numeric_limits<uint32_t>::max());
	add_animation("idle", g_anim.get(directory, prof->get_safe_section("idle")));
}


/**
 * Load all static graphics
 */
void Item_Ware_Descr::load_graphics()
{
	m_icon = g_gr->get_picture(PicMod_Game, m_icon_fname.c_str());
}


/**
 * Creates a new Item_Ware_Descr from data found in the given directory.
 *
 * \param name name of the Item_Ware_Descr to create
 * \param dir directory to read data from
 * \return newly created structure.
 * \throw all exceptions happened during process
 */
Item_Ware_Descr * Item_Ware_Descr::create_from_dir
	(char const * const name, char const * const dir)
{
	Item_Ware_Descr* descr = new Item_Ware_Descr(name);

	try {
		char fname[256];
		snprintf(fname, sizeof(fname), "%s/conf", dir);

		Profile prof(fname);
		descr->parse(dir, &prof);
	}
	catch (...) {
		delete descr;
		throw;
	}

	return descr;
}

};
