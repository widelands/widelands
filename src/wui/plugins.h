/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_WUI_PLUGINS_H
#define WL_WUI_PLUGINS_H

#include <cstdint>
#include <string>
#include <vector>

class LuaInterface;

namespace UI {
class Panel;
}  // namespace UI

class PluginTimers {
public:
	PluginTimers(UI::Panel* p, LuaInterface* lua) : root_panel_(p), lua_(lua) {
	}

	bool plugin_action(const std::string& action, bool failsafe);

	void think();

	void add_plugin_timer(const std::string& action, uint32_t interval, bool failsafe) {
		timers_.emplace_back(action, interval, failsafe);
	}

private:
	struct Timer {
		Timer() = default;
		explicit Timer(const std::string& a, uint32_t i, bool f)
		   : action(a), interval(i), failsafe(f) {
		}

		std::string action;
		uint32_t interval{0U};
		uint32_t next_run{0U};
		bool failsafe{false};
	};

	UI::Panel* root_panel_;
	LuaInterface* lua_;

	std::vector<Timer> timers_;
};

#endif  // end of include guard: WL_WUI_PLUGINS_H
