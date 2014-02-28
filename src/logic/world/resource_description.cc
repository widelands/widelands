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

namespace Widelands {

ResourceDescription::ResourceDescription() : is_detectable_(true), max_amount_(0) {
}

void ResourceDescription::parse(Section & s, const std::string & basedir)
{
	name_ = s.get_name();
	descname_ = s.get_string("name", s.get_name());
	is_detectable_ = s.get_bool("detectable", true);

	max_amount_ = s.get_safe_int("max_amount");
	while (Section::Value const * const v = s.get_next_val("editor_pic")) {
		Editor_Pic i;

		std::vector<std::string> const args(split_string(v->get_string(), " \t"));
		if (args.size() != 1 and args.size() != 2) {
			log
				("Resource '%s' has bad editor_pic=%s\n",
				 name_.c_str(), v->get_string());
			continue;
		}

		i.picname = basedir + "/pics/";
		i.picname += args[0];
		i.upperlimit = -1;

		if (args.size() >= 2) {
			char * endp;
			i.upperlimit = strtol(args[1].c_str(), &endp, 0);

			if (*endp) {
				log
					("Resource '%s' has bad editor_pic=%s\n",
					 name_.c_str(), v->get_string());
				continue;
			}
		}

		editor_pics_.push_back(i);
	}
	if (editor_pics_.empty())
		throw game_data_error("resource '%s' has no editor_pic", name_.c_str());
}

const std::string & ResourceDescription::get_editor_pic
	(uint32_t const amount) const
{
	uint32_t bestmatch = 0;

	assert(editor_pics_.size());

	for (uint32_t i = 1; i < editor_pics_.size(); ++i) {
		const int32_t diff1 =
			editor_pics_[bestmatch].upperlimit - static_cast<int32_t>(amount);
		const int32_t diff2 =
			editor_pics_[i].upperlimit - static_cast<int32_t>(amount);

		// This is a catch-all for high amounts
		if (editor_pics_[i].upperlimit < 0)
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
			if (editor_pics_[bestmatch].upperlimit < 0)
				continue;

			if (diff1 < diff2) {
				bestmatch = i; // still better than previous best match
				continue;
			}

			continue;
		}

		// This is higher than the actual amount
		if
			(editor_pics_[bestmatch].upperlimit < 0     ||
			 diff2                               < diff1 ||
			 diff1                               < 0)
		{
			bestmatch = i;
			continue;
		}
	}

	//noLog("Resource(%s): Editor_Pic '%s' for amount = %u\n",
	//name_.c_str(), editor_pics_[bestmatch].picname.c_str(), amount);

	return editor_pics_[bestmatch].picname;
}

const std::string& ResourceDescription::name() const {
	return name_;
}

const std::string& ResourceDescription::descname() const {
	return descname_;
}

bool ResourceDescription::is_detectable() const {
	return is_detectable_;
}

int32_t ResourceDescription::get_max_amount() const {
	return max_amount_;
}


}  // namespace Widelands
