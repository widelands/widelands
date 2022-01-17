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

#ifndef WL_GRAPHIC_STYLES_FONT_STYLE_H
#define WL_GRAPHIC_STYLES_FONT_STYLE_H

#include "graphic/color.h"

namespace UI {
enum class FontStyle {
	kChatMessage,
	kChatPlayername,
	kChatServer,
	kChatTimestamp,
	kChatWhisper,
	kFsGameSetupHeadings,
	kFsGameSetupIrcClient,
	kFsGameSetupSuperuser,
	kFsGameSetupMapname,
	kFsMenuGameTip,
	kFsMenuInfoPanelHeading,
	kFsMenuInfoPanelParagraph,
	kFsMenuIntro,
	kFsMenuTranslationInfo,
	kItalic,
	kDisabled,
	kFsMenuLabel,
	kWuiLabel,
	kFsTooltipHeader,
	kFsTooltipHotkey,
	kFsTooltip,
	kWuiTooltipHeader,
	kWuiTooltipHotkey,
	kWuiTooltip,
	kWarning,
	kGameSummaryTitle,
	kWuiAttackBoxSliderLabel,
	kWuiGameSpeedAndCoordinates,
	kWuiInfoPanelHeading,
	kWuiInfoPanelParagraph,
	kWuiMessageHeading,
	kWuiMessageParagraph,
	kFsMenuWindowTitle,
	kWuiWindowTitle
};

struct FontStyleInfo {
	enum class Face { kSans, kSerif, kCondensed };

	explicit FontStyleInfo(const std::string& init_face,
	                       const RGBColor& init_color,
	                       int init_size,
	                       bool init_bold,
	                       bool init_italic,
	                       bool init_underline,
	                       bool init_shadow);
	explicit FontStyleInfo(const FontStyleInfo& other);

	std::string as_font_tag(const std::string& text) const;

	Face face() const;
	void make_condensed();

	const RGBColor& color() const;
	void set_color(const RGBColor& new_color);

	int size() const;
	void set_size(int new_size);

	bool bold() const;
	bool italic() const;
	bool underline() const;
	bool shadow() const;

private:
	static Face string_to_face(const std::string& face_);
	const std::string face_to_string() const;

	Face face_;
	RGBColor color_;
	int size_;
	const bool bold_;
	const bool italic_;
	const bool underline_;
	const bool shadow_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_FONT_STYLE_H
