/*
 * Copyright (C) 2002, 2010-2010 by the Widelands Development Team
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

#include "logic/military_data.h"

#include <string>
#include <vector>

#include "container_iterate.h"
#include "helper.h"
#include "i18n.h"
#include "logic/game_data_error.h"

namespace Widelands {

/// Dummy initialization
Military_Data::Military_Data() :
	m_min_retreat(0),
	m_max_retreat(100),
	m_retreat(50)
{}

/***
 * parse
 * \param s A section to parse
 */
void Military_Data::parse(Section & s)
{

	try {
		const char * const interval = s.get_safe_string("retreat_interval");
		std::vector<std::string> list(split_string(interval, "-"));
		if (list.size() != 2)
			throw game_data_error
				("expected %s but found \"%s\"", "\"min-max\"", interval);
		container_iterate(std::vector<std::string>, list, i)
			remove_spaces(*i.current);
		char * endp;
		m_min_retreat = strtol(list[0].c_str(), &endp, 0);
		if (*endp or m_min_retreat > 100)
			throw game_data_error
				("expected %s but found \"%s\"",
				 "positive integer <= 100", list[0].c_str());
		m_max_retreat = strtol(list[1].c_str(), &endp, 0);
		if (*endp or m_max_retreat > 100 or m_max_retreat < m_min_retreat)
			throw game_data_error
				("expected positive integer >= %u <= 100 but found \"%s\"",
				 m_min_retreat, list[1].c_str());

		// That is default value for retreat
		m_retreat = m_max_retreat;

		// If retreat is found, get his value
		if (s.has_val("retreat"))
			m_retreat = s.get_safe_positive("retreat");

		if (m_retreat < m_min_retreat or m_retreat > m_max_retreat)
			throw game_data_error
				("expected positive integer >= %u <= %u but found \"%u\"",
				 m_min_retreat,
				 m_max_retreat,
				 m_retreat);
	} catch (const _wexception & e) {
		throw game_data_error ("retreat: %s", e.what());
	}

}

}

