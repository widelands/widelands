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

#ifndef WL_WUI_CHAT_MSG_LAYOUT_H
#define WL_WUI_CHAT_MSG_LAYOUT_H

#include "chat/chat.h"

// Formats 'chat_message' as richtext.
std::string format_as_richtext(const ChatMessage& chat_message);

#endif  // end of include guard: WL_WUI_CHAT_MSG_LAYOUT_H
