/*
 * Copyright (C) 2008-2011 by the Widelands Development Team
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

#include "chat/chat.h"

#include "logic/constants.h"
#include "logic/player.h"

using namespace Widelands;

ChatProvider::~ChatProvider() {}

// NOCOM(#sirver): Who uses this?
std::string ChatMessage::color() const
{
	if ((playern >= 0) && playern < MAX_PLAYERS) {
		const RGBColor & clr = Player::Colors[playern];
		char buf[sizeof("ffffff")];
		snprintf(buf, sizeof(buf), "%.2x%.2x%.2x", clr.r, clr.g, clr.b);
		return buf;
	}
	return "999999";
}
