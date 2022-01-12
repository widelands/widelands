/*
 * Copyright (C) 2018-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_BUTTON_STYLE_H
#define WL_GRAPHIC_STYLES_BUTTON_STYLE_H

#include <memory>

#include "graphic/styles/panel_styles.h"
#include "graphic/styles/text_panel_style.h"

namespace UI {

enum class ButtonStyle {
	kFsMenuMenu,
	kFsMenuPrimary,
	kFsMenuSecondary,
	kWuiMenu,
	kWuiPrimary,
	kWuiSecondary,
	kWuiBuildingStats
};

PanelStyle to_panel_style(ButtonStyle);

struct ButtonStyleInfo {
	ButtonStyleInfo(const UI::TextPanelStyleInfo* init_enabled,
	                const UI::TextPanelStyleInfo* init_disabled)
	   : enabled_(init_enabled), disabled_(init_disabled) {
	}
	ButtonStyleInfo(const ButtonStyleInfo& other)
	   : enabled_(new UI::TextPanelStyleInfo(other.enabled())),
	     disabled_(new UI::TextPanelStyleInfo(other.disabled())) {
	}

	const UI::TextPanelStyleInfo& enabled() const {
		return *enabled_.get();
	}
	const UI::TextPanelStyleInfo& disabled() const {
		return *disabled_.get();
	}

private:
	std::unique_ptr<const UI::TextPanelStyleInfo> enabled_;
	std::unique_ptr<const UI::TextPanelStyleInfo> disabled_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_BUTTON_STYLE_H
