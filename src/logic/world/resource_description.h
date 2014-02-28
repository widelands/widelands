/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef RESOURCE_DESCRIPTION_H
#define RESOURCE_DESCRIPTION_H

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

#include "logic/widelands.h"

class Section;

namespace Widelands {

class ResourceDescription : boost::noncopyable {
public:
	typedef Resource_Index Index;
	ResourceDescription();

	// Returns the in engine name of this resource.
	const std::string & name     () const;

	// Returns the name of this resource for users. Usually translated.
	const std::string & descname() const;

	// Returns if this resource is detectable by a geologist.
	bool is_detectable() const;

	// Returns the maximum amount that can be in a field for this resource.
	int32_t get_max_amount() const;

	// Returns the path to the image that should be used in the editor to
	// represent an 'amount' of this resource.
	const std::string & get_editor_pic(uint32_t amount) const;

	// Parses the description of this resource.
	void parse(Section &, const std::string &);

private:
	struct Editor_Pic {
		std::string picname;
		int32_t     upperlimit;
	};
	bool                    is_detectable_;
	int32_t                 max_amount_;
	std::string             name_;
	std::string             descname_;
	std::vector<Editor_Pic> editor_pics_;
};

}  // namespace Widelands


#endif /* end of include guard: RESOURCE_DESCRIPTION_H */
