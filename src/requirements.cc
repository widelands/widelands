/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "requirements.h"

#include "instances.h"


namespace Widelands {

Requirements::Requirements ()
{
}

void Requirements::clear()
{
	r.clear();
}

void Requirements::set(tAttribute at, int32_t min, int32_t max)
{
	for (std::vector<MinMax>::iterator it = r.begin(); it != r.end(); ++it) {
		if (it->at == at) {
			it->min = min;
			it->max = max;
			return;
		}
	}

	r.push_back(MinMax(at, min, max));
}

bool Requirements::check(Map_Object* obj) const
{
	for (std::vector<MinMax>::const_iterator it = r.begin(); it != r.end(); ++it) {
		int32_t value = obj->get_tattribute(it->at);

		if (value > it->max || value < it->min)
			return false;
	}

	return true;
}


#define REQUIREMENTS_VERSION 2

/**
 * Read this requirement from a file
 */
void Requirements::Read
(FileRead * fr, Editor_Game_Base *, Map_Map_Object_Loader *)
{
	try {
		const uint16_t packet_version = fr->Unsigned16();
		if (packet_version == REQUIREMENTS_VERSION) {
			for (;;) {
				uint32_t at = fr->Unsigned32();

				if (at == 0xffffffff)
					break;

				int32_t min = fr->Signed32();
				int32_t max = fr->Signed32();

				set(static_cast<tAttribute>(at), min, max);
			}
		} else if (packet_version == 1) {
			int32_t min, max;

			// HitPoints Levels
			min = fr->Unsigned8();
			max = fr->Unsigned8();
			set(atrHP, min, max);

			// Attack Levels
			min = fr->Unsigned8();
			max = fr->Unsigned8();
			set(atrAttack, min, max);

			// Defense levels
			min = fr->Unsigned8();
			max = fr->Unsigned8();
			set(atrDefense, min, max);

			// Evade
			min = fr->Unsigned8();
			max = fr->Unsigned8();
			set(atrEvade, min, max);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("requirements: %s", e.what());
	}
}

void Requirements::Write
(FileWrite * fw, Editor_Game_Base *, Map_Map_Object_Saver *)
{
	fw->Unsigned16(REQUIREMENTS_VERSION);

	for (std::vector<MinMax>::iterator it = r.begin(); it != r.end(); ++it) {
		assert(static_cast<uint32_t>(it->at) != 0xffffffff);
		fw->Unsigned32(it->at);
		fw->Signed32(it->min);
		fw->Signed32(it->max);
	}

	fw->Unsigned32(0xffffffff);
}

}

