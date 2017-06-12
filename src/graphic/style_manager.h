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

#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/panel_styles.h"
#include "scripting/lua_table.h"

static const std::string kTemplateDir = "templates/default/";

class StyleManager {
public:
	struct PanelStyleInfo {
		PanelStyleInfo(const Image* init_image, const RGBAColor& init_color) : image(init_image), color(init_color) {}
		const Image* image;
		const RGBAColor color;
	};

	StyleManager() = default;
	~StyleManager() = default;

	// Late initialization, because Graphics needs to load the image files first.
	void init();

	const PanelStyleInfo& button_style(UI::ButtonStyle) const;
	const PanelStyleInfo& slider_style(UI::SliderStyle) const;
	const PanelStyleInfo& tabpanel_style(UI::TabPanelStyle) const;
	const PanelStyleInfo& editbox_style(UI::PanelStyle) const;
	const PanelStyleInfo& dropdown_style(UI::PanelStyle) const;
	const PanelStyleInfo& scrollbar_style(UI::PanelStyle) const;

private:
	void add_button_style(UI::ButtonStyle style, const LuaTable& table);
	void add_slider_style(UI::SliderStyle style, const LuaTable& table);
	void add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table);
	void add_style(UI::PanelStyle style, const LuaTable& table, std::map<UI::PanelStyle, std::unique_ptr<const PanelStyleInfo>>* map);

	std::map<UI::ButtonStyle, std::unique_ptr<const PanelStyleInfo>> buttonstyles_;
	std::map<UI::SliderStyle, std::unique_ptr<const PanelStyleInfo>> sliderstyles_;
	std::map<UI::TabPanelStyle, std::unique_ptr<const PanelStyleInfo>> tabpanelstyles_;
	std::map<UI::PanelStyle, std::unique_ptr<const PanelStyleInfo>> editboxstyles_;
	std::map<UI::PanelStyle, std::unique_ptr<const PanelStyleInfo>> dropdownstyles_;
	std::map<UI::PanelStyle, std::unique_ptr<const PanelStyleInfo>> scrollbarstyles_;

	DISALLOW_COPY_AND_ASSIGN(StyleManager);
};

#endif  // end of include guard: WL_GRAPHIC_STYLE_MANAGER_H
