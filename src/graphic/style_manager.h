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

#ifndef WL_GRAPHIC_STYLE_MANAGER_H
#define WL_GRAPHIC_STYLE_MANAGER_H

#include <map>
#include <memory>

#include "graphic/styles/building_statistics_style.h"
#include "graphic/styles/button_style.h"
#include "graphic/styles/font_style.h"
#include "graphic/styles/panel_styles.h"
#include "graphic/styles/progress_bar_style.h"
#include "graphic/styles/statistics_plot_style.h"
#include "graphic/styles/table_style.h"
#include "graphic/styles/text_panel_style.h"
#include "graphic/styles/ware_info_style.h"
#include "graphic/styles/window_style.h"
#include "scripting/lua_table.h"

const std::string& template_dir();
void set_template_dir(std::string);

// Load the specified image. If it does not exist,
// print a warning and use a fallback image.
// `path` is relative to the template directory.
const Image& load_safe_template_image(const std::string& path);

class StyleManager {
public:
	// Only create after ImageCache has been initialized.
	StyleManager();
	~StyleManager() = default;

	const UI::BuildingStatisticsStyleInfo& building_statistics_style() const;
	const UI::ButtonStyleInfo& button_style(UI::ButtonStyle) const;
	const UI::TextPanelStyleInfo& slider_style(UI::SliderStyle) const;
	const UI::PanelStyleInfo* tabpanel_style(UI::TabPanelStyle) const;
	const UI::TextPanelStyleInfo& editbox_style(UI::PanelStyle) const;
	const UI::PanelStyleInfo* dropdown_style(UI::PanelStyle) const;
	const UI::PanelStyleInfo* scrollbar_style(UI::PanelStyle) const;
	const UI::ProgressbarStyleInfo& progressbar_style(UI::PanelStyle) const;
	const UI::StatisticsPlotStyleInfo& statistics_plot_style() const;
	const UI::TableStyleInfo& table_style(UI::PanelStyle) const;
	const UI::WareInfoStyleInfo& ware_info_style(UI::WareInfoStyle) const;
	const UI::WindowStyleInfo& window_style(UI::WindowStyle) const;
	const UI::FontStyleInfo& font_style(UI::FontStyle style) const;

	// Special elements
	int minimum_font_size() const;
	const RGBColor& minimap_icon_frame() const;
	const RGBAColor& focused_color() const {
		return focused_color_;
	}
	const RGBAColor& semi_focused_color() const {
		return semi_focused_color_;
	}
	int focus_border_thickness() const {
		return focus_border_thickness_;
	}
	static std::string color_tag(const std::string& text, const RGBColor& color);

private:
	using PanelStyleMap = std::map<UI::PanelStyle, std::unique_ptr<const UI::PanelStyleInfo>>;
	void add_button_style(UI::ButtonStyle style, const LuaTable& table);
	void add_slider_style(UI::SliderStyle style, const LuaTable& table);
	void add_editbox_style(UI::PanelStyle style, const LuaTable& table);
	void add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table);
	void add_progressbar_style(UI::PanelStyle style, const LuaTable& table);
	void add_table_style(UI::PanelStyle style, const LuaTable& table);
	void set_statistics_plot_style(const LuaTable& table);
	void set_building_statistics_style(const LuaTable& table);
	void add_ware_info_style(UI::WareInfoStyle style, const LuaTable& table);
	void add_window_style(UI::WindowStyle style, const LuaTable& table);
	void add_style(UI::PanelStyle style, const LuaTable& table, PanelStyleMap* map);
	void add_font_style(UI::FontStyle font, const LuaTable& table, const std::string& key);

	std::map<UI::ButtonStyle, std::unique_ptr<const UI::ButtonStyleInfo>> buttonstyles_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::TextPanelStyleInfo>> editboxstyles_;
	std::map<UI::SliderStyle, std::unique_ptr<const UI::TextPanelStyleInfo>> sliderstyles_;
	std::map<UI::TabPanelStyle, std::unique_ptr<const UI::PanelStyleInfo>> tabpanelstyles_;
	PanelStyleMap dropdownstyles_;
	PanelStyleMap scrollbarstyles_;

	int minimum_font_size_, focus_border_thickness_;
	RGBColor minimap_icon_frame_;
	RGBAColor focused_color_, semi_focused_color_;
	std::map<UI::FontStyle, std::unique_ptr<const UI::FontStyleInfo>> fontstyles_;
	std::unique_ptr<const UI::BuildingStatisticsStyleInfo> building_statistics_style_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::ProgressbarStyleInfo>> progressbar_styles_;
	std::unique_ptr<const UI::StatisticsPlotStyleInfo> statistics_plot_style_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::TableStyleInfo>> table_styles_;
	std::map<UI::WareInfoStyle, std::unique_ptr<const UI::WareInfoStyleInfo>> ware_info_styles_;
	std::map<UI::WindowStyle, std::unique_ptr<const UI::WindowStyleInfo>> window_styles_;

	DISALLOW_COPY_AND_ASSIGN(StyleManager);
};

extern StyleManager* g_style_manager;

#endif  // end of include guard: WL_GRAPHIC_STYLE_MANAGER_H
