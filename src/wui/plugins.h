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
#include <functional>
#include <string>
#include <vector>

namespace UI {
class Panel;
}  // namespace UI

class PluginTimers {
public:
	using RunLuaCommandFn = std::function<void(const std::string&)>;

	struct Timer {
		Timer() = default;
		explicit Timer(const std::string& n,
		               const std::string& act,
		               uint32_t iv,
		               uint32_t count,
		               bool on,
		               bool safe)
		   : name(n), action(act), interval(iv), remaining_count(count), active(on), failsafe(safe) {
		}

		std::string name;
		std::string action;
		uint32_t interval{0U};
		uint32_t next_run{0U};
		uint32_t remaining_count{0U};
		bool active{true};
		bool failsafe{false};
	};

	PluginTimers(UI::Panel* p, RunLuaCommandFn lua) : root_panel_(p), lua_(lua) {
	}

	bool plugin_action(const std::string& action, bool failsafe);

	void think();

	void add_plugin_timer(const std::string& action, uint32_t interval, bool failsafe) {
		timers_.emplace_back(std::string(), action, interval, 0, true, failsafe);
	}
	void add_plugin_timer(const std::string& name,
	                      const std::string& action,
	                      uint32_t interval,
	                      uint32_t count,
	                      bool active,
	                      bool failsafe) {
		timers_.emplace_back(name, action, interval, count, active, failsafe);
	}

	[[nodiscard]] Timer* get_timer(const std::string& name);
	[[nodiscard]] const Timer* get_timer(const std::string& name) const;
	[[nodiscard]] size_t count_timers() const {
		return timers_.size();
	}

	uint32_t remove_timer(const std::string& name, bool all);

private:
	UI::Panel* root_panel_;
	RunLuaCommandFn lua_;

	std::vector<Timer> timers_;
};

#endif  // end of include guard: WL_WUI_PLUGINS_H
