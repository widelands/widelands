/*
 * Copyright (C) 2005, 2008 by the Widelands Development Team
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

#ifndef WORKAREA_INFO_H
#define WORKAREA_INFO_H

#include <map>
#include <set>
#include <string>
#include <cstring>

//  This type is used to store information about workareas. It stores radii and
//  for each radius a set of strings. Each string contains a description of an
//  activity (or similar) that can be performed within the radius.
typedef std::map<uint32_t, std::set<std::string> > Workarea_Info;

#endif
