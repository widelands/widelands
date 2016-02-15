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

#ifndef WL_WUI_ENCYCLOPEDIA_WINDOW_H
#define WL_WUI_ENCYCLOPEDIA_WINDOW_H

#include <map>
#include <memory>

#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

class InteractivePlayer;

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(InteractivePlayer&, UI::UniqueWindow::Registry&);

private:
	struct EncyclopediaEntry {
		EncyclopediaEntry(const EncyclopediaEntry&) = default;
		EncyclopediaEntry& operator = (const EncyclopediaEntry&) = default;
		EncyclopediaEntry(const Widelands::DescriptionIndex _index,
		                  const std::string& _descname,
		                  const Image* _icon)
		   : index(_index), descname(_descname), icon(_icon) {
		}
		Widelands::DescriptionIndex index;
		std::string descname;
		const Image* icon;

		bool operator<(const EncyclopediaEntry other) const {
			return descname < other.descname;
		}
	};

	InteractivePlayer& iaplayer() const;

	// Fill table of contents
	void fill_entries(const char* key, std::vector<EncyclopediaEntry>* entries);
	void fill_buildings();
	void fill_wares();
	void fill_workers();

	// Update contents when an entry is selected
	void entry_selected(const std::string& key,
	                    const std::string& script_path,
	                    const Widelands::MapObjectType& type);

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

#endif  // end of include guard: WL_WUI_ENCYCLOPEDIA_WINDOW_H
