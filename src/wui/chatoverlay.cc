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

#include "chatoverlay.h"

#include "chat.h"
#include "graphic/rendertarget.h"
#include "graphic/richtext.h"
#include "profile/profile.h"

/**
 * Time, in seconds, that chat messages are shown in the overlay.
 */
static const int32_t CHAT_DISPLAY_TIME = 10;
static const uint32_t MARGIN = 2;

struct ChatOverlay::Impl : Widelands::NoteReceiver<ChatMessage> {
	bool transparent;
	ChatProvider * chat;
	bool havemessages;

	/// Reception time of oldest message
	time_t oldest;

	/// Layouted message list
	UI::RichText rt;

	Impl() : chat(0), havemessages(false) {}

	void recompute();
	virtual void receive(const ChatMessage & note);
};

ChatOverlay::ChatOverlay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
	: UI::Panel(parent, x, y, w, h), m(new Impl())
{
	Section & s = g_options.pull_section("global");
	m->transparent = s.get_bool("transparent_chat", true);

	set_think(true);
	m->rt.set_width(w - 2 * MARGIN);
	m->rt.set_background_color(RGBColor(50, 50, 50));
}

ChatOverlay::~ChatOverlay()
{
}

void ChatOverlay::setChatProvider(ChatProvider & chat)
{
	m->chat = &chat;
	m->connect(chat);
	m->recompute();
}

/**
 * Check for message expiry.
 */
void ChatOverlay::think()
{
	if (m->havemessages) {
		if (time(0) - m->oldest > CHAT_DISPLAY_TIME)
			m->recompute();
	}
}

/**
 * Callback that is run when a new chat message comes in.
 */
void ChatOverlay::Impl::receive(const ChatMessage & note)
{
	recompute();
}

/**
 * Recompute the chat message display.
 */
void ChatOverlay::Impl::recompute()
{
	int32_t const now = time(0);

	havemessages = false;

	std::vector<ChatMessage> const & msgs = chat->getMessages();
	uint32_t idx = msgs.size();
	std::string richtext;
	while (idx && now - msgs[idx - 1].time <= CHAT_DISPLAY_TIME) {
		richtext = msgs[idx - 1].toPrintable() + richtext;
		havemessages = true;
		oldest = msgs[idx - 1].time;
		idx--;
	}

	if (havemessages)
		rt.parse("<rt>" + richtext + "</rt>");
}

void ChatOverlay::draw(RenderTarget & dst)
{
	if (!m->havemessages)
		return;

	int32_t height = m->rt.height();
	int32_t top = get_h() - height - 2 * MARGIN;

	m->rt.draw(dst, Point(MARGIN, top + MARGIN), !m->transparent);
}
