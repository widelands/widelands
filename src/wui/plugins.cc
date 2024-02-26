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
#include "scripting/lua_interface.h"
#include "ui_basic/messagebox.h"

bool PluginTimers::plugin_action(const std::string& action, bool failsafe) {
	try {
		lua_->interpret_string(action);
		return true;
	} catch (const LuaError& e) {
		if (!failsafe || g_fail_on_lua_error) {
			log_err("FATAL: Lua error in plugin: %s", e.what());
			throw;
		}

		log_err("Lua error in plugin: %s", e.what());
		UI::WLMessageBox m(root_panel_, UI::WindowStyle::kWui, _("Plugin Error"),
		                   format_l(_("Error when running plugin:\n%s"), e.what()),
		                   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
		return false;
	}
}

void PluginTimers::think() {
	const uint32_t time = SDL_GetTicks();
	for (auto plugin = timers_.begin(); plugin != timers_.end();) {
		if (time >= plugin->next_run) {
			plugin->next_run = time + plugin->interval;
			if (!plugin_action(plugin->action, plugin->failsafe)) {
				// In case of an error, remove it from the queue
				log_err("Unregistering defective plugin timer");
				plugin = timers_.erase(plugin);
				continue;
			}
		}
		++plugin;
	}
}
