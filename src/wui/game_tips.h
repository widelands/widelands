/*
 * Copyright (C) 2007-2021 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_TIPS_H
#define WL_WUI_GAME_TIPS_H

#include "ui_basic/progresswindow.h"

/// Displays game tips in progress window
struct GameTips : public UI::IProgressVisualization {
	GameTips(UI::ProgressWindow& progressWindow, const std::vector<std::string>&);
	~GameTips() override;

	void update(RenderTarget&, const Recti& bounds) override;
	void stop() override;

private:
	struct Tip {
		std::string text;
		int32_t interval;
	};
	void load_tips(const std::string& name);
	void show_tip(RenderTarget&, const Recti& bounds, int32_t index);

	uint32_t last_updated_;
	uint32_t update_after_;
	UI::ProgressWindow& progressWindow_;
	bool registered_;
	uint32_t last_tip_;

	std::vector<Tip> tips_;
};

#endif  // end of include guard: WL_WUI_GAME_TIPS_H
