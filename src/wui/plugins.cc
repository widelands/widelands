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

#include "wui/plugins.h"

#include <SDL_timer.h>

#include "base/log.h"
#include "base/string.h"
#include "scripting/lua_errors.h"
#include "ui_basic/messagebox.h"

bool PluginTimers::plugin_action(const std::string& action, bool failsafe) {
	try {
		lua_(action);
		return true;
	} catch (const LuaError& e) {
		if (!failsafe || g_fail_on_lua_error) {
			log_err("FATAL: Lua error in plugin: %s", e.what());
			throw;
		}

		log_err("Lua error in plugin: %s", e.what());
		UI::WLMessageBox m(root_panel_,
		                   root_panel_->get_panel_style() == UI::PanelStyle::kWui ?
                            UI::WindowStyle::kWui :
                            UI::WindowStyle::kFsMenu,
		                   _("Plugin Error"), format_l(_("Error when running plugin:\n%s"), e.what()),
		                   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
		return false;
	}
}

PluginTimers::Timer* PluginTimers::get_timer(const std::string& name) {
	for (Timer& timer : timers_) {
		if (timer.name == name) {
			return &timer;
		}
	}
	return nullptr;
}

const PluginTimers::Timer* PluginTimers::get_timer(const std::string& name) const {
	for (const Timer& timer : timers_) {
		if (timer.name == name) {
			return &timer;
		}
	}
	return nullptr;
}

uint32_t PluginTimers::remove_timer(const std::string& name, bool all) {
	uint32_t erased = 0;
	for (auto timer = timers_.begin(); timer != timers_.end();) {
		if (timer->name == name) {
			timer = timers_.erase(timer);
			++erased;
			if (!all) {
				break;
			}
		} else {
			++timer;
		}
	}
	return erased;
}

void PluginTimers::think() {
	const uint32_t curtime = SDL_GetTicks();
	for (auto timer = timers_.begin(); timer != timers_.end();) {
		if (curtime >= timer->next_run) {
			timer->next_run = curtime + timer->interval;
			if (timer->active) {
				if (!plugin_action(timer->action, timer->failsafe)) {
					// In case of an error, remove it from the queue
					log_err("Unregistering defective plugin timer");
					timer = timers_.erase(timer);
					continue;
				}

				if (timer->remaining_count != 0) {
					--timer->remaining_count;
					if (timer->remaining_count == 0) {
						timer->active = false;
					}
				}
			}
		}
		++timer;
	}
}
