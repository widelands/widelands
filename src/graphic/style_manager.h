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

#ifndef WL_GRAPHIC_STYLE_MANAGER_H
#define WL_GRAPHIC_STYLE_MANAGER_H

#include <map>
#include <memory>

#include "graphic/styles/building_statistics_style.h"
#include "graphic/styles/button_style.h"
#include "graphic/styles/font_style.h"
#include "graphic/styles/panel_styles.h"
#include "graphic/styles/paragraph_style.h"
#include "graphic/styles/progress_bar_style.h"
#include "graphic/styles/statistics_plot_style.h"
#include "graphic/styles/table_style.h"
#include "graphic/styles/text_panel_style.h"
#include "graphic/styles/ware_info_style.h"
#include "graphic/styles/window_style.h"
#include "scripting/lua_table.h"

extern const std::string kDefaultTemplate;
const std::string& template_dir();
void set_template_dir(std::string);
inline bool is_using_default_theme() {
	return template_dir() == kDefaultTemplate;
}

namespace UI {

enum class ColorStyle {
	kCampaignBarbarianThron,
	kCampaignBarbarianBoldreth,
	kCampaignBarbarianKhantrukh,
	kCampaignEmpireLutius,
	kCampaignEmpireAmalea,
	kCampaignEmpireSaledus,
	kCampaignEmpireMarcus,
	kCampaignEmpireJulia,
	kCampaignAtlanteanJundlina,
	kCampaignAtlanteanSidolus,
	kCampaignAtlanteanLoftomor,
	kCampaignAtlanteanColionder,
	kCampaignAtlanteanOpol,
	kCampaignAtlanteanOstur,
	kCampaignAtlanteanKalitath,
	kCampaignFrisianReebaud,
	kCampaignFrisianHauke,
	kCampaignFrisianMaukor,
	kCampaignFrisianMurilius,
	kCampaignFrisianClaus,
	kCampaignFrisianHenneke,
	kCampaignFrisianIniucundus,
	kCampaignFrisianAngadthur,
	kCampaignFrisianAmazon,
	kCampaignFrisianKetelsen,
	kSPScenarioRiverAdvisor,

	// Returned when lookup by name fails
	kUnknown
};

/**************************************************************************************
 *
 * Don't forget to update doc/sphinx/source/themes.rst when you add or remove styles!
 *
 **************************************************************************************/

enum class StyledSize {
	kFsTextDefaultGap,
	kFsTextSpaceBeforeInlineHeader,
	kFsMainMenuDropdownHeight,
	kWuiTextDefaultGap,
	kWuiTextSpaceBeforeInlineHeader,
	kWuiSpaceBeforeImmovableIcon,
	kWinConditionMessageGap,
	kHelpTerrainTreeHeaderSpaceBefore,
	kHelpTerrainTreeHeaderSpaceAfter,
	kEditorTooltipIconGap,
	kCampaignMessageBoxDefaultH,
	kCampaignMessageBoxDefaultW,
	kCampaignMessageBoxTopPosY,
	kCampaignMessageBoxSizeStep,
	kCampaignMessageBoxMinH,
	kCampaignMessageBoxMaxH,
	kCampaignMessageBoxMinW,
	kCampaignMessageBoxMaxW,
	kCampaignFri02PoemIndent,
	kSPScenarioPlateauMessageBoxPosY,
	kUIDefaultPadding,
	kToolbarButtonSize
};

}  // namespace UI

/** Try to resolve an image file path relative to the
 * active theme or the template directory or the data directory, in this order.
 * Returns a fallback image path on failure.
 */
std::string resolve_template_image_filename(const std::string& path);

class StyleManager {
public:
	// Only create after ImageCache has been initialized.
	StyleManager();
	~StyleManager() = default;

	[[nodiscard]] const UI::BuildingStatisticsStyleInfo& building_statistics_style() const;
	[[nodiscard]] const UI::ButtonStyleInfo& button_style(UI::ButtonStyle) const;
	[[nodiscard]] const UI::TextPanelStyleInfo& slider_style(UI::SliderStyle) const;
	[[nodiscard]] const UI::PanelStyleInfo* tabpanel_style(UI::TabPanelStyle) const;
	[[nodiscard]] const UI::TextPanelStyleInfo& editbox_style(UI::PanelStyle) const;
	[[nodiscard]] const UI::PanelStyleInfo* dropdown_style(UI::PanelStyle) const;
	[[nodiscard]] const UI::PanelStyleInfo* scrollbar_style(UI::PanelStyle) const;
	[[nodiscard]] const UI::ProgressbarStyleInfo& progressbar_style(UI::PanelStyle) const;
	[[nodiscard]] const UI::StatisticsPlotStyleInfo& statistics_plot_style() const;
	[[nodiscard]] const UI::TableStyleInfo& table_style(UI::PanelStyle) const;
	[[nodiscard]] const UI::WareInfoStyleInfo& ware_info_style(UI::WareInfoStyle) const;
	[[nodiscard]] const UI::WindowStyleInfo& window_style(UI::WindowStyle) const;
	[[nodiscard]] const UI::FontStyleInfo& font_style(UI::FontStyle style) const;
	[[nodiscard]] const UI::ParagraphStyleInfo& paragraph_style(UI::ParagraphStyle style) const;
	[[nodiscard]] const RGBColor& color(UI::ColorStyle id) const;
	[[nodiscard]] int styled_size(UI::StyledSize id) const;

	// Look up by name for Lua.
	// If there is no style defined with the given name, then log it as warning and return a style
	// that makes the erroneously formatted text stand out.
	[[nodiscard]] const UI::FontStyleInfo& font_style(std::string name) const;
	[[nodiscard]] const UI::ParagraphStyleInfo& paragraph_style(std::string name) const;
	[[nodiscard]] const RGBColor& color(std::string name) const;
	// Returns 0 if 'name' is not defined.
	[[nodiscard]] int styled_size(std::string name) const;

	// Special elements
	[[nodiscard]] int minimum_font_size() const;
	[[nodiscard]] const RGBColor& minimap_icon_frame() const;
	[[nodiscard]] const RGBAColor& focused_color() const {
		return focused_color_;
	}
	[[nodiscard]] const RGBAColor& semi_focused_color() const {
		return semi_focused_color_;
	}
	[[nodiscard]] int focus_border_thickness() const {
		return focus_border_thickness_;
	}
	static std::string color_tag(const std::string& text, const RGBColor& color);

private:
	using PanelStyleMap = std::map<UI::PanelStyle, std::unique_ptr<const UI::PanelStyleInfo>>;
	void add_button_style(UI::ButtonStyle style,
	                      const LuaTable& table,
	                      const std::string& parent,
	                      const std::string& key);
	void add_slider_style(UI::SliderStyle style,
	                      const LuaTable& table,
	                      const std::string& parent,
	                      const std::string& key);
	void add_editbox_style(UI::PanelStyle style, const LuaTable& table, const std::string& key);
	void add_tabpanel_style(UI::TabPanelStyle style,
	                        const LuaTable& table,
	                        const std::string& parent,
	                        const std::string& key);
	void add_progressbar_style(UI::PanelStyle style, const LuaTable& table, const std::string& key);
	void add_table_style(UI::PanelStyle style, const LuaTable& table, const std::string& key);
	void set_statistics_plot_style(const LuaTable& table);
	void set_building_statistics_style(const LuaTable& table);
	void add_ware_info_style(UI::WareInfoStyle style, const LuaTable& table, const std::string& key);
	void add_window_style(UI::WindowStyle style, const LuaTable& table, const std::string& key);
	void add_dropdown_style(UI::PanelStyle style,
	                        const LuaTable& table,
	                        const std::string& parent,
	                        const std::string& key);
	void add_scrollbar_style(UI::PanelStyle style,
	                         const LuaTable& table,
	                         const std::string& parent,
	                         const std::string& key);
	void add_font_style(UI::FontStyle font, const LuaTable& table, const std::string& key);
	void
	add_paragraph_style(UI::ParagraphStyle style, const LuaTable& table, const std::string& key);
	void add_color(UI::ColorStyle id, const LuaTable& table, const std::string& key);
	void add_styled_size(UI::StyledSize id, const LuaTable& table, const std::string& key);

	std::map<UI::ButtonStyle, std::unique_ptr<const UI::ButtonStyleInfo>> buttonstyles_;
	std::map<UI::PanelStyle, std::unique_ptr<const UI::TextPanelStyleInfo>> editboxstyles_;
	std::map<UI::SliderStyle, std::unique_ptr<const UI::TextPanelStyleInfo>> sliderstyles_;
	std::map<UI::TabPanelStyle, std::unique_ptr<const UI::PanelStyleInfo>> tabpanelstyles_;
	PanelStyleMap dropdownstyles_;
	PanelStyleMap scrollbarstyles_;

	int minimum_font_size_, focus_border_thickness_;
	RGBColor minimap_icon_frame_;
	RGBAColor focused_color_, semi_focused_color_;
	std::map<std::string, UI::ColorStyle> color_keys_;
	std::map<UI::ColorStyle, RGBColor> colors_;
	std::map<std::string, UI::StyledSize> styled_size_keys_;
	std::map<UI::StyledSize, int> styled_sizes_;
	std::map<std::string, UI::FontStyle> fontstyle_keys_;
	std::map<UI::FontStyle, std::unique_ptr<const UI::FontStyleInfo>> fontstyles_;
	std::map<std::string, UI::ParagraphStyle> paragraphstyle_keys_;
	std::map<UI::ParagraphStyle, std::unique_ptr<const UI::ParagraphStyleInfo>> paragraphstyles_;
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
