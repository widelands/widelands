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

#include "wui/chatoverlay.h"

#include "chat.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "logmessage.h"
#include "profile/profile.h"

/**
 * Time, in seconds, that chat messages are shown in the overlay.
 */
static const int32_t CHAT_DISPLAY_TIME = 10;
static const uint32_t MARGIN = 2;

struct ChatOverlay::Impl : Widelands::NoteReceiver<ChatMessage>,
	Widelands::NoteReceiver<LogMessage> {
	bool transparent_;
	ChatProvider * chat_;
	bool havemessages_;

	/// Reception time of oldest message
	time_t oldest_;

	/// Layouted message list
	std::string all_text_;

	/// Log messages
	std::vector<LogMessage> log_messages_;

	Impl() : transparent_(false), chat_(nullptr), havemessages_(false), oldest_(0) {}

	void recompute();
	virtual void receive(const ChatMessage & note) override;
	virtual void receive(const LogMessage & note) override;
};

ChatOverlay::ChatOverlay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
	: UI::Panel(parent, x, y, w, h), m(new Impl())
{
	Section & s = g_options.pull_section("global");
	m->transparent_ = s.get_bool("transparent_chat", true);

	set_think(true);
}

ChatOverlay::~ChatOverlay()
{
}

void ChatOverlay::setChatProvider(ChatProvider & chat)
{
	m->chat_ = &chat;
	Widelands::NoteReceiver<ChatMessage>* cmr
		= dynamic_cast<Widelands::NoteReceiver<ChatMessage>*>(m.get());
	cmr->connect(chat);
	m->recompute();
}

void ChatOverlay::setLogProvider(Widelands::NoteSender<LogMessage>& log_sender)
{
	Widelands::NoteReceiver<LogMessage>* lmr
		= dynamic_cast<Widelands::NoteReceiver<LogMessage>*>(m.get());
	lmr->connect(log_sender);
}


/**
 * Check for message expiry.
 */
void ChatOverlay::think()
{
	if (m->havemessages_) {
		if (time(nullptr) - m->oldest_ > CHAT_DISPLAY_TIME)
			m->recompute();
	}
}

/**
 * Callback that is run when a new chat message comes in.
 */
void ChatOverlay::Impl::receive(const ChatMessage & /* note */)
{
	recompute();
}

void ChatOverlay::Impl::receive(const LogMessage& note)
{
	log_messages_.push_back(note);
	recompute();
}


/**
 * Recompute the chat message display.
 */
void ChatOverlay::Impl::recompute()
{
	int32_t const now = time(nullptr);

	havemessages_ = false;

	// Parse the chat message list as well as the log message list
	// and display them in chronological order
	int32_t chat_idx = chat_ != nullptr ? chat_->getMessages().size() - 1 : -1;
	int32_t log_idx = log_messages_.empty() ? -1 : log_messages_.size() - 1;
	std::string richtext;

	while ((chat_idx >= 0 || log_idx >= 0)) {
		if
			(chat_idx < 0 ||
				(log_idx >= 0 && chat_->getMessages()[chat_idx].time < log_messages_[log_idx].time))
		{
			// Log message is more recent
			oldest_ = log_messages_[log_idx].time;
			// Do some richtext formatting here
			if (now - oldest_ < CHAT_DISPLAY_TIME) {
				richtext = "<p><font face=DejaVuSerif size=14 color=dddddd bold=1>"
					+ log_messages_[log_idx].msg + "<br></font></p>" + richtext;
			}
			log_idx--;
		} else if
			(log_idx < 0 ||
				(chat_idx >= 0 && chat_->getMessages()[chat_idx].time >= log_messages_[log_idx].time))
		{
			// Chat message is more recent
			oldest_ = chat_->getMessages()[chat_idx].time;
			if (now - oldest_ < CHAT_DISPLAY_TIME) {
				richtext = chat_->getMessages()[chat_idx].toPrintable()
					+ richtext;
			}
			chat_idx--;
		} else {
			// Shoudn't happen
			assert(false);
		}
		havemessages_ = true;
	}

	// Parse log messages to clear old ones
	while (!log_messages_.empty()) {
		if (log_messages_.front().time < now - CHAT_DISPLAY_TIME) {
			log_messages_.erase(log_messages_.begin());
		} else {
			break;
		}
	}

	if (havemessages_) {
		all_text_ = "<rt>" + richtext + "</rt>";
	}
}

void ChatOverlay::draw(RenderTarget & dst)
{
	if (!m->havemessages_)
		return;

	const Image* im = UI::g_fh1->render(m->all_text_, get_w());
	// Background
	int32_t height = im->height() > get_h() ? get_h() : im->height();
	int32_t top = get_h() - height - 2 * MARGIN;

	//FIXME: alpha channel not respected
	if (!m->transparent_) {
		Rect rect(0, top, im->width(), height);
		dst.fill_rect(rect, RGBAColor(50, 50, 50, 128));
	}
	int32_t topcrop = im->height() - height;
	Rect cropRect(0, topcrop, im->width(), height);

	Point pt(0, top);
	dst.blitrect(pt, im, cropRect);
}
