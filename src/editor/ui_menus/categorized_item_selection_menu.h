/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H
#define WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H

#include <algorithm>
#include <cmath>
#include <string>

#include "boost/format.hpp"

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "logic/description_maintainer.h"
#include "logic/map_objects/world/editor_category.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"

template <typename DescriptionType, typename ToolType>
class CategorizedItemSelectionMenu : public UI::Box {
public:
	// Creates a box with a tab panel for each category in 'categories' and
	// populates them with the 'descriptions' ordered by the category by calling
	// 'create_checkbox' for each of the descriptions. Calls
	// 'select_correct_tool' whenever a selection has been made, also keeps a
	// text label updated and updates the 'tool' with current selections. Does
	// not take ownership.
	CategorizedItemSelectionMenu(
	   UI::Panel* parent,
	   const DescriptionMaintainer<Widelands::EditorCategory>& categories,
	   const DescriptionMaintainer<DescriptionType>& descriptions,
	   std::function<UI::Checkbox*(UI::Panel* parent, const DescriptionType& descr)> create_checkbox,
	   const std::function<void()> select_correct_tool,
	   ToolType* const tool);

private:
	// Called when an item was selected.
	void selected(int32_t, bool);

	// Update the label with the currently selected object names.
	void update_label();

	const DescriptionMaintainer<DescriptionType>& descriptions_;
	std::function<void()> select_correct_tool_;
	bool protect_against_recursive_select_;
	UI::TabPanel tab_panel_;
	UI::MultilineTextarea current_selection_names_;
	std::map<int, UI::Checkbox*> checkboxes_;
	ToolType* const tool_;  // not owned
};

template <typename DescriptionType, typename ToolType>
CategorizedItemSelectionMenu<DescriptionType, ToolType>::CategorizedItemSelectionMenu(
   UI::Panel* parent,
   const DescriptionMaintainer<Widelands::EditorCategory>& categories,
   const DescriptionMaintainer<DescriptionType>& descriptions,
   const std::function<UI::Checkbox*(UI::Panel* parent, const DescriptionType& descr)>
      create_checkbox,
   const std::function<void()> select_correct_tool,
   ToolType* const tool)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     descriptions_(descriptions),
     select_correct_tool_(select_correct_tool),
     protect_against_recursive_select_(false),
     tab_panel_(this, 0, 0, g_gr->images().get("images/wui/window_background_dark.png")),
     current_selection_names_(this,
                              0,
                              0,
                              20,
                              20,
                              "",
                              UI::Align::kCenter,
                              g_gr->images().get("images/ui_basic/but1.png"),
                              UI::MultilineTextarea::ScrollMode::kNoScrolling),
     tool_(tool) {
	current_selection_names_.set_background(
	   g_gr->images().get("images/wui/window_background_dark.png"));
	add(&tab_panel_);

	for (uint32_t category_index = 0; category_index < categories.size(); ++category_index) {
		const Widelands::EditorCategory& category = categories.get(category_index);
		// Internal immovables not to be placed by map editors.
		if (category.name() == "immovables_none") {
			continue;
		}

		std::vector<int> item_indices;
		for (size_t j = 0; j < descriptions_.size(); ++j) {
			if (descriptions_.get(j).editor_category()->name() != category.name()) {
				continue;
			}
			item_indices.push_back(j);
		}

		UI::Box* vertical = new UI::Box(&tab_panel_, 0, 0, UI::Box::Vertical);
		const int kSpacing = 5;
		vertical->add_space(kSpacing);

		int nitems_handled = 0;
		UI::Box* horizontal = nullptr;
		for (const int i : item_indices) {
			if (nitems_handled % category.items_per_row() == 0) {
				horizontal = new UI::Box(vertical, 0, 0, UI::Box::Horizontal);
				horizontal->add_space(kSpacing);

				vertical->add(horizontal);
				vertical->add_space(kSpacing);
			}
			assert(horizontal != nullptr);

			UI::Checkbox* cb = create_checkbox(horizontal, descriptions_.get(i));
			cb->set_state(tool_->is_enabled(i));
			cb->changedto.connect(boost::bind(&CategorizedItemSelectionMenu::selected, this, i, _1));
			checkboxes_[i] = cb;
			horizontal->add(cb);
			horizontal->add_space(kSpacing);
			++nitems_handled;
		}
		tab_panel_.add(category.name(), category.picture(), vertical, category.descname());
	}
	add(&current_selection_names_, UI::Box::Resizing::kFullSize);
	tab_panel_.sigclicked.connect(boost::bind(&CategorizedItemSelectionMenu::update_label, this));
	update_label();
}

template <typename DescriptionType, typename ToolType>
void CategorizedItemSelectionMenu<DescriptionType, ToolType>::selected(const int32_t n,
                                                                       const bool t) {
	if (protect_against_recursive_select_)
		return;

	//  TODO(unknown): This code is erroneous. It checks the current key state. What it
	//  needs is the key state at the time the mouse was clicked. See the
	//  usage comment for get_key_state.
	const bool multiselect = SDL_GetModState() & KMOD_CTRL;
	if (!t && (!multiselect || tool_->get_nr_enabled() == 1))
		checkboxes_[n]->set_state(true);
	else {
		if (!multiselect) {
			for (uint32_t i = 0; tool_->get_nr_enabled(); ++i)
				tool_->enable(i, false);
			//  disable all checkboxes
			protect_against_recursive_select_ = true;
			const int32_t size = checkboxes_.size();
			for (int32_t i = 0; i < size; ++i) {
				if (i != n)
					checkboxes_[i]->set_state(false);
			}
			protect_against_recursive_select_ = false;
		}

		tool_->enable(n, t);
		select_correct_tool_();
		update_label();
	}
}

template <typename DescriptionType, typename ToolType>
void CategorizedItemSelectionMenu<DescriptionType, ToolType>::update_label() {
	current_selection_names_.set_size(tab_panel_.get_inner_w(), 20);
	std::string buf = "";
	constexpr int max_string_size = 100;
	int j = tool_->get_nr_enabled();
	for (int i = 0; j && buf.size() < max_string_size; ++i) {
		if (tool_->is_enabled(i)) {
			if (j < tool_->get_nr_enabled()) {
				buf += " • ";
			}
			buf += descriptions_.get(i).descname();
			--j;
		}
	}
	if (buf.size() > max_string_size) {
		/** TRANSLATORS: %s are the currently selected items in an editor tool*/
		buf = (boost::format(_("Current: %s …")) % buf).str();
	} else if (buf.empty()) {
		/** TRANSLATORS: Help text in an editor tool*/
		buf = _("Click to select an item. Use the Ctrl key to select multiple items.");
	} else {
		/** TRANSLATORS: %s are the currently selected items in an editor tool*/
		buf = (boost::format(_("Current: %s")) % buf).str();
	}
	current_selection_names_.set_text(buf);
}

#endif  // end of include guard: WL_EDITOR_UI_MENUS_CATEGORIZED_ITEM_SELECTION_MENU_H
