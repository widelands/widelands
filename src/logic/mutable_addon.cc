/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/mutable_addon.h"

#include <memory>
#include <regex>
#include <string>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "io/profile.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "logic/map.h"
#include "map_io/map_elemental_packet.h"
#include "map_io/map_loader.h"

namespace AddOns {

size_t MutableAddOn::do_recursively_copy_file_or_directory(const std::string& source,
                                                           const std::string& dest,
                                                           const bool dry_run) {
	size_t result = 0;
	if (g_fs->is_directory(source)) {
		if (!dry_run) {
			g_fs->ensure_directory_exists(dest);
		}
		for (const std::string& file : g_fs->list_directory(source)) {
			result += do_recursively_copy_file_or_directory(
			   file, dest + FileSystem::file_separator() + FileSystem::fs_filename(file.c_str()),
			   dry_run);
		}
	} else {
		result++;
		if (!dry_run) {
			FileRead fr;
			fr.open(*g_fs, source);
			const size_t bytes = fr.get_size();
			std::unique_ptr<char[]> data(new char[bytes]);
			fr.data_complete(data.get(), bytes);

			FileWrite fw;
			fw.data(data.get(), bytes);
			fw.write(*g_fs, dest);
			callback_progress_(1);
		}
	}
	return result;
}

static std::string read_text_file(const std::string& filename) {
	FileRead fr;
	fr.open(*g_fs, filename);
	const size_t bytes = fr.get_size();
	std::unique_ptr<char[]> data(new char[bytes + 1]);
	fr.data_complete(data.get(), bytes);
	data[bytes] = 0;
	return data.get();
}

std::unique_ptr<MutableAddOn> MutableAddOn::create_mutable_addon(const AddOnInfo& a) {
	switch (a.category) {
	case AddOnCategory::kWorld:
		return std::unique_ptr<WorldAddon>(new WorldAddon(a));
	case AddOnCategory::kTribes:
		return std::unique_ptr<TribesAddon>(new TribesAddon(a));
	case AddOnCategory::kScript:
		return std::unique_ptr<ScriptAddon>(new ScriptAddon(a));
	case AddOnCategory::kMaps:
		return std::unique_ptr<MapsAddon>(new MapsAddon(a));
	case AddOnCategory::kMapGenerator:
		return std::unique_ptr<MapGenAddon>(new MapGenAddon(a));
	case AddOnCategory::kUIPlugin:
		return std::unique_ptr<UIPluginAddon>(new UIPluginAddon(a));
	case AddOnCategory::kCampaign:
		return std::unique_ptr<CampaignAddon>(new CampaignAddon(a));
	case AddOnCategory::kWinCondition:
		return std::unique_ptr<WinCondAddon>(new WinCondAddon(a));
	case AddOnCategory::kStartingCondition:
		return std::unique_ptr<StartingCondAddon>(new StartingCondAddon(a));
	case AddOnCategory::kTheme:
		return std::unique_ptr<ThemeAddon>(new ThemeAddon(a));
	default:
		throw wexception("Invalid category %u for addon %s", static_cast<uint32_t>(a.category),
		                 a.internal_name.c_str());
	}
}

MutableAddOn::MutableAddOn(const AddOnInfo& a)
   : internal_name_(a.internal_name),
     descname_(a.unlocalized_descname),
     description_(a.unlocalized_description),
     author_(a.unlocalized_author),
     version_(AddOns::version_to_string(a.version, false)),
     min_wl_version_(a.min_wl_version),
     max_wl_version_(a.max_wl_version),
     category_(a.category),
     force_sync_safe_(false),
     directory_(kAddOnDir + FileSystem::file_separator() + internal_name_) {
}

std::string MutableAddOn::profile_path() {
	return directory_ + FileSystem::file_separator() + kAddOnMainFile;
}

void MutableAddOn::update_info(const std::string& descname,
                               const std::string& author,
                               const std::string& description,
                               const std::string& version,
                               const std::string& min_wl_version,
                               const std::string& max_wl_version) {
	descname_ = descname;
	author_ = author;
	description_ = description;
	version_ = version;
	min_wl_version_ = min_wl_version;
	max_wl_version_ = max_wl_version;
}

void MutableAddOn::setup_temp_dir() {
	// If the add-on exists on disk already create it in ~/.widelands/temp,
	// then delete the original add-on directory and move it over.
	if (g_fs->file_exists(directory_)) {
		backup_path_ = directory_;
		directory_ = kTempFileDir + FileSystem::file_separator() + timestring() + ".autogen." +
		             internal_name_ + kTempFileExtension;
		if (g_fs->file_exists(directory_)) {
			g_fs->fs_unlink(directory_);
		}
	}
}

void MutableAddOn::cleanup_temp_dir() {
	// Move addon from temp to addons
	if (!backup_path_.empty()) {
		g_fs->fs_unlink(backup_path_);
		g_fs->fs_rename(directory_, backup_path_);
		directory_ = backup_path_;
		backup_path_.clear();
	}
}

std::string MutableAddOn::parse_requirements() {
	// · We need to read the original `addon` file (if it exists) to
	//   determine the requirements. Then write the file, and we are done.
	const std::string directory = kAddOnDir + FileSystem::file_separator() + internal_name_;
	bool dir_exists = g_fs->file_exists(directory);
	const std::string profile_path = directory + FileSystem::file_separator() + kAddOnMainFile;

	std::string requirements;
	if (dir_exists) {
		Profile p;
		p.read(profile_path.c_str());
		requirements = p.get_safe_section("global").get_safe_string("requires");
	}
	return requirements;
}

bool MutableAddOn::write_to_disk() {
	callback_init_(1);
	callback_progress_(0);

	// Step 1: Gather the requirements of all contained maps
	const std::string requirements = parse_requirements();

	// Step 2: Create the addons directory
	g_fs->ensure_directory_exists(directory_);

	// Step 3: Write profile (`addon` file)
	Profile p;
	Section& s = p.create_section("global");

	s.set_translated_string("name", descname_);
	s.set_translated_string("description", description_);
	s.set_string("author", author_);
	s.set_string("version", version_);
	s.set_string("category", AddOns::kAddOnCategories.at(category_).internal_name);
	s.set_string("requires", requirements);
	s.set_string("min_wl_version", min_wl_version_);
	s.set_string("max_wl_version", max_wl_version_);
	s.set_bool("sync_safe", force_sync_safe_);

	p.write(profile_path().c_str(), false);

	callback_progress_(1);
	return true;
}

MapsAddon::MapsAddon(const AddOnInfo& a) : MutableAddOn(a) {
	recursively_initialize_tree_from_disk(directory_, tree_);
	if (tree_.subdirectories.empty() && tree_.maps.empty()) {
		tree_.subdirectories.emplace(
		   a.internal_name.substr(0, a.internal_name.size() - kAddOnExtension.size()),
		   DirectoryTree());
	}

	Profile profile;
	std::string profile_path = directory_;
	profile_path += FileSystem::file_separator();
	profile_path += "dirnames";
	profile.read(profile_path.c_str());
	if (Section* s = profile.get_section("global")) {
		for (;;) {
			const Section::Value* v = s->get_next_val();
			if (v == nullptr) {
				break;
			}
			dirnames_[v->get_name()] = v->get_untranslated_string();
		}
	}
}

std::string MapsAddon::parse_requirements() {
	// · For maps, we need to gather the information regarding the maps' required add-ons
	//   before writing the profile so we can generate the correct `requires` string.
	std::string requirements;
	std::vector<std::string> req;
	try {
		parse_map_requirements(tree_, req);
	} catch (const WException& e) {
		throw WLWarning("", _("A map file is invalid:\n%s"), e.what());
	}
	if (const size_t nr = req.size()) {
		requirements = req[0];
		for (size_t i = 1; i < nr; ++i) {
			requirements += ',';
			requirements += req[i];
		}
	}
	return requirements;
}

void MapsAddon::parse_map_requirements(const DirectoryTree& tree, std::vector<std::string>& req) {
	for (const auto& pair : tree.subdirectories) {
		parse_map_requirements(pair.second, req);
	}

	// Place to save temp data
	Widelands::Map map;
	Widelands::MapElementalPacket packet;

	for (const auto& pair : tree.maps) {
		std::unique_ptr<FileSystem> fs(g_fs->make_sub_file_system(pair.second));
		assert(fs);

		packet.pre_read(*fs, &map);

		for (const auto& r : map.required_addons()) {
			if (std::find(req.begin(), req.end(), r.first) == req.end()) {
				req.push_back(r.first);
			}
		}
	}
}

const AddOnVersion MapsAddon::kNoVersionRequirement = {std::numeric_limits<uint32_t>::max()};

AddOnVersion MapsAddon::detect_min_wl_version(const DirectoryTree* start) const {
	if (start == nullptr) {
		return detect_min_wl_version(&tree_);
	}

	AddOnVersion min_version = kNoVersionRequirement;

	for (const auto& pair : start->subdirectories) {
		AddOnVersion v = detect_min_wl_version(&pair.second);
		if (is_newer_version(v, min_version)) {
			min_version = v;
		}
	}

	for (const auto& pair : start->maps) {
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(pair.second);
		if (ml == nullptr) {
			log_warn("%s: %s (%s) is not a valid map file!", get_internal_name().c_str(),
			         pair.first.c_str(), pair.second.c_str());
			continue;
		}

		map.set_filename(pair.second);
		ml->preload_map(true, nullptr);

		AddOnVersion v = AddOns::string_to_version(map.version().minimum_required_widelands_version);
		if (is_newer_version(v, min_version)) {
			min_version = v;
		}
	}

	return min_version;
}

std::map<std::string, unsigned> MapsAddon::count_all_dirnames(const DirectoryTree* start) const {
	if (start == nullptr) {
		return count_all_dirnames(&tree_);
	}

	std::map<std::string, unsigned> result;
	auto increment = [&result](const std::string& s, unsigned delta) {
		auto it = result.find(s);
		if (it == result.end()) {
			result[s] = delta;
		} else {
			it->second += delta;
		}
	};

	for (const auto& pair : start->subdirectories) {
		increment(pair.first, 1);
		for (const auto& subdir : count_all_dirnames(&pair.second)) {
			increment(subdir.first, subdir.second);
		}
	}
	return result;
}

size_t MapsAddon::do_recursively_create_filesystem_structure(const std::string& dir,
                                                             const DirectoryTree& tree,
                                                             std::set<std::string>* all_dirnames,
                                                             const bool dry_run) {
	size_t result = 0;
	// Dirs
	for (const auto& pair : tree.subdirectories) {
		if (all_dirnames != nullptr) {
			all_dirnames->insert(pair.first);
		}
		const std::string subdir = dir + FileSystem::file_separator() + pair.first;
		if (!dry_run) {
			g_fs->ensure_directory_exists(subdir);
		}
		result +=
		   do_recursively_create_filesystem_structure(subdir, pair.second, all_dirnames, dry_run);
	}

	// Maps
	for (const auto& pair : tree.maps) {
		result++;
		if (dry_run) {
			continue;
		}

		std::string source_path = pair.second;
		assert(source_path.size() > kWidelandsMapExtension.size());
		assert(source_path.compare(source_path.size() - kWidelandsMapExtension.size(),
		                           kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0);

		do_recursively_copy_file_or_directory(
		   source_path, dir + FileSystem::file_separator() + pair.first, dry_run);
	}
	return result;
}

void MapsAddon::recursively_initialize_tree_from_disk(const std::string& dir, DirectoryTree& tree) {
	for (const std::string& file : g_fs->list_directory(dir)) {
		if (FileSystem::filename_ext(file) == kWidelandsMapExtension) {
			tree.maps[FileSystem::fs_filename(file.c_str())] = file;
		} else if (g_fs->is_directory(file)) {
			DirectoryTree child;
			recursively_initialize_tree_from_disk(file, child);
			tree.subdirectories[FileSystem::fs_filename(file.c_str())] = child;
		}
	}
}

bool MapsAddon::write_to_disk() {
	setup_temp_dir();

	if (!MutableAddOn::write_to_disk()) {
		return false;
	}

	// Create the directory structure and copy the maps
	std::set<std::string> all_dirnames;
	callback_init_(
	   do_recursively_create_filesystem_structure(directory_, tree_, &all_dirnames, true));
	do_recursively_create_filesystem_structure(directory_, tree_, nullptr, false);

	// Create dirnames profile
	Profile dirnames;
	Section& s = dirnames.pull_section("global");
	for (const std::string& dir : all_dirnames) {
		const auto it = dirnames_.find(dir);
		s.set_translated_string(
		   dir.c_str(), (it != dirnames_.end() && !it->second.empty()) ? it->second : dir);
	}
	std::string path = directory_;
	path += FileSystem::file_separator();
	path += "dirnames";
	dirnames.write(path.c_str());

	cleanup_temp_dir();

	return true;
}

CampaignAddon::CampaignAddon(const AddOnInfo& a)
   : MapsAddon(a),
     // (?:^|$|\\r\\n|\\n|.) is the c++ equivalent for multiline regex matching of .
     // Find difficulties.descname
     rex_difficulty_("(difficulties(?:^|$|\\r\\n|\\n|.)*descname\\s*=\\s*_\")(.*)(?=\"(?:^|$|"
                     "\\r\\n|\\n|.)*image)"),
     // Find difficulties.image
     rex_difficulty_icon_("(image\\s*=\\s*\")(.*)(?=\")"),
     // Find campaigns.descname
     rex_descname_("(campaigns(?:^|$|\\r\\n|\\n|.)*descname\\s*=\\s*_\")(.*)(?=\")"),
     // Find campaigns.description
     rex_description_("(description\\s*=\\s*_\")(.*)(?=\")"),
     // Find campaigns.difficulty.description
     rex_short_desc_("(difficulty.*description\\s*=\\s*_\")(.*)(?=\")"),
     // Find campaigns.tribe
     rex_tribe_("(tribe\\s*=\\s*\")(.*)(?=\")"),
     // Find campaigns.scenarios
     rex_scenario_("(scenarios\\s*=\\s*\\{)(?:^|$|\\r\\n|\\n|.)*(?=\"dummy)") {
	if (luafile_exists()) {
		auto extract_string = [this](std::regex& rex) {
			std::smatch match;
			if (std::regex_search(lua_contents_, match, rex) && match.size() >= 2) {
				// 0: full match, 1: (tribe = "), 2: what we want
				return match[2].str();
			}
			return std::string("");
		};

		lua_contents_ = read_text_file(directory_ + FileSystem::file_separator() + "campaigns.lua");
		metadata_.tribe = extract_string(rex_tribe_);
		metadata_.difficulty = extract_string(rex_difficulty_);
		metadata_.difficulty_icon = extract_string(rex_difficulty_icon_);
		metadata_.short_desc = extract_string(rex_short_desc_);
	}
}

bool CampaignAddon::luafile_exists() {
	return g_fs->file_exists(directory_ + FileSystem::file_separator() + "campaigns.lua");
}

void CampaignAddon::do_recursively_add_scenarios(std::string& scenarios,
                                                 const std::string& dir,
                                                 const DirectoryTree& tree) {
	// Dirs
	for (const auto& pair : tree.subdirectories) {
		const std::string subdir = dir + pair.first + FileSystem::file_separator();
		do_recursively_add_scenarios(scenarios, subdir, pair.second);
	}

	// Maps
	for (const auto& pair : tree.maps) {
		// Indent line to match template
		scenarios.append("\n            \"" + internal_name_ + ":" + dir + pair.first + "\",");
	}
}

bool CampaignAddon::write_to_disk() {
	setup_temp_dir();

	if (!MutableAddOn::write_to_disk()) {  // NOLINT
		return false;
	}

	// Create the directory structure and copy the maps
	callback_init_(do_recursively_create_filesystem_structure(directory_, tree_, nullptr, true) + 1);
	do_recursively_create_filesystem_structure(directory_, tree_, nullptr, false);

	// Modify campaigns.lua
	std::string scenario_list;
	do_recursively_add_scenarios(scenario_list, "", tree_);

	bool init = false;
	std::string luafile_out = directory_ + FileSystem::file_separator() + "campaigns.lua";
	std::string luafile_in = backup_path_ + FileSystem::file_separator() + "campaigns.lua";
	if (g_fs->file_exists(luafile_in)) {
		// Make a backup in case the user unintentionally overwrites his modifications
		do_recursively_copy_file_or_directory(luafile_in, luafile_out + ".backup", false);
	} else {
		luafile_in = "templates/campaigns.lua";
		init = true;
	}

	lua_contents_ = read_text_file(luafile_in);
	if (init) {
		// Addon name is only set during initialization
		lua_contents_ = std::regex_replace(lua_contents_, std::regex("_addon_"), internal_name_);
	}

	// Insert data using regex magic
	lua_contents_ = std::regex_replace(lua_contents_, rex_description_, "$1" + description_);
	lua_contents_ =
	   std::regex_replace(lua_contents_, rex_difficulty_icon_, "$1" + metadata_.difficulty_icon);
	lua_contents_ = std::regex_replace(lua_contents_, rex_difficulty_, "$1" + metadata_.difficulty);
	lua_contents_ = std::regex_replace(lua_contents_, rex_descname_, "$1" + descname_);
	lua_contents_ = std::regex_replace(lua_contents_, rex_short_desc_, "$1" + metadata_.short_desc);
	lua_contents_ = std::regex_replace(lua_contents_, rex_tribe_, "$1" + metadata_.tribe);
	lua_contents_ =
	   std::regex_replace(lua_contents_, rex_scenario_, "$1" + scenario_list + "\n            ");

	// Flush
	FileWrite fw;
	fw.data(lua_contents_.c_str(), lua_contents_.size());
	fw.write(*g_fs, luafile_out);

	cleanup_temp_dir();

	callback_progress_(1);
	return true;
}

bool ThemeAddon::write_to_disk() {
	if (!MutableAddOn::write_to_disk()) {
		return false;
	}

	if (!g_fs->file_exists(directory_ + FileSystem::file_separator() + "init.lua")) {
		// Copy default theme into addon folder as starting point
		callback_init_(do_recursively_copy_file_or_directory("templates/default/", directory_, true));
		do_recursively_copy_file_or_directory("templates/default/", directory_, false);
	}

	return true;
}

}  // namespace AddOns
