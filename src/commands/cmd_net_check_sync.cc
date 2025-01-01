/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#include "commands/cmd_net_check_sync.h"

namespace Widelands {

CmdNetCheckSync::CmdNetCheckSync(const Time& dt, SyncReportCallback cb)
   : Command(dt), callback_(std::move(cb)) {
}

void CmdNetCheckSync::execute(Widelands::Game& /* game */) {
	callback_();
}

}  // namespace Widelands
