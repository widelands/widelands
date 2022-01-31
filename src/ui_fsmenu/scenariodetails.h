/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_SCENARIODETAILS_H
#define WL_UI_FSMENU_SCENARIODETAILS_H

#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_fsmenu/campaigns.h"

/**
 * Show a Box with information about a campaign or tutorial scenario.
 */
class ScenarioDetails : public UI::Box {
public:
	explicit ScenarioDetails(Panel* parent);

	void update(const ScenarioData& scenariodata);

private:
	UI::MultilineTextarea name_label_;
	UI::MultilineTextarea descr_;
};

#endif  // end of include guard: WL_UI_FSMENU_SCENARIODETAILS_H
