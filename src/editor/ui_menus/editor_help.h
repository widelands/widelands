/*
 * Copyright (C) 2002-2004, 2006, 2009 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_HELP_H
#define WL_EDITOR_UI_MENUS_EDITOR_HELP_H

#include <map>
#include <memory>
#include <vector>

#include "logic/map_objects/map_object.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

class EditorInteractive;

struct EditorHelp : public UI::UniqueWindow {
	EditorHelp(EditorInteractive&, UI::UniqueWindow::Registry&);

private:
	struct HelpEntry {
		enum class Type {
			kTerrain,
			kTree
		};

		HelpEntry(const HelpEntry& other) : HelpEntry(other.index, other.descname, other.icon) {
		}
		HelpEntry(const Widelands::DescriptionIndex _index,
		          const std::string& _descname,
		          const Image* _icon)
		   : index(_index), descname(_descname), icon(_icon) {
		}
		Widelands::DescriptionIndex index;
		std::string descname;
		const Image* icon;

		bool operator<(const HelpEntry other) const {
			return descname < other.descname;
		}
	};

	struct HelpTab {
		HelpTab(const std::string& _key,
		        const std::string& _image_filename,
		        const std::string& _tooltip,
		        const std::string& _script_path,
		        const EditorHelp::HelpEntry::Type _type)
		   : key(_key),
		     image_filename(_image_filename),
		     tooltip(_tooltip),
		     script_path(_script_path),
		     type(_type) {
		}
		const std::string key;
		const std::string image_filename;
		const std::string tooltip;
		const std::string script_path;
		const EditorHelp::HelpEntry::Type type;
	};

	EditorInteractive& eia() const;

	// Fill table of contents
	void fill_entries(const char* key, std::vector<HelpEntry>& entries);
	void fill_terrains();
	void fill_trees();

	// Update contents when an entry is selected
	void entry_selected(const std::string& key,
	                    const std::string& script_path,
	                    const HelpEntry::Type& type);

	// UI elements
	UI::TabPanel tabs_;

	// Wrapper boxes so we can add some padding
	std::map<std::string, std::unique_ptr<UI::Box>> wrapper_boxes_;
	// Main contents boxes for each tab
	std::map<std::string, std::unique_ptr<UI::Box>> boxes_;
	// A tab's table of contents
	std::map<std::string, std::unique_ptr<UI::Listselect<Widelands::DescriptionIndex>>> lists_;
	// The contents shown when an entry is selected in a tab
	std::map<std::string, std::unique_ptr<UI::MultilineTextarea>> contents_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_HELP_H
