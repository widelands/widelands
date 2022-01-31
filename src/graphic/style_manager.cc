/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#include "graphic/style_manager.h"

#include <memory>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_interface.h"

constexpr const char* const kDefaultTemplate = "templates/default/";
static std::string g_template_dir;
static std::map<std::string, std::unique_ptr<StyleManager>> g_style_managers;
StyleManager* g_style_manager(nullptr);  // points to an entry in `g_style_managers`

const std::string& template_dir() {
	return g_template_dir;
}
void set_template_dir(std::string dir) {
	if (dir.empty()) {
		// Empty string means "use default"
		dir = kDefaultTemplate;
	}

	if (dir.back() != '/') {
		dir += '/';
	}

	if (!g_fs->is_directory(dir)) {
		if (dir == kDefaultTemplate) {
			throw wexception("Default template directory '%s' does not exist!", dir.c_str());
		}
		log_warn("set_template_dir: template directory '%s' does not exist, using default template",
		         dir.c_str());
		dir = kDefaultTemplate;
	}

	if (g_template_dir == dir) {
		// nothing to do
		return;
	}

	g_template_dir = dir;

	auto it = g_style_managers.find(g_template_dir);
	if (it != g_style_managers.end()) {
		g_style_manager = it->second.get();
	} else {
		g_style_manager = new StyleManager();
		g_style_managers[g_template_dir] = std::unique_ptr<StyleManager>(g_style_manager);
	}
}

const Image& load_safe_template_image(const std::string& path) {
	try {
		return *g_image_cache->get(template_dir() + path);
	} catch (const ImageNotFound& error) {
		log_warn(
		   "Template image '%s' not found, using fallback image (%s)", path.c_str(), error.what());
		return *g_image_cache->get("images/novalue.png");
	}
}

namespace {
// Read RGB(A) color from LuaTable
RGBColor read_rgb_color(const LuaTable& table) {
	std::vector<int> rgbcolor = table.array_entries<int>();
	if (rgbcolor.size() != 3) {
		throw wexception("Expected 3 entries for RGB color, but got %" PRIuS ".", rgbcolor.size());
	}
	return RGBColor(rgbcolor[0], rgbcolor[1], rgbcolor[2]);
}
RGBAColor read_rgba_color(const LuaTable& table) {
	std::vector<int> rgbacolor = table.array_entries<int>();
	if (rgbacolor.size() != 4) {
		throw wexception("Expected 4 entries for RGBA color, but got %" PRIuS ".", rgbacolor.size());
	}
	return RGBAColor(rgbacolor[0], rgbacolor[1], rgbacolor[2], rgbacolor[3]);
}

// Read font style from LuaTable
UI::FontStyleInfo* read_font_style(const LuaTable& parent_table, const std::string& table_key) {
	std::unique_ptr<LuaTable> style_table = parent_table.get_table(table_key);
	const int size = style_table->get_int("size");
	if (size < 1) {
		throw wexception(
		   "Font size %d too small for %s, must be at least 1!", size, table_key.c_str());
	}
	return new UI::FontStyleInfo(
	   style_table->get_string("face"), read_rgb_color(*style_table->get_table("color")), size,
	   style_table->has_key<std::string>("bold") ? style_table->get_bool("bold") : false,
	   style_table->has_key<std::string>("italic") ? style_table->get_bool("italic") : false,
	   style_table->has_key<std::string>("underline") ? style_table->get_bool("underline") : false,
	   style_table->has_key<std::string>("shadow") ? style_table->get_bool("shadow") : false);
}

// Read image filename and RGBA color from LuaTable
UI::PanelStyleInfo* read_panel_style(const LuaTable& table) {
	const std::string image = table.get_string("image");
	std::vector<int> rgbcolor = table.get_table("color")->array_entries<int>();
	if (rgbcolor.size() != 3) {
		throw wexception("Expected 3 entries for RGB color, but got %" PRIuS ".", rgbcolor.size());
	}
	return new UI::PanelStyleInfo(
	   image.empty() ? nullptr : g_image_cache->get(image),
	   RGBAColor(rgbcolor[0], rgbcolor[1], rgbcolor[2], 0),
	   table.has_key<std::string>("margin") ? table.get_int("margin") : 0);
}

// Read text panel style from LuaTable
UI::TextPanelStyleInfo* read_text_panel_style(const LuaTable& table) {
	return new UI::TextPanelStyleInfo(
	   read_font_style(table, "font"), read_panel_style(*table.get_table("background")));
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

StyleManager::StyleManager() {
	ScopedTimer timer("Style Manager: Reading style templates took %ums", true);

	buttonstyles_.clear();
	sliderstyles_.clear();
	tabpanelstyles_.clear();
	editboxstyles_.clear();
	dropdownstyles_.clear();
	scrollbarstyles_.clear();

	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script(format("%1%init.lua", template_dir())));

	// Buttons
	std::unique_ptr<LuaTable> element_table = table->get_table("buttons");
	std::unique_ptr<LuaTable> style_table = element_table->get_table("fsmenu");
	add_button_style(UI::ButtonStyle::kFsMenuMenu, *style_table->get_table("menu"));
	add_button_style(UI::ButtonStyle::kFsMenuPrimary, *style_table->get_table("primary"));
	add_button_style(UI::ButtonStyle::kFsMenuSecondary, *style_table->get_table("secondary"));
	style_table = element_table->get_table("wui");
	add_button_style(UI::ButtonStyle::kWuiMenu, *style_table->get_table("menu"));
	add_button_style(UI::ButtonStyle::kWuiPrimary, *style_table->get_table("primary"));
	add_button_style(UI::ButtonStyle::kWuiSecondary, *style_table->get_table("secondary"));
	add_button_style(UI::ButtonStyle::kWuiBuildingStats, *style_table->get_table("building_stats"));
	check_completeness(
	   "buttons", buttonstyles_.size(), static_cast<size_t>(UI::ButtonStyle::kWuiBuildingStats));

	// Sliders
	element_table = table->get_table("sliders");
	style_table = element_table->get_table("fsmenu");
	add_slider_style(UI::SliderStyle::kFsMenu, *style_table->get_table("menu"));
	style_table = element_table->get_table("wui");
	add_slider_style(UI::SliderStyle::kWuiLight, *style_table->get_table("light"));
	add_slider_style(UI::SliderStyle::kWuiDark, *style_table->get_table("dark"));
	check_completeness(
	   "sliders", sliderstyles_.size(), static_cast<size_t>(UI::SliderStyle::kWuiDark));

	// Tabpanels
	element_table = table->get_table("tabpanels");
	style_table = element_table->get_table("fsmenu");
	add_tabpanel_style(UI::TabPanelStyle::kFsMenu, *style_table->get_table("menu"));
	style_table = element_table->get_table("wui");
	add_tabpanel_style(UI::TabPanelStyle::kWuiLight, *style_table->get_table("light"));
	add_tabpanel_style(UI::TabPanelStyle::kWuiDark, *style_table->get_table("dark"));
	check_completeness(
	   "tabpanels", tabpanelstyles_.size(), static_cast<size_t>(UI::TabPanelStyle::kWuiDark));

	// Editboxes
	element_table = table->get_table("editboxes");
	add_editbox_style(UI::PanelStyle::kFsMenu, *element_table->get_table("fsmenu"));
	add_editbox_style(UI::PanelStyle::kWui, *element_table->get_table("wui"));
	check_completeness(
	   "editboxes", editboxstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Dropdowns
	element_table = table->get_table("dropdowns");
	style_table = element_table->get_table("fsmenu");
	add_style(UI::PanelStyle::kFsMenu, *style_table->get_table("menu"), &dropdownstyles_);
	style_table = element_table->get_table("wui");
	add_style(UI::PanelStyle::kWui, *style_table->get_table("menu"), &dropdownstyles_);
	check_completeness(
	   "dropdowns", dropdownstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Scrollbars
	element_table = table->get_table("scrollbars");
	style_table = element_table->get_table("fsmenu");
	add_style(UI::PanelStyle::kFsMenu, *style_table->get_table("menu"), &scrollbarstyles_);
	style_table = element_table->get_table("wui");
	add_style(UI::PanelStyle::kWui, *style_table->get_table("menu"), &scrollbarstyles_);
	check_completeness(
	   "scrollbars", scrollbarstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Building statistics etc. for map objects
	set_building_statistics_style(*table->get_table("building_statistics"));

	// Progress bars
	element_table = table->get_table("progressbar");
	add_progressbar_style(UI::PanelStyle::kFsMenu, *element_table->get_table("fsmenu"));
	add_progressbar_style(UI::PanelStyle::kWui, *element_table->get_table("wui"));
	check_completeness(
	   "progressbars", progressbar_styles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Table and listselect
	element_table = table->get_table("tables");
	add_table_style(UI::PanelStyle::kFsMenu, *element_table->get_table("fsmenu"));
	add_table_style(UI::PanelStyle::kWui, *element_table->get_table("wui"));
	check_completeness("tables", table_styles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Windows
	element_table = table->get_table("windows");
	add_window_style(UI::WindowStyle::kFsMenu, *element_table->get_table("fsmenu"));
	add_window_style(UI::WindowStyle::kWui, *element_table->get_table("wui"));
	check_completeness("windows", window_styles_.size(), static_cast<size_t>(UI::WindowStyle::kWui));

	// Statistics plot
	set_statistics_plot_style(*table->get_table("statistics_plot"));

	// Ware info in warehouses, construction actions etc.
	element_table = table->get_table("wareinfo");
	add_ware_info_style(UI::WareInfoStyle::kNormal, *element_table->get_table("normal"));
	add_ware_info_style(UI::WareInfoStyle::kHighlight, *element_table->get_table("highlight"));
	check_completeness(
	   "wareinfos", ware_info_styles_.size(), static_cast<size_t>(UI::WareInfoStyle::kHighlight));

	// Special elements
	minimum_font_size_ = table->get_int("minimum_font_size");
	if (minimum_font_size_ < 1) {
		throw wexception("Font size too small for minimum_font_size, must be at least 1!");
	}
	minimap_icon_frame_ = read_rgb_color(*table->get_table("minimap_icon_frame"));
	focused_color_ = read_rgba_color(*table->get_table("background_focused"));
	semi_focused_color_ = read_rgba_color(*table->get_table("background_semi_focused"));
	focus_border_thickness_ = table->get_int("focus_border_thickness");
	if (focus_border_thickness_ < 1) {
		throw wexception("focus_border_thickness must be at least 1");
	}

	// Fonts
	element_table = table->get_table("fonts");
	add_font_style(UI::FontStyle::kChatMessage, *element_table, "chat_message");
	add_font_style(UI::FontStyle::kChatPlayername, *element_table, "chat_playername");
	add_font_style(UI::FontStyle::kChatServer, *element_table, "chat_server");
	add_font_style(UI::FontStyle::kChatTimestamp, *element_table, "chat_timestamp");
	add_font_style(UI::FontStyle::kChatWhisper, *element_table, "chat_whisper");
	add_font_style(UI::FontStyle::kItalic, *element_table, "italic");
	add_font_style(
	   UI::FontStyle::kFsGameSetupHeadings, *element_table, "fsmenu_game_setup_headings");
	add_font_style(
	   UI::FontStyle::kFsGameSetupSuperuser, *element_table, "fsmenu_game_setup_superuser");
	add_font_style(
	   UI::FontStyle::kFsGameSetupIrcClient, *element_table, "fsmenu_game_setup_irc_client");
	add_font_style(UI::FontStyle::kFsGameSetupMapname, *element_table, "fsmenu_game_setup_mapname");
	add_font_style(UI::FontStyle::kFsMenuGameTip, *element_table, "fsmenu_gametip");
	add_font_style(
	   UI::FontStyle::kFsMenuInfoPanelHeading, *element_table, "fsmenu_info_panel_heading");
	add_font_style(
	   UI::FontStyle::kFsMenuInfoPanelParagraph, *element_table, "fsmenu_info_panel_paragraph");
	add_font_style(UI::FontStyle::kFsMenuIntro, *element_table, "fsmenu_intro");
	add_font_style(UI::FontStyle::kGameSummaryTitle, *element_table, "game_summary_title");
	add_font_style(UI::FontStyle::kFsMenuTranslationInfo, *element_table, "fsmenu_translation_info");
	add_font_style(UI::FontStyle::kDisabled, *element_table, "disabled");
	add_font_style(UI::FontStyle::kFsMenuLabel, *element_table, "label_fs");
	add_font_style(UI::FontStyle::kWuiLabel, *element_table, "label_wui");
	add_font_style(UI::FontStyle::kWuiTooltipHeader, *element_table, "tooltip_header_wui");
	add_font_style(UI::FontStyle::kWuiTooltipHotkey, *element_table, "tooltip_hotkey_wui");
	add_font_style(UI::FontStyle::kWuiTooltip, *element_table, "tooltip_wui");
	add_font_style(UI::FontStyle::kFsTooltipHeader, *element_table, "tooltip_header_fs");
	add_font_style(UI::FontStyle::kFsTooltipHotkey, *element_table, "tooltip_hotkey_fs");
	add_font_style(UI::FontStyle::kFsTooltip, *element_table, "tooltip_fs");
	add_font_style(UI::FontStyle::kWarning, *element_table, "warning");
	add_font_style(
	   UI::FontStyle::kWuiAttackBoxSliderLabel, *element_table, "wui_attack_box_slider_label");
	add_font_style(
	   UI::FontStyle::kWuiGameSpeedAndCoordinates, *element_table, "wui_game_speed_and_coordinates");
	add_font_style(UI::FontStyle::kWuiInfoPanelHeading, *element_table, "wui_info_panel_heading");
	add_font_style(
	   UI::FontStyle::kWuiInfoPanelParagraph, *element_table, "wui_info_panel_paragraph");
	add_font_style(UI::FontStyle::kWuiMessageHeading, *element_table, "wui_message_heading");
	add_font_style(UI::FontStyle::kWuiMessageParagraph, *element_table, "wui_message_paragraph");
	add_font_style(UI::FontStyle::kFsMenuWindowTitle, *element_table, "fs_window_title");
	add_font_style(UI::FontStyle::kWuiWindowTitle, *element_table, "wui_window_title");
	check_completeness(
	   "fonts", fontstyles_.size(), static_cast<size_t>(UI::FontStyle::kWuiWindowTitle));
}

// Return functions for the styles
const UI::ButtonStyleInfo& StyleManager::button_style(UI::ButtonStyle style) const {
	assert(buttonstyles_.count(style) == 1);
	return *buttonstyles_.at(style);
}

const UI::TextPanelStyleInfo& StyleManager::slider_style(UI::SliderStyle style) const {
	assert(sliderstyles_.count(style) == 1);
	return *sliderstyles_.at(style);
}

const UI::PanelStyleInfo* StyleManager::tabpanel_style(UI::TabPanelStyle style) const {
	assert(tabpanelstyles_.count(style) == 1);
	return tabpanelstyles_.at(style).get();
}

const UI::TextPanelStyleInfo& StyleManager::editbox_style(UI::PanelStyle style) const {
	assert(editboxstyles_.count(style) == 1);
	return *editboxstyles_.at(style);
}

const UI::PanelStyleInfo* StyleManager::dropdown_style(UI::PanelStyle style) const {
	assert(dropdownstyles_.count(style) == 1);
	return dropdownstyles_.at(style).get();
}

const UI::PanelStyleInfo* StyleManager::scrollbar_style(UI::PanelStyle style) const {
	assert(scrollbarstyles_.count(style) == 1);
	return scrollbarstyles_.at(style).get();
}

const UI::BuildingStatisticsStyleInfo& StyleManager::building_statistics_style() const {
	return *building_statistics_style_;
}

const UI::ProgressbarStyleInfo& StyleManager::progressbar_style(UI::PanelStyle style) const {
	assert(progressbar_styles_.count(style) == 1);
	return *progressbar_styles_.at(style);
}

const UI::StatisticsPlotStyleInfo& StyleManager::statistics_plot_style() const {
	return *statistics_plot_style_;
}

const UI::TableStyleInfo& StyleManager::table_style(UI::PanelStyle style) const {
	assert(table_styles_.count(style) == 1);
	return *table_styles_.at(style);
}

const UI::WareInfoStyleInfo& StyleManager::ware_info_style(UI::WareInfoStyle style) const {
	assert(ware_info_styles_.count(style) == 1);
	return *ware_info_styles_.at(style);
}

const UI::WindowStyleInfo& StyleManager::window_style(UI::WindowStyle style) const {
	assert(window_styles_.count(style) == 1);
	return *window_styles_.at(style);
}

const UI::FontStyleInfo& StyleManager::font_style(UI::FontStyle style) const {
	assert(fontstyles_.count(style) == 1);
	return *fontstyles_.at(style);
}

int StyleManager::minimum_font_size() const {
	return minimum_font_size_;
}

const RGBColor& StyleManager::minimap_icon_frame() const {
	return minimap_icon_frame_;
}

std::string StyleManager::color_tag(const std::string& text, const RGBColor& color) {
	return format("<font color=%s>%s</font>", color.hex_value(), text);
}

// Fill the maps
void StyleManager::add_button_style(UI::ButtonStyle style, const LuaTable& table) {
	buttonstyles_.insert(
	   std::make_pair(style, std::unique_ptr<const UI::ButtonStyleInfo>(new UI::ButtonStyleInfo(
	                            read_text_panel_style(*table.get_table("enabled")),
	                            read_text_panel_style(*table.get_table("disabled"))))));
}

void StyleManager::add_slider_style(UI::SliderStyle style, const LuaTable& table) {
	sliderstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::TextPanelStyleInfo>(read_text_panel_style(table))));
}

void StyleManager::add_editbox_style(UI::PanelStyle style, const LuaTable& table) {
	editboxstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::TextPanelStyleInfo>(read_text_panel_style(table))));
}

void StyleManager::add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table) {
	tabpanelstyles_.insert(
	   std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_panel_style(table))));
}

void StyleManager::add_progressbar_style(UI::PanelStyle style, const LuaTable& table) {
	std::unique_ptr<LuaTable> color_table = table.get_table("background_colors");
	progressbar_styles_.insert(std::make_pair(
	   style, std::unique_ptr<const UI::ProgressbarStyleInfo>(new UI::ProgressbarStyleInfo(
	             read_font_style(table, "font"), read_rgb_color(*color_table->get_table("low")),
	             read_rgb_color(*color_table->get_table("medium")),
	             read_rgb_color(*color_table->get_table("high"))))));
}

void StyleManager::add_table_style(UI::PanelStyle style, const LuaTable& table) {
	table_styles_.insert(std::make_pair(
	   style, std::unique_ptr<const UI::TableStyleInfo>(new UI::TableStyleInfo(
	             read_font_style(table, "enabled"), read_font_style(table, "disabled"),
	             read_font_style(table, "hotkey")))));
}

void StyleManager::set_statistics_plot_style(const LuaTable& table) {
	std::unique_ptr<LuaTable> fonts_table = table.get_table("fonts");
	std::unique_ptr<LuaTable> colors_table = table.get_table("colors");
	statistics_plot_style_.reset(new UI::StatisticsPlotStyleInfo(
	   read_font_style(*fonts_table, "x_tick"), read_font_style(*fonts_table, "y_min_value"),
	   read_font_style(*fonts_table, "y_max_value"),
	   read_rgb_color(*colors_table->get_table("axis_line")),
	   read_rgb_color(*colors_table->get_table("zero_line"))));
}

void StyleManager::set_building_statistics_style(const LuaTable& table) {
	std::unique_ptr<LuaTable> window_table = table.get_table("statistics_window");
	std::unique_ptr<LuaTable> colors_table = table.get_table("colors");
	std::unique_ptr<LuaTable> fonts_table = window_table->get_table("fonts");
	building_statistics_style_.reset(new UI::BuildingStatisticsStyleInfo(
	   read_font_style(*fonts_table, "button_font"), read_font_style(*fonts_table, "details_font"),
	   window_table->get_int("editbox_margin"), read_font_style(table, "census_font"),
	   read_font_style(table, "statistics_font"),
	   read_rgb_color(*colors_table->get_table("construction")),
	   read_rgb_color(*colors_table->get_table("neutral")),
	   read_rgb_color(*colors_table->get_table("low")),
	   read_rgb_color(*colors_table->get_table("medium")),
	   read_rgb_color(*colors_table->get_table("high")),
	   read_rgb_color(*colors_table->get_table("low_alt")),
	   read_rgb_color(*colors_table->get_table("medium_alt")),
	   read_rgb_color(*colors_table->get_table("high_alt"))));
}

void StyleManager::add_ware_info_style(UI::WareInfoStyle style, const LuaTable& table) {
	std::unique_ptr<LuaTable> fonts_table = table.get_table("fonts");
	std::unique_ptr<LuaTable> colors_table = table.get_table("colors");
	ware_info_styles_.insert(std::make_pair(
	   style, std::unique_ptr<const UI::WareInfoStyleInfo>(new UI::WareInfoStyleInfo(
	             read_font_style(*fonts_table, "header"), read_font_style(*fonts_table, "info"),
	             g_image_cache->get(table.get_string("icon_background_image")),
	             read_rgb_color(*colors_table->get_table("icon_frame")),
	             read_rgb_color(*colors_table->get_table("icon_background")),
	             read_rgb_color(*colors_table->get_table("info_background"))))));
}

void StyleManager::add_window_style(UI::WindowStyle style, const LuaTable& table) {
	window_styles_.insert(std::make_pair(
	   style, std::unique_ptr<const UI::WindowStyleInfo>(new UI::WindowStyleInfo(
	             read_rgba_color(*table.get_table("window_border_focused")),
	             read_rgba_color(*table.get_table("window_border_unfocused")),
	             g_image_cache->get(table.get_string("border_top")),
	             g_image_cache->get(table.get_string("border_bottom")),
	             g_image_cache->get(table.get_string("border_right")),
	             g_image_cache->get(table.get_string("border_left")),
	             g_image_cache->get(table.get_string("background")), table.get_string("button_pin"),
	             table.get_string("button_unpin"), table.get_string("button_minimize"),
	             table.get_string("button_unminimize"), table.get_string("button_close")))));
}

void StyleManager::add_style(UI::PanelStyle style, const LuaTable& table, PanelStyleMap* map) {
	map->insert(std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(read_panel_style(table))));
}

void StyleManager::add_font_style(UI::FontStyle font_key,
                                  const LuaTable& table,
                                  const std::string& table_key) {
	fontstyles_.emplace(std::make_pair(
	   font_key, std::unique_ptr<UI::FontStyleInfo>(read_font_style(table, table_key))));
}
