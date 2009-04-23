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

#include "item_ware_descr.h"

#include "graphic.h"
#include "i18n.h"
#include "profile.h"

namespace Widelands {

Item_Ware_Descr::Item_Ware_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s)
	:
	Map_Object_Descr(_name, _descname),
	m_helptext      (global_s.get_string("help", "")),
	m_icon_fname(directory + "/menu.png"),
	m_icon(0)
{
	m_default_target_quantity =
		global_s.get_positive
			("default_target_quantity", std::numeric_limits<uint32_t>::max());
	add_animation("idle", g_anim.get(directory, prof.get_safe_section("idle")));

	m_preciousness =
		static_cast<uint8_t>(global_s.get_natural("preciousness", 0));
}


/**
 * Load all static graphics
 */
void Item_Ware_Descr::load_graphics()
{
	m_icon = g_gr->get_picture(PicMod_Game, m_icon_fname.c_str());
}

}
