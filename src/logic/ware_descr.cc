/*
 * Copyright (C) 2002-2004, 2006-2008, 2011-2012 by the Widelands Development Team
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

#include <iostream> // TODO remove test output
#include <boost/lexical_cast.hpp>

#include "logic/ware_descr.h"
#include "logic/tribe.h"

#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "profile/profile.h"
#include "upcast.h"

namespace Widelands {

WareDescr::WareDescr
	(const Tribe_Descr & gtribe, char const * const _name,
	 char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s)
	:
	Map_Object_Descr(_name, _descname),
	m_tribe(gtribe),
	m_helptext(global_s.get_string("help", "")),
	m_icon_fname(directory + "/menu.png"),
	m_icon(g_gr ? g_gr->images().get("pics/but0.png") : nullptr) // because of dedicated
{
	m_default_target_quantity =
		global_s.get_positive("default_target_quantity", std::numeric_limits<uint32_t>::max());

	add_animation("idle", g_gr->animations().load(directory, prof.get_safe_section("idle")));

	m_preciousness =
		static_cast<uint8_t>(global_s.get_natural("preciousness", 0));
}


/**
 * Returns an index of buildings that can produce this ware
 */
std::set<Building_Index> & WareDescr::producers() {
	// find buildings that produce this ware
	std::cout << ("XXXX Test ware description for ") << name() << std::endl; // TODO remove test output

 // NOCOM(#gunchleoc2sirver): I can't fill this up in the constructor, because the building descriptions haven't been parsed in the tribe yet. lua_map.cc insists on a const though. I don't think that changing the parse order in tribe.cc is a stable solution. What now?
	Building_Index const nr_buildings = tribe().get_nrbuildings();
	std::cout << ("XXXXXX Test nr buildings ") << nr_buildings << std::endl; // TODO remove test output

	for (Building_Index i = 0; i < nr_buildings; ++i) {
		const Building_Descr & descr = *tribe().get_building_descr(i);
		std::cout << ("XXXXXX Test building descr ") << descr.name() << std::endl; // TODO remove test output

		if (upcast(ProductionSite_Descr const, de, &descr)) {
			std::cout << ("XXXXXX Test productionsite ") << descr.name() << std::endl; // TODO remove test output

			for (auto ware_index : de->output_ware_types()) {
 // NOCOM(#gunchleoc2sirver): using lexical_cast here, because I don't know how to compare a char*. Can this be done more elegantly?
				if (boost::lexical_cast<std::string>(name()).compare(boost::lexical_cast<std::string>(tribe().get_ware_descr(ware_index)->name()))) {
					assert(not m_producers.count(i));
					m_producers.insert(i);
					std::cout << ("XXXXXX Test producer ") << name() << std::endl; // TODO remove test output
				}
			}
		}
	}
	return m_producers;
}


/**
 * Load all static graphics
 */
void WareDescr::load_graphics()
{
	m_icon = g_gr->images().get(m_icon_fname);
}

}
