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

#ifndef WL_GRAPHIC_STYLE_MANAGER_H
#define WL_GRAPHIC_STYLE_MANAGER_H

#include <map>
#include <memory>

#include "graphic/styles/font_styles.h"
#include "graphic/styles/map_object_style.h"
#include "graphic/styles/panel_styles.h"
#include "graphic/styles/ware_info_style.h"
#include "scripting/lua_table.h"

static const std::string kTemplateDir = "templates/default/";

namespace UI {
// NOCOM move to own file?
struct ProgressbarStyleInfo {
	UI::FontStyleInfo font;
	RGBColor low_color;
	RGBColor medium_color;
	RGBColor high_color;
};

struct StatisticsPlotStyleInfo {
	UI::FontStyleInfo x_tick_font;
	UI::FontStyleInfo y_min_value_font;
	UI::FontStyleInfo y_max_value_font;

	RGBColor axis_line_color;
	RGBColor zero_line_color;
};

struct TableStyleInfo {
	UI::FontStyleInfo enabled;
	UI::FontStyleInfo disabled;
};

} // namespace UI

class StyleManager {
public:
	StyleManager() = default;
	~StyleManager() = default;

	// Late initialization, because Graphics needs to load the image files first.
	void init();

	const UI::PanelStyleInfo* button_style(UI::ButtonStyle) const;
	const UI::PanelStyleInfo* slider_style(UI::SliderStyle) const;
	const UI::PanelStyleInfo* tabpanel_style(UI::TabPanelStyle) const;
	const UI::PanelStyleInfo* editbox_style(UI::PanelStyle) const;
	const UI::PanelStyleInfo* dropdown_style(UI::PanelStyle) const;
	const UI::PanelStyleInfo* scrollbar_style(UI::PanelStyle) const;
	const UI::MapObjectStyleInfo& map_object_style() const;
	const UI::ProgressbarStyleInfo& progressbar_style(UI::PanelStyle) const;
	const UI::StatisticsPlotStyleInfo& statistics_plot_style() const;
	const UI::TableStyleInfo& table_style(UI::PanelStyle) const;
	const UI::WareInfoStyleInfo& ware_info_style(UI::WareInfoStyle) const;
	const UI::FontStyleInfo& font_style(UI::FontStyle style) const;

	// Special elements
	int minimum_font_size() const;
	const RGBColor& minimap_icon_frame() const;
	static std::string color_tag(const std::string& text, const RGBColor& color);

private:
	using PanelStyleMap = std::map<UI::PanelStyle, std::unique_ptr<const UI::PanelStyleInfo>>;
	void add_button_style(UI::ButtonStyle style, const LuaTable& table, const std::string& key);
	void add_slider_style(UI::SliderStyle style, const LuaTable& table, const std::string& key);
	void add_editbox_style(UI::PanelStyle style, const LuaTable& table, const std::string& key);
	void add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table);
	void add_progressbar_style(UI::PanelStyle style, const LuaTable& table);
	void add_table_style(UI::PanelStyle style, const LuaTable& table);
	void add_ware_info_style(UI::WareInfoStyle style, const LuaTable& table);
	void add_style(UI::PanelStyle style, const LuaTable& table, PanelStyleMap* map);
	void add_font_style(UI::FontStyle font, const LuaTable& table, const std::string& key);

	std::map<UI::ButtonStyle, std::unique_ptr<const UI::PanelStyleInfo>> buttonstyles_;
	std::map<UI::SliderStyle, std::unique_ptr<const UI::PanelStyleInfo>> sliderstyles_;
	std::map<UI::TabPanelStyle, std::unique_ptr<const UI::PanelStyleInfo>> tabpanelstyles_;
	PanelStyleMap editboxstyles_;
	PanelStyleMap dropdownstyles_;
	PanelStyleMap scrollbarstyles_;

	int minimum_font_size_;
	RGBColor minimap_icon_frame_;
	std::map<UI::FontStyle, std::unique_ptr<const UI::FontStyleInfo>> fontstyles_;
	std::unique_ptr<const UI::MapObjectStyleInfo> map_object_style_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::ProgressbarStyleInfo>> progressbar_styles_;
	std::unique_ptr<const UI::StatisticsPlotStyleInfo> statistics_plot_style_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::TableStyleInfo>> table_styles_;
	std::map<UI::WareInfoStyle, std::unique_ptr<const UI::WareInfoStyleInfo>> ware_info_styles_;

	DISALLOW_COPY_AND_ASSIGN(StyleManager);
};

#endif  // end of include guard: WL_GRAPHIC_STYLE_MANAGER_H
