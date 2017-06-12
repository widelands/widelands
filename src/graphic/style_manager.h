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

#include "graphic/panel_styles.h"
#include "scripting/lua_table.h"

static const std::string kTemplateDir = "templates/default/";

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

private:
	using PanelStyleMap = std::map<UI::PanelStyle, std::unique_ptr<const UI::PanelStyleInfo>>;
	void add_button_style(UI::ButtonStyle style, const LuaTable& table);
	void add_slider_style(UI::SliderStyle style, const LuaTable& table);
	void add_tabpanel_style(UI::TabPanelStyle style, const LuaTable& table);
	void add_style(UI::PanelStyle style, const LuaTable& table, PanelStyleMap* map);

	std::map<UI::ButtonStyle, std::unique_ptr<const UI::PanelStyleInfo>> buttonstyles_;
	std::map<UI::SliderStyle, std::unique_ptr<const UI::PanelStyleInfo>> sliderstyles_;
	std::map<UI::TabPanelStyle, std::unique_ptr<const UI::PanelStyleInfo>> tabpanelstyles_;
	PanelStyleMap editboxstyles_;
	PanelStyleMap dropdownstyles_;
	PanelStyleMap scrollbarstyles_;

	DISALLOW_COPY_AND_ASSIGN(StyleManager);
};

#endif  // end of include guard: WL_GRAPHIC_STYLE_MANAGER_H
