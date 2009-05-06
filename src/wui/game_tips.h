/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef GAME_TIPS_H
#define GAME_TIPS_H

#include "ui_basic/progresswindow.h"

#include <string>
#include <cstring>
#include <vector>

/// Displays game tips in progress window
struct GameTips : public UI::IProgressVisualization {
	GameTips
		(UI::ProgressWindow & progressWindow, std::vector<std::string>,
		 std::string style = "default");
	virtual ~GameTips();

	virtual void update(bool repaint);
	virtual void stop();

private:
	struct Tip {
		std::string  text;
		int32_t          interval;
	};
	void load_style(std::string);
	void load_tips(std::string);

	void show_tip(int32_t index);
	RGBColor color_from_hex(const char * hexcode, const RGBColor & def);
	uint32_t colorvalue_from_hex(char c1, char c2);

	uint32_t             m_lastUpdated;
	uint32_t             m_updateAfter;
	UI::ProgressWindow & m_progressWindow;
	bool                 m_registered;
	uint32_t                 m_lastTip;

	std::vector<Tip>     m_tips;
	std::string          m_background_picture;
	uint32_t                 m_width;
	uint32_t                 m_height;
	uint32_t                 m_pading_l;
	uint32_t                 m_pading_r;
	uint32_t                 m_pading_t;
	uint32_t                 m_pading_b;
	int32_t                  m_font_size;
	RGBColor             m_bgcolor;
	RGBColor             m_color;
};

#endif
