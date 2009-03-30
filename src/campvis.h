/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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

#ifndef CAMPVIS_H
#define CAMPVIS_H

#include <stdint.h>
#include <cstring>
#include <string>


struct Campaign_visibility_save {
	std::string get_path();
	void set_campaign_visibility(std::string const &, bool);
	void set_map_visibility     (std::string const &, bool);

private:
	void make_campvis  (std::string const &);
	void update_campvis(std::string const &);
};


#endif
