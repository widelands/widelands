/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#include "graphic/style_manager.h"

#include <memory>

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "scripting/lua_interface.h"

namespace {
// Read image filename and RGB color from LuaTable
UI::PanelStyleInfo* read_style(const LuaTable& table) {
	const std::string image = table.get_string("image");
	std::vector<int> rgbcolor = table.get_table("color")->array_entries<int>();
	if (rgbcolor.size() != 3) {
		throw wexception("Expected 3 entries for RGB color, but got %" PRIuS ".", rgbcolor.size());
	}
	return new UI::PanelStyleInfo(image.empty() ? nullptr : g_gr->images().get(image),
	                              RGBAColor(rgbcolor[0], rgbcolor[1], rgbcolor[2], 0));
}

// Stupid completeness check - enum classes weren't meant for iterating, so we just compare the size
// to the last enum member. This assumes that there are no holes in the enum, and will need
// adjusting if the last enum member changes.
void check_completeness(const std::string& name, size_t map_size, size_t last_enum_member) {
	if (map_size != last_enum_member + 1) {
		throw wexception("StyleManager: There is a definition missing for the '%s'.", name.c_str());
	}
}
}  // namespace

void StyleManager::init() {
	buttonstyles_.clear();
	sliderstyles_.clear();
	tabpanelstyles_.clear();
	editboxstyles_.clear();
	dropdownstyles_.clear();
	scrollbarstyles_.clear();

	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script(kTemplateDir + "init.lua"));

	// Buttons
	std::unique_ptr<LuaTable> element_table = table->get_table("buttons");
	std::unique_ptr<LuaTable> style_table = element_table->get_table("fsmenu");
	add_button_style(UI::ButtonStyle::kFsMenuMenu, *style_table->get_table("menu").get());
	add_button_style(UI::ButtonStyle::kFsMenuPrimary, *style_table->get_table("primary").get());
	add_button_style(UI::ButtonStyle::kFsMenuSecondary, *style_table->get_table("secondary").get());
	style_table = element_table->get_table("wui");
	add_button_style(UI::ButtonStyle::kWuiMenu, *style_table->get_table("menu").get());
	add_button_style(UI::ButtonStyle::kWuiPrimary, *style_table->get_table("primary").get());
	add_button_style(UI::ButtonStyle::kWuiSecondary, *style_table->get_table("secondary").get());
	add_button_style(
	   UI::ButtonStyle::kWuiBuildingStats, *style_table->get_table("building_stats").get());
	check_completeness(
	   "buttons", buttonstyles_.size(), static_cast<size_t>(UI::ButtonStyle::kWuiBuildingStats));

	// Sliders
	element_table = table->get_table("sliders");
	style_table = element_table->get_table("fsmenu");
	add_slider_style(UI::SliderStyle::kFsMenu, *style_table->get_table("menu").get());
	style_table = element_table->get_table("wui");
	add_slider_style(UI::SliderStyle::kWuiLight, *style_table->get_table("light").get());
	add_slider_style(UI::SliderStyle::kWuiDark, *style_table->get_table("dark").get());
	check_completeness(
	   "sliders", sliderstyles_.size(), static_cast<size_t>(UI::SliderStyle::kWuiDark));

	// Tabpanels
	element_table = table->get_table("tabpanels");
	style_table = element_table->get_table("fsmenu");
	add_tabpanel_style(UI::TabPanelStyle::kFsMenu, *style_table->get_table("menu").get());
	style_table = element_table->get_table("wui");
	add_tabpanel_style(UI::TabPanelStyle::kWuiLight, *style_table->get_table("light").get());
	add_tabpanel_style(UI::TabPanelStyle::kWuiDark, *style_table->get_table("dark").get());
	check_completeness(
	   "tabpanels", tabpanelstyles_.size(), static_cast<size_t>(UI::TabPanelStyle::kWuiDark));

	// Editboxes
	element_table = table->get_table("editboxes");
	style_table = element_table->get_table("fsmenu");
	add_style(UI::PanelStyle::kFsMenu, *style_table->get_table("menu").get(), &editboxstyles_);
	style_table = element_table->get_table("wui");
	add_style(UI::PanelStyle::kWui, *style_table->get_table("menu").get(), &editboxstyles_);
	check_completeness(
	   "editboxes", editboxstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Dropdowns
	element_table = table->get_table("dropdowns");
	style_table = element_table->get_table("fsmenu");
	add_style(UI::PanelStyle::kFsMenu, *style_table->get_table("menu").get(), &dropdownstyles_);
	style_table = element_table->get_table("wui");
	add_style(UI::PanelStyle::kWui, *style_table->get_table("menu").get(), &dropdownstyles_);
	check_completeness(
	   "dropdowns", dropdownstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Scrollbars
	element_table = table->get_table("scrollbars");
	style_table = element_table->get_table("fsmenu");
	add_style(UI::PanelStyle::kFsMenu, *style_table->get_table("menu").get(), &scrollbarstyles_);
	style_table = element_table->get_table("wui");
	add_style(UI::PanelStyle::kWui, *style_table->get_table("menu").get(), &scrollbarstyles_);
	check_completeness(
	   "scrollbars", scrollbarstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));
}

// Return functions for the styles
const UI::PanelStyleInfo* StyleManager::button_style(UI::ButtonStyle style) const {
	assert(buttonstyles_.count(style) == 1);
	return buttonstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::slider_style(UI::SliderStyle style) const {
	assert(sliderstyles_.count(style) == 1);
	return sliderstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::tabpanel_style(UI::TabPanelStyle style) const {
	assert(tabpanelstyles_.count(style) == 1);
	return tabpanelstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::editbox_style(UI::PanelStyle style) const {
	assert(editboxstyles_.count(style) == 1);
	return editboxstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::dropdown_style(UI::PanelStyle style) const {
	assert(dropdownstyles_.count(style) == 1);
	return dropdownstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::scrollbar_style(UI::PanelStyle style) const {
	assert(scrollbarstyles_.count(style) == 1);
	return scrollbarstyles_.at(style).get();
}

// Fill the maps
void StyleManager::add_button_style(UI::ButtonStyle style, const LuaTable& table) {
	buttonstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_style(table))));
}

void StyleManager::add_slider_style(UI::SliderStyle style, const LuaTable& table) {
	sliderstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_style(table))));
}

void StyleManager::add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table) {
	tabpanelstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_style(table))));
}

void StyleManager::add_style(UI::PanelStyle style, const LuaTable& table, PanelStyleMap* map) {
	map->insert(std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_style(table))));
}
