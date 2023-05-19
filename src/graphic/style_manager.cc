/*
 * Copyright (C) 2017-2023 by the Widelands Development Team
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

#include <cassert>
#include <memory>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_interface.h"

const std::string kDefaultTemplate("templates/default/");
static std::string g_template_dir;
static std::map<std::string, std::unique_ptr<StyleManager>> g_style_managers;
StyleManager* g_style_manager(nullptr);       // points to an entry in `g_style_managers`
static StyleManager* default_style(nullptr);  // points to the default style in `g_style_managers`

const std::string& template_dir() {
	return g_template_dir;
}
void set_template_dir(std::string dir) {
	if (dir.empty()) {
		// Empty string means "use default"
		dir = kDefaultTemplate;
	} else if (default_style == nullptr) {
		// The default template must be loaded first to provide fallback values if needed
		set_template_dir("");
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
		if (default_style == nullptr) {
			assert(g_template_dir == kDefaultTemplate);
			default_style = g_style_manager;
		}
	}
}

std::string resolve_template_image_filename(const std::string& path) {
	if (starts_with(path, "map:")) {
		/* Skip the lookup for map-specific images. */
		return path;
	}

	/* Check if the current theme provides an override path. */
	std::string override_path = template_dir() + path;
	if (g_fs->file_exists(override_path)) {
		return override_path;
	}

	/* Check if the default theme provides an alternative path. */
	if (!is_using_default_theme()) {
		override_path = kDefaultTemplate + path;
		if (g_fs->file_exists(override_path)) {
			return override_path;
		}
	}

	/* If it's a regular image path, use that. */
	if (g_fs->file_exists(path)) {
		return path;
	}

	/* If all else fails (e.g. a missing template sprite): Default image. */
	log_warn("Template image '%s' not found, using fallback image", path.c_str());
	return "images/novalue.png";
}

namespace {

// Shorthand helper function to make sure falling back to the default style is possible,
// or raise an exception if it is not
void fail_if_doing_default_style(std::string style_type, std::string style_name) {
	if (default_style == nullptr) {
		throw wexception(
		   "The default template does not have %s '%s'", style_type.c_str(), style_name.c_str());
	}
	assert(g_template_dir != kDefaultTemplate);
}

// Get the subtable for 'section' safely from 'table', allowing fallback if it is missing
std::unique_ptr<LuaTable> try_section_or_empty(LuaInterface& lua,
                                               const LuaTable& table,
                                               std::string section,
                                               std::string parent = "") {
	if (table.has_key(section)) {
		return table.get_table(section);
	}

	std::string name = parent.empty() ? section : format("%s.%s", parent, section);
	fail_if_doing_default_style("section", name);
	return lua.empty_table();
}

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
	return new UI::FontStyleInfo(style_table->get_string("face"),
	                             read_rgb_color(*style_table->get_table("color")), size,
	                             style_table->get_bool_with_default("bold", false),
	                             style_table->get_bool_with_default("italic", false),
	                             style_table->get_bool_with_default("underline", false),
	                             style_table->get_bool_with_default("shadow", false));
}

// Read paragraph style from LuaTable
UI::ParagraphStyleInfo* read_paragraph_style(const LuaTable& parent_table,
                                             const std::string& table_key) {
	std::unique_ptr<LuaTable> style_table = parent_table.get_table(table_key);
	return new UI::ParagraphStyleInfo(read_font_style(*style_table, "font"),
	                                  get_string_with_default(*style_table, "align", ""),
	                                  get_string_with_default(*style_table, "valign", ""),
	                                  style_table->get_int_with_default("indent", 0),
	                                  style_table->get_int_with_default("spacing", 0),
	                                  style_table->get_int_with_default("space_before", 0),
	                                  style_table->get_int_with_default("space_after", 0));
}

// Read image filename and RGBA color from LuaTable
UI::PanelStyleInfo* read_panel_style(const LuaTable& table) {
	const std::string image = table.get_string("image");
	std::vector<int> rgbcolor = table.get_table("color")->array_entries<int>();
	if (rgbcolor.size() != 3) {
		throw wexception("Expected 3 entries for RGB color, but got %" PRIuS ".", rgbcolor.size());
	}
	return new UI::PanelStyleInfo(image.empty() ? nullptr : g_image_cache->get(image),
	                              RGBAColor(rgbcolor[0], rgbcolor[1], rgbcolor[2], 0),
	                              table.get_int_with_default("margin", 0));
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

	std::string section;
	std::string sub;
	std::unique_ptr<LuaTable> element_table;
	std::unique_ptr<LuaTable> style_table;

	// Buttons
	section = "buttons";
	element_table = try_section_or_empty(lua, *table, section);
	sub = "fsmenu";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_button_style(UI::ButtonStyle::kFsMenuMenu, *style_table, sub, "menu");
	add_button_style(UI::ButtonStyle::kFsMenuPrimary, *style_table, sub, "primary");
	add_button_style(UI::ButtonStyle::kFsMenuSecondary, *style_table, sub, "secondary");
	sub = "wui";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_button_style(UI::ButtonStyle::kWuiMenu, *style_table, sub, "menu");
	add_button_style(UI::ButtonStyle::kWuiPrimary, *style_table, sub, "primary");
	add_button_style(UI::ButtonStyle::kWuiSecondary, *style_table, sub, "secondary");
	add_button_style(UI::ButtonStyle::kWuiBuildingStats, *style_table, sub, "building_stats");
	check_completeness(
	   "buttons", buttonstyles_.size(), static_cast<size_t>(UI::ButtonStyle::kWuiBuildingStats));

	// Sliders
	section = "sliders";
	element_table = try_section_or_empty(lua, *table, section);
	sub = "fsmenu";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_slider_style(UI::SliderStyle::kFsMenu, *style_table, sub, "menu");
	sub = "wui";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_slider_style(UI::SliderStyle::kWuiLight, *style_table, sub, "light");
	add_slider_style(UI::SliderStyle::kWuiDark, *style_table, sub, "dark");
	check_completeness(
	   "sliders", sliderstyles_.size(), static_cast<size_t>(UI::SliderStyle::kWuiDark));

	// Tabpanels
	section = "tabpanels";
	element_table = try_section_or_empty(lua, *table, section);
	sub = "fsmenu";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_tabpanel_style(UI::TabPanelStyle::kFsMenu, *style_table, sub, "menu");
	sub = "wui";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_tabpanel_style(UI::TabPanelStyle::kWuiLight, *style_table, sub, "light");
	add_tabpanel_style(UI::TabPanelStyle::kWuiDark, *style_table, sub, "dark");
	check_completeness(
	   "tabpanels", tabpanelstyles_.size(), static_cast<size_t>(UI::TabPanelStyle::kWuiDark));

	// Editboxes
	section = "editboxes";
	element_table = try_section_or_empty(lua, *table, section);
	add_editbox_style(UI::PanelStyle::kFsMenu, *element_table, "fsmenu");
	add_editbox_style(UI::PanelStyle::kWui, *element_table, "wui");
	check_completeness(
	   "editboxes", editboxstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Dropdowns
	section = "dropdowns";
	element_table = try_section_or_empty(lua, *table, section);
	sub = "fsmenu";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_dropdown_style(UI::PanelStyle::kFsMenu, *style_table, sub, "menu");
	sub = "wui";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_dropdown_style(UI::PanelStyle::kWui, *style_table, sub, "menu");
	check_completeness(
	   "dropdowns", dropdownstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Scrollbars
	section = "scrollbars";
	element_table = try_section_or_empty(lua, *table, section);
	sub = "fsmenu";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_scrollbar_style(UI::PanelStyle::kFsMenu, *style_table, sub, "menu");
	sub = "wui";
	style_table = try_section_or_empty(lua, *element_table, sub, section);
	add_scrollbar_style(UI::PanelStyle::kWui, *style_table, sub, "menu");
	check_completeness(
	   "scrollbars", scrollbarstyles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Building statistics etc. for map objects
	element_table = try_section_or_empty(lua, *table, "building_statistics");
	set_building_statistics_style(*element_table);

	// Progress bars
	section = "progressbar";
	element_table = try_section_or_empty(lua, *table, section);
	add_progressbar_style(UI::PanelStyle::kFsMenu, *element_table, "fsmenu");
	add_progressbar_style(UI::PanelStyle::kWui, *element_table, "wui");
	check_completeness(
	   "progressbars", progressbar_styles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Table and listselect
	section = "tables";
	element_table = try_section_or_empty(lua, *table, section);
	add_table_style(UI::PanelStyle::kFsMenu, *element_table, "fsmenu");
	add_table_style(UI::PanelStyle::kWui, *element_table, "wui");
	check_completeness("tables", table_styles_.size(), static_cast<size_t>(UI::PanelStyle::kWui));

	// Windows
	section = "windows";
	element_table = try_section_or_empty(lua, *table, section);
	add_window_style(UI::WindowStyle::kFsMenu, *element_table, "fsmenu");
	add_window_style(UI::WindowStyle::kWui, *element_table, "wui");
	check_completeness("windows", window_styles_.size(), static_cast<size_t>(UI::WindowStyle::kWui));

	// Statistics plot
	section = "statistics_plot";
	if (table->has_key(section)) {
		set_statistics_plot_style(*table->get_table(section));
	} else {
		fail_if_doing_default_style("section", section);
		const UI::StatisticsPlotStyleInfo& fallback = default_style->statistics_plot_style();
		statistics_plot_style_.reset(
		   new UI::StatisticsPlotStyleInfo(new UI::FontStyleInfo(fallback.x_tick_font()),
		                                   new UI::FontStyleInfo(fallback.y_min_value_font()),
		                                   new UI::FontStyleInfo(fallback.y_max_value_font()),
		                                   fallback.axis_line_color(), fallback.zero_line_color()));
	}

	// Ware info in warehouses, construction actions etc.
	section = "wareinfo";
	element_table = try_section_or_empty(lua, *table, section);
	add_ware_info_style(UI::WareInfoStyle::kNormal, *element_table, "normal");
	add_ware_info_style(UI::WareInfoStyle::kHighlight, *element_table, "highlight");
	check_completeness(
	   "wareinfos", ware_info_styles_.size(), static_cast<size_t>(UI::WareInfoStyle::kHighlight));

	// Special elements
	std::string key;

	key = "minimum_font_size";
	if (table->has_key(key)) {
		minimum_font_size_ = table->get_int(key);
		if (minimum_font_size_ < 1) {
			throw wexception("Font size too small for minimum_font_size, must be at least 1!");
		}
	} else {
		fail_if_doing_default_style("entry", key);
		minimum_font_size_ = default_style->minimum_font_size();
	}

	key = "minimap_icon_frame";
	if (table->has_key(key)) {
		minimap_icon_frame_ = read_rgb_color(*table->get_table(key));
	} else {
		fail_if_doing_default_style("entry", key);
		minimap_icon_frame_ = default_style->minimap_icon_frame();
	}

	key = "background_focused";
	if (table->has_key(key)) {
		focused_color_ = read_rgba_color(*table->get_table(key));
	} else {
		fail_if_doing_default_style("entry", key);
		focused_color_ = default_style->focused_color();
	}

	key = "background_semi_focused";
	if (table->has_key(key)) {
		semi_focused_color_ = read_rgba_color(*table->get_table(key));
	} else {
		fail_if_doing_default_style("entry", key);
		semi_focused_color_ = default_style->semi_focused_color();
	}

	key = "focus_border_thickness";
	if (table->has_key(key)) {
		focus_border_thickness_ = table->get_int(key);
		if (focus_border_thickness_ < 1) {
			throw wexception("focus_border_thickness must be at least 1");
		}
	} else {
		fail_if_doing_default_style("entry", key);
		focus_border_thickness_ = default_style->focus_border_thickness();
	}

	// Fonts
	section = "fonts";
	element_table = try_section_or_empty(lua, *table, section);
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
	add_font_style(UI::FontStyle::kUnknown, *element_table, "unknown");
	check_completeness("fonts", fontstyles_.size(), static_cast<size_t>(UI::FontStyle::kUnknown));

	// Paragraphs
	section = "paragraphs";
	element_table = try_section_or_empty(lua, *table, section);
	add_paragraph_style(UI::ParagraphStyle::kReadmeTitle, *element_table, "readme_title");
	add_paragraph_style(UI::ParagraphStyle::kAboutTitle, *element_table, "about_title");
	add_paragraph_style(UI::ParagraphStyle::kAboutSubtitle, *element_table, "about_subtitle");
	add_paragraph_style(UI::ParagraphStyle::kAuthorsHeading1, *element_table, "authors_heading_1");
	add_paragraph_style(UI::ParagraphStyle::kFsHeading1, *element_table, "fs_heading_1");
	add_paragraph_style(UI::ParagraphStyle::kFsHeading2, *element_table, "fs_heading_2");
	add_paragraph_style(UI::ParagraphStyle::kFsHeading3, *element_table, "fs_heading_3");
	add_paragraph_style(UI::ParagraphStyle::kFsHeading4, *element_table, "fs_heading_4");
	add_paragraph_style(UI::ParagraphStyle::kFsText, *element_table, "fs_text");
	add_paragraph_style(
	   UI::ParagraphStyle::kWuiObjectivesHeading, *element_table, "wui_objectives_heading");
	add_paragraph_style(UI::ParagraphStyle::kWuiHeading1, *element_table, "wui_heading_1");
	add_paragraph_style(UI::ParagraphStyle::kWuiHeading2, *element_table, "wui_heading_2");
	add_paragraph_style(UI::ParagraphStyle::kWuiHeading3, *element_table, "wui_heading_3");
	add_paragraph_style(UI::ParagraphStyle::kWuiHeading4, *element_table, "wui_heading_4");
	add_paragraph_style(UI::ParagraphStyle::kWuiText, *element_table, "wui_text");
	add_paragraph_style(UI::ParagraphStyle::kWuiImageLine, *element_table, "wui_image_line");
	add_paragraph_style(UI::ParagraphStyle::kWuiLoreAuthor, *element_table, "wui_lore_author");
	add_paragraph_style(UI::ParagraphStyle::kUnknown, *element_table, "unknown");
	check_completeness(
	   "paragraphs", paragraphstyles_.size(), static_cast<size_t>(UI::ParagraphStyle::kUnknown));

	// Colors
	section = "colors";
	element_table = try_section_or_empty(lua, *table, section);
	add_color(UI::ColorStyle::kCampaignBarbarianThron, *element_table, "campaign_bar_thron");
	add_color(UI::ColorStyle::kCampaignBarbarianBoldreth, *element_table, "campaign_bar_boldreth");
	add_color(UI::ColorStyle::kCampaignBarbarianKhantrukh, *element_table, "campaign_bar_khantrukh");
	add_color(UI::ColorStyle::kCampaignEmpireLutius, *element_table, "campaign_emp_lutius");
	add_color(UI::ColorStyle::kCampaignEmpireAmalea, *element_table, "campaign_emp_amalea");
	add_color(UI::ColorStyle::kCampaignEmpireSaledus, *element_table, "campaign_emp_saledus");
	add_color(UI::ColorStyle::kCampaignEmpireMarcus, *element_table, "campaign_emp_marcus");
	add_color(UI::ColorStyle::kCampaignEmpireJulia, *element_table, "campaign_emp_julia");
	add_color(UI::ColorStyle::kCampaignAtlanteanJundlina, *element_table, "campaign_atl_jundlina");
	add_color(UI::ColorStyle::kCampaignAtlanteanSidolus, *element_table, "campaign_atl_sidolus");
	add_color(UI::ColorStyle::kCampaignAtlanteanLoftomor, *element_table, "campaign_atl_loftomor");
	add_color(UI::ColorStyle::kCampaignAtlanteanColionder, *element_table, "campaign_atl_colionder");
	add_color(UI::ColorStyle::kCampaignAtlanteanOpol, *element_table, "campaign_atl_opol");
	add_color(UI::ColorStyle::kCampaignAtlanteanOstur, *element_table, "campaign_atl_ostur");
	add_color(UI::ColorStyle::kCampaignAtlanteanKalitath, *element_table, "campaign_atl_kalitath");
	add_color(UI::ColorStyle::kCampaignFrisianReebaud, *element_table, "campaign_fri_reebaud");
	add_color(UI::ColorStyle::kCampaignFrisianHauke, *element_table, "campaign_fri_hauke");
	add_color(UI::ColorStyle::kCampaignFrisianMaukor, *element_table, "campaign_fri_maukor");
	add_color(UI::ColorStyle::kCampaignFrisianMurilius, *element_table, "campaign_fri_murilius");
	add_color(UI::ColorStyle::kCampaignFrisianClaus, *element_table, "campaign_fri_claus");
	add_color(UI::ColorStyle::kCampaignFrisianHenneke, *element_table, "campaign_fri_henneke");
	add_color(UI::ColorStyle::kCampaignFrisianIniucundus, *element_table, "campaign_fri_iniucundus");
	add_color(UI::ColorStyle::kCampaignFrisianAngadthur, *element_table, "campaign_fri_angadthur");
	add_color(UI::ColorStyle::kCampaignFrisianAmazon, *element_table, "campaign_fri_amazon");
	add_color(UI::ColorStyle::kCampaignFrisianKetelsen, *element_table, "campaign_fri_ketelsen");
	add_color(UI::ColorStyle::kSPScenarioRiverAdvisor, *element_table, "map_river_advisor");
	add_color(UI::ColorStyle::kUnknown, *element_table, "unknown");
	check_completeness("colors", colors_.size(), static_cast<size_t>(UI::ColorStyle::kUnknown));

	// Sizes
	section = "styled_sizes";
	element_table = try_section_or_empty(lua, *table, section);
	add_styled_size(UI::StyledSize::kFsTextDefaultGap, *element_table, "fs_text_default_gap");
	add_styled_size(UI::StyledSize::kFsTextSpaceBeforeInlineHeader, *element_table,
	                "fs_text_space_before_inline_header");
	add_styled_size(
	   UI::StyledSize::kFsMainMenuDropdownHeight, *element_table, "fs_main_menu_dropdown_height");
	add_styled_size(UI::StyledSize::kWuiTextDefaultGap, *element_table, "wui_text_default_gap");
	add_styled_size(UI::StyledSize::kWuiTextSpaceBeforeInlineHeader, *element_table,
	                "wui_text_space_before_inline_header");
	add_styled_size(UI::StyledSize::kWuiSpaceBeforeImmovableIcon, *element_table,
	                "wui_space_before_immovable_icon");
	add_styled_size(
	   UI::StyledSize::kWinConditionMessageGap, *element_table, "win_condition_message_gap");
	add_styled_size(UI::StyledSize::kHelpTerrainTreeHeaderSpaceBefore, *element_table,
	                "help_terrain_tree_header_space_before");
	add_styled_size(UI::StyledSize::kHelpTerrainTreeHeaderSpaceAfter, *element_table,
	                "help_terrain_tree_header_space_after");
	add_styled_size(
	   UI::StyledSize::kEditorTooltipIconGap, *element_table, "editor_tooltip_icon_gap");
	add_styled_size(UI::StyledSize::kCampaignMessageBoxDefaultH, *element_table,
	                "campaign_message_box_default_h");
	add_styled_size(UI::StyledSize::kCampaignMessageBoxDefaultW, *element_table,
	                "campaign_message_box_default_w");
	add_styled_size(
	   UI::StyledSize::kCampaignMessageBoxTopPosY, *element_table, "campaign_message_box_top_pos_y");
	add_styled_size(UI::StyledSize::kCampaignMessageBoxSizeStep, *element_table,
	                "campaign_message_box_size_step");
	add_styled_size(
	   UI::StyledSize::kCampaignMessageBoxMinH, *element_table, "campaign_message_box_h_min");
	add_styled_size(
	   UI::StyledSize::kCampaignMessageBoxMaxH, *element_table, "campaign_message_box_h_max");
	add_styled_size(
	   UI::StyledSize::kCampaignMessageBoxMinW, *element_table, "campaign_message_box_w_min");
	add_styled_size(
	   UI::StyledSize::kCampaignMessageBoxMaxW, *element_table, "campaign_message_box_w_max");
	add_styled_size(
	   UI::StyledSize::kCampaignFri02PoemIndent, *element_table, "campaign_fri02_poem_indent");
	add_styled_size(UI::StyledSize::kSPScenarioPlateauMessageBoxPosY, *element_table,
	                "map_plateau_message_pos_y");
	add_styled_size(UI::StyledSize::kUIDefaultPadding, *element_table, "ui_default_padding");
	add_styled_size(UI::StyledSize::kToolbarButtonSize, *element_table, "toolbar_button_size");
	check_completeness("styled_sizes", styled_sizes_.size(),
	                   static_cast<size_t>(UI::StyledSize::kToolbarButtonSize));
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

const UI::FontStyleInfo& StyleManager::font_style(std::string style_name) const {
	if (fontstyle_keys_.count(style_name) != 1) {
		log_warn("Undefined font style requested: %s", style_name.c_str());
		return *fontstyles_.at(UI::FontStyle::kUnknown);
	}
	return font_style(fontstyle_keys_.at(style_name));
}

const UI::ParagraphStyleInfo& StyleManager::paragraph_style(UI::ParagraphStyle style) const {
	assert(paragraphstyles_.count(style) == 1);
	return *paragraphstyles_.at(style);
}

const UI::ParagraphStyleInfo& StyleManager::paragraph_style(std::string style_name) const {
	if (paragraphstyle_keys_.count(style_name) != 1) {
		log_warn("Undefined paragraph style requested: %s", style_name.c_str());
		return *paragraphstyles_.at(UI::ParagraphStyle::kUnknown);
	}
	return paragraph_style(paragraphstyle_keys_.at(style_name));
}

const RGBColor& StyleManager::color(UI::ColorStyle id) const {
	assert(colors_.count(id) == 1);
	return colors_.at(id);
}

const RGBColor& StyleManager::color(std::string name) const {
	if (color_keys_.count(name) != 1) {
		log_warn("Undefined color style requested: %s", name.c_str());
		return colors_.at(UI::ColorStyle::kUnknown);
	}
	return color(color_keys_.at(name));
}

int StyleManager::styled_size(UI::StyledSize id) const {
	assert(styled_sizes_.count(id) == 1);
	return styled_sizes_.at(id);
}

int StyleManager::styled_size(std::string name) const {
	if (styled_size_keys_.count(name) != 1) {
		log_warn("Undefined styled size requested: %s", name.c_str());
		return 0;
	}
	return styled_size(styled_size_keys_.at(name));
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
void StyleManager::add_button_style(UI::ButtonStyle style,
                                    const LuaTable& table,
                                    const std::string& parent,
                                    const std::string& key) {
	UI::ButtonStyleInfo* b_style;
	if (table.has_key(key)) {
		std::unique_ptr<LuaTable> bst = table.get_table(key);
		b_style = new UI::ButtonStyleInfo(read_text_panel_style(*(bst->get_table("enabled"))),
		                                  read_text_panel_style(*(bst->get_table("disabled"))));
	} else {
		fail_if_doing_default_style("button style", format("%s.%s", parent, key));
		b_style = new UI::ButtonStyleInfo(default_style->button_style(style));
	}
	buttonstyles_.insert(std::make_pair(style, std::unique_ptr<const UI::ButtonStyleInfo>(b_style)));
}

void StyleManager::add_slider_style(UI::SliderStyle style,
                                    const LuaTable& table,
                                    const std::string& parent,
                                    const std::string& key) {
	UI::TextPanelStyleInfo* s_style;
	if (table.has_key(key)) {
		s_style = read_text_panel_style(*table.get_table(key));
	} else {
		fail_if_doing_default_style("slider style", format("%s.%s", parent, key));
		s_style = new UI::TextPanelStyleInfo(default_style->slider_style(style));
	}
	sliderstyles_.insert(std::make_pair(style, std::unique_ptr<UI::TextPanelStyleInfo>(s_style)));
}

void StyleManager::add_editbox_style(UI::PanelStyle style,
                                     const LuaTable& table,
                                     const std::string& key) {
	UI::TextPanelStyleInfo* eb_style;
	if (table.has_key(key)) {
		eb_style = read_text_panel_style(*table.get_table(key));
	} else {
		fail_if_doing_default_style("editbox style", key);
		eb_style = new UI::TextPanelStyleInfo(default_style->editbox_style(style));
	}
	editboxstyles_.insert(std::make_pair(style, std::unique_ptr<UI::TextPanelStyleInfo>(eb_style)));
}

void StyleManager::add_tabpanel_style(UI::TabPanelStyle style,
                                      const LuaTable& table,
                                      const std::string& parent,
                                      const std::string& key) {
	UI::PanelStyleInfo* tp_style;
	if (table.has_key(key)) {
		tp_style = read_panel_style(*table.get_table(key));
	} else {
		fail_if_doing_default_style("tabpanel style", format("%s.%s", parent, key));
		tp_style = new UI::PanelStyleInfo(*default_style->tabpanel_style(style));
	}
	tabpanelstyles_.insert(std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(tp_style)));
}

void StyleManager::add_progressbar_style(UI::PanelStyle style,
                                         const LuaTable& table,
                                         const std::string& key) {
	UI::ProgressbarStyleInfo* pb_style;
	if (table.has_key(key)) {
		std::unique_ptr<LuaTable> style_table = table.get_table(key);
		std::unique_ptr<LuaTable> color_table = style_table->get_table("background_colors");
		pb_style = new UI::ProgressbarStyleInfo(read_font_style(*style_table, "font"),
		                                        read_rgb_color(*color_table->get_table("low")),
		                                        read_rgb_color(*color_table->get_table("medium")),
		                                        read_rgb_color(*color_table->get_table("high")));
	} else {
		fail_if_doing_default_style("progressbar style", key);
		pb_style = new UI::ProgressbarStyleInfo(default_style->progressbar_style(style));
	}
	progressbar_styles_.insert(
	   std::make_pair(style, std::unique_ptr<const UI::ProgressbarStyleInfo>(pb_style)));
}

void StyleManager::add_table_style(UI::PanelStyle style,
                                   const LuaTable& table,
                                   const std::string& key) {
	UI::TableStyleInfo* t_style;
	if (table.has_key(key)) {
		std::unique_ptr<LuaTable> style_table = table.get_table(key);
		t_style = new UI::TableStyleInfo(read_font_style(*style_table, "enabled"),
		                                 read_font_style(*style_table, "disabled"),
		                                 read_font_style(*style_table, "hotkey"));
	} else {
		fail_if_doing_default_style("table style", key);
		const UI::TableStyleInfo& fallback = default_style->table_style(style);
		t_style = new UI::TableStyleInfo(new UI::FontStyleInfo(fallback.enabled()),
		                                 new UI::FontStyleInfo(fallback.disabled()),
		                                 new UI::FontStyleInfo(fallback.hotkey()));
	}
	table_styles_.insert(std::make_pair(style, std::unique_ptr<const UI::TableStyleInfo>(t_style)));
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
	UI::FontStyleInfo* census_font;
	UI::FontStyleInfo* status_font;
	UI::FontStyleInfo* button_font;
	UI::FontStyleInfo* details_font;
	int editbox_margin;
	RGBColor construction_color;
	RGBColor neutral_color;
	RGBColor low_color;
	RGBColor medium_color;
	RGBColor high_color;
	RGBColor low_alt_color;
	RGBColor medium_alt_color;
	RGBColor high_alt_color;

	if (table.has_key("census_font")) {
		census_font = read_font_style(table, "census_font");
	} else {
		fail_if_doing_default_style("section", "building_statistics.census_font");
		census_font = new UI::FontStyleInfo(default_style->building_statistics_style().census_font());
	}
	if (table.has_key("statistics_font")) {
		status_font = read_font_style(table, "statistics_font");
	} else {
		fail_if_doing_default_style("section", "building_statistics.statistics_font");
		status_font =
		   new UI::FontStyleInfo(default_style->building_statistics_style().statistics_font());
	}

	if (table.has_key("statistics_window")) {
		std::unique_ptr<LuaTable> window_table = table.get_table("statistics_window");
		std::unique_ptr<LuaTable> fonts_table = window_table->get_table("fonts");
		button_font = read_font_style(*fonts_table, "button_font");
		details_font = read_font_style(*fonts_table, "details_font");
		editbox_margin = window_table->get_int("editbox_margin");
	} else {
		fail_if_doing_default_style("section", "building_statistics.statistics_window");
		const UI::BuildingStatisticsStyleInfo& fallback = default_style->building_statistics_style();
		button_font = new UI::FontStyleInfo(fallback.building_statistics_button_font());
		details_font = new UI::FontStyleInfo(fallback.building_statistics_details_font());
		editbox_margin = fallback.editbox_margin();
	}

	if (table.has_key("colors")) {
		std::unique_ptr<LuaTable> colors_table = table.get_table("colors");
		construction_color = read_rgb_color(*colors_table->get_table("construction"));
		neutral_color = read_rgb_color(*colors_table->get_table("neutral"));
		low_color = read_rgb_color(*colors_table->get_table("low"));
		medium_color = read_rgb_color(*colors_table->get_table("medium"));
		high_color = read_rgb_color(*colors_table->get_table("high"));
		low_alt_color = read_rgb_color(*colors_table->get_table("low_alt"));
		medium_alt_color = read_rgb_color(*colors_table->get_table("medium_alt"));
		high_alt_color = read_rgb_color(*colors_table->get_table("high_alt"));
	} else {
		fail_if_doing_default_style("section", "building_statistics.colors");
		const UI::BuildingStatisticsStyleInfo& fallback = default_style->building_statistics_style();
		construction_color = fallback.construction_color();
		neutral_color = fallback.neutral_color();
		low_color = fallback.low_color();
		medium_color = fallback.medium_color();
		high_color = fallback.high_color();
		low_alt_color = fallback.alternative_low_color();
		medium_alt_color = fallback.alternative_medium_color();
		high_alt_color = fallback.alternative_high_color();
	}

	building_statistics_style_.reset(new UI::BuildingStatisticsStyleInfo(
	   button_font, details_font, editbox_margin, census_font, status_font, construction_color,
	   neutral_color, low_color, medium_color, high_color, low_alt_color, medium_alt_color,
	   high_alt_color));
}

void StyleManager::add_ware_info_style(UI::WareInfoStyle style,
                                       const LuaTable& table,
                                       const std::string& key) {
	UI::WareInfoStyleInfo* wi_style;
	if (table.has_key(key)) {
		std::unique_ptr<LuaTable> style_table = table.get_table(key);
		std::unique_ptr<LuaTable> fonts_table = style_table->get_table("fonts");
		std::unique_ptr<LuaTable> colors_table = style_table->get_table("colors");
		wi_style = new UI::WareInfoStyleInfo(
		   read_font_style(*fonts_table, "header"), read_font_style(*fonts_table, "info"),
		   g_image_cache->get(style_table->get_string("icon_background_image")),
		   read_rgb_color(*colors_table->get_table("icon_frame")),
		   read_rgb_color(*colors_table->get_table("icon_background")),
		   read_rgb_color(*colors_table->get_table("info_background")));
	} else {
		fail_if_doing_default_style("ware info style", key);
		const UI::WareInfoStyleInfo& fallback = default_style->ware_info_style(style);
		wi_style = new UI::WareInfoStyleInfo(new UI::FontStyleInfo(fallback.header_font()),
		                                     new UI::FontStyleInfo(fallback.info_font()),
		                                     fallback.icon_background_image(), fallback.icon_frame(),
		                                     fallback.icon_background(), fallback.info_background());
	}
	ware_info_styles_.insert(
	   std::make_pair(style, std::unique_ptr<const UI::WareInfoStyleInfo>(wi_style)));
}

void StyleManager::add_window_style(UI::WindowStyle style,
                                    const LuaTable& table,
                                    const std::string& key) {
	UI::WindowStyleInfo* w_style;
	if (table.has_key(key)) {
		std::unique_ptr<LuaTable> style_table = table.get_table(key);
		w_style = new UI::WindowStyleInfo(
		   read_rgba_color(*style_table->get_table("window_border_focused")),
		   read_rgba_color(*style_table->get_table("window_border_unfocused")),
		   g_image_cache->get(style_table->get_string("border_top")),
		   g_image_cache->get(style_table->get_string("border_bottom")),
		   g_image_cache->get(style_table->get_string("border_right")),
		   g_image_cache->get(style_table->get_string("border_left")),
		   g_image_cache->get(style_table->get_string("background")),
		   style_table->get_string("button_pin"), style_table->get_string("button_unpin"),
		   style_table->get_string("button_minimize"), style_table->get_string("button_unminimize"),
		   style_table->get_string("button_close"));
	} else {
		fail_if_doing_default_style("window style", key);
		w_style = new UI::WindowStyleInfo(default_style->window_style(style));
		/*
		      const UI::WindowStyleInfo& fallback = default_style->window_style(style);
		      w_style = new UI::WindowStyleInfo(
		         fallback.window_border_focused(), fallback.window_border_unfocused(),
		         fallback.border_top(), fallback.border_bottom(), fallback.border_right(),
		         fallback.border_left(), fallback.background(), fallback.button_pin(),
		         fallback.button_unpin(), fallback.button_minimize(), fallback.button_unminimize(),
		         fallback.button_close());
		*/
	}
	window_styles_.insert(
	   std::make_pair(style, std::unique_ptr<const UI::WindowStyleInfo>(w_style)));
}

void StyleManager::add_dropdown_style(UI::PanelStyle style,
                                      const LuaTable& table,
                                      const std::string& parent,
                                      const std::string& key) {
	UI::PanelStyleInfo* dd_style;
	if (table.has_key(key)) {
		dd_style = read_panel_style(*table.get_table(key));
	} else {
		fail_if_doing_default_style("dropdown style", format("%s.%s", parent, key));
		dd_style = new UI::PanelStyleInfo(*default_style->dropdown_style(style));
	}
	dropdownstyles_.insert(std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(dd_style)));
}

void StyleManager::add_scrollbar_style(UI::PanelStyle style,
                                       const LuaTable& table,
                                       const std::string& parent,
                                       const std::string& key) {
	UI::PanelStyleInfo* sb_style;
	if (table.has_key(key)) {
		sb_style = read_panel_style(*table.get_table(key));
	} else {
		fail_if_doing_default_style("scrollbar style", format("%s.%s", parent, key));
		sb_style = new UI::PanelStyleInfo(*default_style->scrollbar_style(style));
	}
	scrollbarstyles_.insert(std::make_pair(style, std::unique_ptr<UI::PanelStyleInfo>(sb_style)));
}

void StyleManager::add_font_style(UI::FontStyle font_key,
                                  const LuaTable& table,
                                  const std::string& table_key) {
	UI::FontStyleInfo* fontstyle;
	if (table.has_key(table_key)) {
		fontstyle = read_font_style(table, table_key);
	} else {
		fail_if_doing_default_style("font style", table_key);
		fontstyle = new UI::FontStyleInfo(default_style->font_style(font_key));
	}
	fontstyles_.emplace(std::make_pair(font_key, std::unique_ptr<UI::FontStyleInfo>(fontstyle)));
	fontstyle_keys_.emplace(std::make_pair(table_key, font_key));
}

void StyleManager::add_paragraph_style(UI::ParagraphStyle style,
                                       const LuaTable& table,
                                       const std::string& table_key) {
	UI::ParagraphStyleInfo* p_style;
	if (table.has_key(table_key)) {
		p_style = read_paragraph_style(table, table_key);
	} else {
		fail_if_doing_default_style("paragraph style", table_key);
		p_style = new UI::ParagraphStyleInfo(default_style->paragraph_style(style));
	}
	paragraphstyles_.emplace(
	   std::make_pair(style, std::unique_ptr<UI::ParagraphStyleInfo>(p_style)));
	paragraphstyle_keys_.emplace(std::make_pair(table_key, style));
}

void StyleManager::add_color(UI::ColorStyle id, const LuaTable& table, const std::string& key) {
	if (table.has_key(key)) {
		const std::unique_ptr<LuaTable> color_table(table.get_table(key));
		colors_.emplace(std::make_pair(id, read_rgb_color(*color_table)));
	} else {
		fail_if_doing_default_style("color style", key);
		colors_.emplace(std::make_pair(id, default_style->color(id)));
	}
	color_keys_.emplace(std::make_pair(key, id));
}

void StyleManager::add_styled_size(UI::StyledSize id,
                                   const LuaTable& table,
                                   const std::string& key) {
	if (table.has_key(key)) {
		styled_sizes_.emplace(std::make_pair(id, table.get_int(key)));
	} else {
		fail_if_doing_default_style("styled size", key);
		styled_sizes_.emplace(std::make_pair(id, default_style->styled_size(id)));
	}
	styled_size_keys_.emplace(std::make_pair(key, id));
}
