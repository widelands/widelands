/*
* Copyright (C) 2010 The Widelands Development Team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __WIDELANDS_MAP_H__
#define __WIDELANDS_MAP_H__

#include <string>
#include "protocol.h"

class WidelandsMap 
{
	public:
		WidelandsMap():
			m_name(""),
			m_w(0),
			m_h(0)
			{}
		unsigned int w() {return m_w; } 
		unsigned int h() {return m_w; }
		std::string name() {return m_name; }
		WLGGZGameType gametype() { return m_gametype; }
		
		void set_name(std::string name) { m_name = name; }
		void set_size(int w, int h) { m_w = w; m_h = h; }
		void set_gametype(WLGGZGameType type) { m_gametype=type; }

	private:
		std::string m_name;
		unsigned int m_w, m_h;
		WLGGZGameType m_gametype;
};

#endif //__WIDELANDS_MAP_H__