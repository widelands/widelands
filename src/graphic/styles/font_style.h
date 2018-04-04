/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#include <string>

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
   kFsGameSetupMapname,
   kFsMenuGameTip,
   kFsMenuInfoPanelHeading,
   kFsMenuInfoPanelParagraph,
   kFsMenuIntro,
	kFsMenuTitle,
	kFsMenuTranslationInfo,
   kLabel,
   kTooltip,
   kWarning,
   kWuiGameSpeedAndCoordinates,
   kWuiInfoPanelHeading,
   kWuiInfoPanelParagraph,
   kWuiMessageHeading,
   kWuiMessageParagraph,
   kWuiWindowTitle
};

struct FontStyleInfo {
	enum class Face { kSans, kSerif, kCondensed };

	FontStyleInfo();
	explicit FontStyleInfo(const std::string& face, const RGBColor& color, int size);
	explicit FontStyleInfo(const Face& face, const RGBColor& color, int size);

	std::string as_font_tag(const std::string& text) const;

private:
	static Face string_to_face(const std::string& face);
	const std::string face_to_string() const;

public:
	Face face;
	RGBColor color;
	int size;
	bool bold;
	bool italic;
	bool underline;
	bool shadow;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_FONT_STYLE_H
