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

#include "logic/world/resource_description.h"

#include "helper.h"
#include "log.h"
#include "logic/game_data_error.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"

namespace Widelands {

ResourceDescription::ResourceDescription(const LuaTable& table)
   : name_(table.get_string("name")),
     descname_(table.get_string("descname")),
     detectable_(table.get_bool("detectable")),
     max_amount_(table.get_int("max_amount")) {
	std::unique_ptr<LuaTable> st = table.get_table("editor_pictures");
	const std::vector<int> keys = st->keys<int>();
	for (int upper_limit : keys) {
		ResourceDescription::EditorPicture editor_picture = {st->get_string(upper_limit), upper_limit};
		editor_pictures_.push_back(editor_picture);
	}
}

const std::string & ResourceDescription::get_editor_pic
	(uint32_t const amount) const
{
	uint32_t bestmatch = 0;

	assert(editor_pictures_.size());

	for (uint32_t i = 1; i < editor_pictures_.size(); ++i) {
		const int32_t diff1 =
			editor_pictures_[bestmatch].upper_limit - static_cast<int32_t>(amount);
		const int32_t diff2 =
			editor_pictures_[i].upper_limit - static_cast<int32_t>(amount);

		// This is a catch-all for high amounts
		if (editor_pictures_[i].upper_limit < 0)
		{
			if (diff1 < 0) {
				bestmatch = i;
				continue;
			}

			continue;
		}

		// This is lower than the actual amount
		if (diff2 < 0)
		{
			if (editor_pictures_[bestmatch].upper_limit < 0)
				continue;

			if (diff1 < diff2) {
				bestmatch = i; // still better than previous best match
				continue;
			}

			continue;
		}

		// This is higher than the actual amount
		if
			(editor_pictures_[bestmatch].upper_limit < 0     ||
			 diff2                               < diff1 ||
			 diff1                               < 0)
		{
			bestmatch = i;
			continue;
		}
	}

	//noLog("Resource(%s): Editor_Pic '%s' for amount = %u\n",
	//name_.c_str(), editor_pictures_[bestmatch].picname.c_str(), amount);

	return editor_pictures_[bestmatch].picname;
}

const std::string& ResourceDescription::name() const {
	return name_;
}

const std::string& ResourceDescription::descname() const {
	return descname_;
}

bool ResourceDescription::detectable() const {
	return detectable_;
}

int32_t ResourceDescription::max_amount() const {
	return max_amount_;
}


}  // namespace Widelands
