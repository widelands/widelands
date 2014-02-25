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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef GAME_TIPS_H
#define GAME_TIPS_H

#include <cstring>
#include <string>
#include <vector>

#include "ui_basic/progresswindow.h"

/// Displays game tips in progress window
struct GameTips : public UI::IProgressVisualization {
	GameTips
		(UI::ProgressWindow & progressWindow, const std::vector<std::string>&);
	virtual ~GameTips();

	virtual void update(bool repaint) override;
	virtual void stop() override;

private:
	struct Tip {
		std::string  text;
		int32_t          interval;
	};
	void load_tips(std::string);
	void show_tip(int32_t index);

	uint32_t             m_lastUpdated;
	uint32_t             m_updateAfter;
	UI::ProgressWindow & m_progressWindow;
	bool                 m_registered;
	uint32_t             m_lastTip;

	std::vector<Tip>     m_tips;
};

#endif
