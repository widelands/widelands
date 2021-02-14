/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_LOGIC_MUTABLE_ADDON_H
#define WL_LOGIC_MUTABLE_ADDON_H

#include <memory>
#include <regex>

#include "logic/addons.h"

namespace AddOns {

class MutableAddOn {
public:
	explicit MutableAddOn(const AddOnInfo& a);
	virtual ~MutableAddOn() {
	}
	// Creates an addon with its type matching its category
	static std::unique_ptr<MutableAddOn> create_mutable_addon(const AddOnInfo& a);
	void update_info(const std::string& descname,
	                 const std::string& author,
	                 const std::string& description,
	                 const std::string& version);
	// May throw a WLWarning, if it fails
	virtual bool write_to_disk();

	const std::string& get_internal_name() {
		return internal_name_;
	}
	const std::string& get_descname() {
		return descname_;
	}
	const std::string& get_description() {
		return description_;
	}
	const std::string& get_author() {
		return author_;
	}
	const std::string& get_version() {
		return version_;
	}
	void set_version(const std::string& version) {
		version_ = version;
	}
	AddOnCategory get_category() {
		return category_;
	}

protected:
	virtual std::string parse_requirements();
	std::string profile_path();
	void setup_temp_dir();
	void cleanup_temp_dir();

	std::string internal_name_, descname_, description_, author_, version_;
	AddOnCategory category_;

	std::string directory_, backup_path_;
};

class WorldAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
};

class TribesAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
};

class ScriptAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
};

class MapsAddon : public MutableAddOn {
public:
	explicit MapsAddon(const AddOnInfo& a);
	bool write_to_disk() override;

	struct DirectoryTree {
		std::map<std::string /* file name in add-on */, std::string /* path of source map */> maps;
		std::map<std::string, DirectoryTree> subdirectories;
	};
	DirectoryTree* get_tree() {
		return &tree_;
	}

protected:
	std::string parse_requirements() override;
	void do_recursively_create_filesystem_structure(const std::string& dir,
	                                                const DirectoryTree& tree);
	DirectoryTree tree_;

private:
	void recursively_initialize_tree_from_disk(const std::string& dir, DirectoryTree& tree);
	void parse_map_requirements(const DirectoryTree& tree, std::vector<std::string>& req);
};

class CampaignAddon : public MapsAddon {
public:
	explicit CampaignAddon(const AddOnInfo& a);
	bool write_to_disk() override;
	bool luafile_exists();

	struct CampaignInfo {
		std::string tribe, short_desc, difficulty, difficulty_icon;
	};

	const CampaignInfo& get_metadata() {
		return metadata_;
	}

	void set_tribe(const std::string& tribe) {
		metadata_.tribe = tribe;
	}

	void set_short_desc(const std::string& desc) {
		metadata_.short_desc = desc;
	}

	void set_difficulty(const std::string& difficulty) {
		metadata_.difficulty = difficulty;
	}

	void set_difficulty_icon(const std::string& difficulty_icon) {
		metadata_.difficulty_icon = difficulty_icon;
	}

private:
	void do_recursively_add_scenarios(std::string& scenarios,
	                                  const std::string& dir,
	                                  const DirectoryTree& tree);

	std::string lua_contents_;
	std::regex rex_difficulty_, rex_difficulty_icon_, rex_descname_, rex_description_,
	   rex_short_desc_, rex_tribe_, rex_scenario_;
	CampaignInfo metadata_;
};

class WinCondAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
};

class StartingCondAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
};

class ThemeAddon : public MutableAddOn {
public:
	using MutableAddOn::MutableAddOn;
	bool write_to_disk() override;
};

}  // namespace AddOns

#endif  // end of include guard: WL_LOGIC_MUTABLE_ADDON_H
