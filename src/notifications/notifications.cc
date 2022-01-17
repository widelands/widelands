/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "notifications/notifications.h"

#include "base/log.h"

namespace Notifications {

NotificationsManager* NotificationsManager::get() {
	static NotificationsManager instance;
	return &instance;
}

NotificationsManager::NotificationsManager() : next_subscriber_id_(1), num_subscribers_(0) {
}

NotificationsManager::~NotificationsManager() {
	if (num_subscribers_ != 0) {
		log_err("NotificationsManager is destroyed, but there are still subscribers.\n");
	}
}

}  // namespace Notifications
