/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef CHATOVERLAY_H
#define CHATOVERLAY_H

#include "logic/notification.h"
#include "ui_basic/panel.h"

struct ChatProvider;
struct LogMessage;

/**
 * The overlay that displays all new chat messages for some timeout on the main window.
 *
 * \see GameChatPanel, GameChatMenu
 */
struct ChatOverlay : public UI::Panel {
	ChatOverlay(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);
	~ChatOverlay();

	void setChatProvider(ChatProvider &);
	void setLogProvider(Widelands::NoteSender<LogMessage> &);
	virtual void draw(RenderTarget &) override;
	virtual void think() override;

private:
	struct Impl;
	std::unique_ptr<Impl> m;
};

#endif // CHATOVERLAY_H
