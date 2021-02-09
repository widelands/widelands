/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#include "logic/addon.h"

#include <memory>

#include "base/i18n.h"
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

namespace AddOns {

static void do_recursively_copy_file_or_directory(const std::string& source,
                                                  const std::string& dest) {
	if (g_fs->is_directory(source)) {
		g_fs->ensure_directory_exists(dest);
		for (const std::string& file : g_fs->list_directory(source)) {
			do_recursively_copy_file_or_directory(
			   file, dest + FileSystem::file_separator() + FileSystem::fs_filename(file.c_str()));
		}
	} else {
		FileRead fr;
		fr.open(*g_fs, source);
		const size_t bytes = fr.get_size();
		std::unique_ptr<char[]> data(new char[bytes]);
		fr.data_complete(data.get(), bytes);

		FileWrite fw;
		fw.data(data.get(), bytes);
		fw.write(*g_fs, dest);
	}
}

std::unique_ptr<Addon> Addon::create_mutable_addon(const AddOnInfo& a) {
	switch (a.category) {
	case AddOnCategory::kWorld:
		return std::unique_ptr<WorldAddon>(new WorldAddon(a));
	case AddOnCategory::kTribes:
		return std::unique_ptr<TribesAddon>(new TribesAddon(a));
	case AddOnCategory::kScript:
		return std::unique_ptr<ScriptAddon>(new ScriptAddon(a));
	case AddOnCategory::kMaps:
		return std::unique_ptr<MapsAddon>(new MapsAddon(a));
	case AddOnCategory::kCampaign:
		return std::unique_ptr<CampaignAddon>(new CampaignAddon(a));
	case AddOnCategory::kWinCondition:
		return std::unique_ptr<WinCondAddon>(new WinCondAddon(a));
	case AddOnCategory::kStartingCondition:
		return std::unique_ptr<StartingCondAddon>(new StartingCondAddon(a));
	case AddOnCategory::kTheme:
		return std::unique_ptr<ThemeAddon>(new ThemeAddon(a));
	default:
		return std::unique_ptr<Addon>(new Addon(a));
	}
}

Addon::Addon(const AddOnInfo& a)
   : internal_name_(a.internal_name),
     descname_(a.unlocalized_descname),
     description_(a.unlocalized_description),
     author_(a.unlocalized_author),
     version_(AddOns::version_to_string(a.version, false)),
     category_(a.category),
     directory_(kAddOnDir + FileSystem::file_separator() + internal_name_),
     profile_path_(directory_ + FileSystem::file_separator() + kAddOnMainFile) {
}

void Addon::update_info(const std::string& descname,
                        const std::string& author,
                        const std::string& description,
                        const std::string& version) {
	descname_ = descname;
	author_ = author;
	description_ = description;
	version_ = version;
}

std::string Addon::parse_requirements() {
	// · We need to read the original `addon` file (if it exists) to
	//   determine the requirements. Then write the file, and we are done.
	const std::string directory = kAddOnDir + FileSystem::file_separator() + internal_name_;
	bool dir_exists = g_fs->file_exists(directory);
	const std::string profile_path = directory + FileSystem::file_separator() + kAddOnMainFile;

	std::string requires;
	if (dir_exists) {
		Profile p;
		p.read(profile_path.c_str());
		requires = p.get_safe_section("global").get_safe_string("requires");
	}
	return requires;
}

bool Addon::write_to_disk() {

	// Step 1: Gather the requirements of all contained maps
	std::string requires = parse_requirements();

	// Step 2: Create the `addon` file.
	g_fs->ensure_directory_exists(directory_);
	// Write profile
	{
		Profile p;
		Section& s = p.create_section("global");

		s.set_translated_string("name", descname_);
		s.set_translated_string("description", description_);
		s.set_translated_string("author", author_);
		s.set_string("version", version_);
		s.set_string("category", AddOns::kAddOnCategories.at(category_).internal_name);
		s.set_string("requires", requires);

		p.write(profile_path_.c_str(), false);
	}

	return true;
}

MapsAddon::MapsAddon(const AddOnInfo& a) : Addon(a) {
	recursively_initialize_tree_from_disk(directory_, tree_);
}

std::string MapsAddon::parse_requirements() {
	// · For maps, we need to gather the information regarding the maps' required add-ons
	//   before writing the profile so we can generate the correct `requires` string.
	std::string requires;
	std::vector<std::string> req;
	try {
		parse_map_requirements(tree_, req);
	} catch (const WException& e) {
		throw WLWarning("", _("A map file is invalid:\n%s"), e.what());
	}
	if (const size_t nr = req.size()) {
		requires = req[0];
		for (size_t i = 1; i < nr; ++i) {
			requires += ',';
			requires += req[i];
		}
	}
	return requires;
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

void MapsAddon::do_recursively_create_filesystem_structure(const std::string& dir,
                                                           const DirectoryTree& tree,
                                                           const std::string& addon_basedir,
                                                           const std::string& backup_basedir) {
	// Dirs
	for (const auto& pair : tree.subdirectories) {
		const std::string subdir = dir + FileSystem::file_separator() + pair.first;
		g_fs->ensure_directory_exists(subdir);
		do_recursively_create_filesystem_structure(
		   subdir, pair.second, addon_basedir, backup_basedir);
	}

	// Maps
	for (const auto& pair : tree.maps) {
		// If the filepath starts with the add-on's path, we need to use the backup instead
		std::string source_path = pair.second;
		if (source_path.size() >= addon_basedir.size() &&
		    source_path.compare(0, addon_basedir.size(), addon_basedir) == 0) {
			assert(!backup_basedir.empty());
			std::string temp = source_path.substr(addon_basedir.size());
			source_path = backup_basedir;
			source_path += temp;
		}
		assert(source_path.size() > kWidelandsMapExtension.size());
		assert(source_path.compare(source_path.size() - kWidelandsMapExtension.size(),
		                           kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0);

		do_recursively_copy_file_or_directory(
		   source_path, dir + FileSystem::file_separator() + pair.first);
	}
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
	// If the add-on exists on disk already and our add-on is of type Map Set,
	// make a backup copy of the whole original add-on in ~/.widelands/temp, then
	// delete the original add-on directory and create it anew.
	std::string backup_path;
	if (g_fs->file_exists(directory_)) {
		backup_path = kTempFileDir + FileSystem::file_separator() + descname_ + kTempFileExtension;
		if (g_fs->file_exists(backup_path)) {
			g_fs->fs_unlink(backup_path);
		}
		g_fs->fs_rename(directory_, backup_path);
	}

	if (!Addon::write_to_disk()) {
		return false;
	}

	// Create the directory structure and copy the maps
	do_recursively_create_filesystem_structure(directory_, tree_, directory_, backup_path);

	// Delete the backup (if it existed)
	if (!backup_path.empty()) {
		g_fs->fs_unlink(backup_path);
	}
	return true;
}

bool ThemeAddon::write_to_disk() {
	if (!Addon::write_to_disk()) {
		return false;
	}

	if (!g_fs->file_exists(directory_ + FileSystem::file_separator() + "init.lua")) {
		// Copy default theme into addon folder as starting point
		do_recursively_copy_file_or_directory("templates/default/", directory_);
	}

	return true;
}

}  // namespace AddOns
