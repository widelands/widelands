/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include <memory>

#include <SDL.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "config.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/world.h"
#include "sound/sound_handler.h"

using namespace Widelands;

namespace  {

constexpr size_t kCutoff = 3; // NOCOM For testing, we don't want the JSON to get huge!

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw wexception("Unable to initialize SDL: %s", SDL_GetError());
	}

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	// We don't really need graphics or sound here, but we will get error messages
	// when they aren't initialized
	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false);

	g_sound_handler.init();
	g_sound_handler.nosound_ = true;
}

// Defines some convenience writing functions for the JSOn format
class JSONFileWrite : public FileWrite {
public:
	JSONFileWrite() : FileWrite(), level_(0) {}

	void write_string(const std::string& s, bool use_indent = false) {
		std::string writeme = s;
		if (use_indent) {
			for (int i = 0; i < level_; ++i) {
				writeme = (boost::format("   %s") % writeme).str();
			}
		}
		data(writeme.c_str(), writeme.size());
	}
	void write_key_value(const std::string& key, const std::string& quoted_value) {
		write_string((boost::format("\"%s\": %s") % key % quoted_value).str(), true);
	}
	void write_key_value_string(const std::string& key, const std::string& value) {
		std::string quoted_value = value;
		boost::replace_all(quoted_value, "\"", "\\\"");
		write_key_value(key, "\"" + value + "\"");
	}
	void write_key_value_int(const std::string& key, const int value) {
		write_key_value(key, boost::lexical_cast<std::string>(value));
	}
	void open_brace() {
		write_string("{\n", true);
		++level_;
	}
	void close_brace(int current = 0, int total = 0) {
		--level_;
		// JSON hates a final comma
		if (current < total - 1) {
			write_string("},\n", true);
		} else {
			write_string("}\n", true);
		}
	}
	void open_array(const std::string& name) {
		write_string((boost::format("\"%s\":[\n") % name).str(), true);
		++level_;
		//write_string("{\n""", true);
		//++level_;
	}
	void close_array(int current = 0, int total = 0) {
		--level_;
		write_string("\n");
		// write_string("}\n", true);
		//--level_;
		// JSON hates a final comma
		if (current < total - 1) {
			write_string("],\n", true);
		} else {
			write_string("]\n", true);
		}
	}
	// JSON hates a final comma
	void close_element() {
		write_string(",\n");
	}
private:
	int level_;
};

}  // namespace

int main(int argc, char ** argv)
{

	try {
		i18n::set_locale("en");
		initialize();

		EditorGameBase egbase(nullptr);
		const World& world = egbase.world();
		const Tribes& tribes = egbase.tribes();

		// Write JSON.
		{
			JSONFileWrite fw;
			fw.open_brace(); // Main
			fw.open_array("tribes"); // Tribes

			/// Tribes
			std::vector<TribeBasicInfo> tribeinfos = tribes.get_all_tribeinfos();
			for (size_t tribe_index = 0; tribe_index < tribeinfos.size() && tribe_index < kCutoff; ++tribe_index) {
				const TribeBasicInfo& tribe_info = tribeinfos[tribe_index];
				const TribeDescr* tribe =
						egbase.tribes().get_tribe_descr(egbase.tribes().tribe_index(tribe_info.name));
				log("Writing tribe: %s\n", tribe->name().c_str());
				fw.open_brace(); // TribeDescr
				fw.write_key_value_string("name", tribe_info.name);
				fw.close_element();
				fw.write_key_value_string("descname", tribe_info.descname);
				fw.close_element();
				fw.write_key_value_string("author", tribe_info.author);
				fw.close_element();
				fw.write_key_value_string("tooltip", tribe_info.tooltip);
				fw.close_element();
				fw.write_key_value_string("icon", tribe_info.icon);
				fw.close_element();

				/// Buildings

				fw.open_array("buildings"); // Buildings
				const std::vector<DescriptionIndex>& buildings = tribe->buildings();
				for (size_t building_index = 0; building_index < buildings.size() && building_index < kCutoff; ++building_index) {
					const BuildingDescr* building = tribes.get_building_descr(building_index);
					log("Writing building: %s\n", building->name().c_str());
					fw.open_brace();
					fw.write_key_value_string("descname", building->descname());
					fw.write_string("\n");

					fw.close_brace(building_index, std::min(buildings.size(), kCutoff)); // Building
				}
				fw.close_array(); // Buildings

				fw.close_brace(tribe_index, std::min(tribeinfos.size(), kCutoff)); // TribeDescr
			}
			fw.close_array(); // Tribes

			// NOCOM write_key_value_int("number", 42);

			fw.close_brace(); // Main
			fw.write(FileSystem::create("."), "mapobjects.json");
		}
	}
	catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		g_sound_handler.shutdown();
		return 1;
	}
	g_sound_handler.shutdown();
	return 0;
}
