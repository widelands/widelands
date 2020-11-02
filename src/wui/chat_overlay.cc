/*
 * Copyright (C) 2011-2020 by the Widelands Development Team
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

#include "wui/chat_overlay.h"

#include <memory>

#include "base/wexception.h"
#include "chat/chat.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/rt_errors.h"
#include "sound/sound_handler.h"
#include "wlapplication_options.h"
#include "wui/chat_msg_layout.h"
#include "wui/logmessage.h"

/**
 * Time, in seconds, that chat messages are shown in the overlay.
 */
static const int32_t CHAT_DISPLAY_TIME = 10;
static const uint32_t MARGIN = 2;

struct ChatOverlay::Impl {
	bool transparent_ = false;
	ChatProvider* chat_ = nullptr;
	bool havemessages_ = false;

	/// Reception time of oldest message
	time_t oldest_ = std::time(nullptr);
	time_t sound_played_ = std::time(nullptr);

	/// Layouted message list
	std::string all_text_;

	/// Log messages
	std::vector<LogMessage> log_messages_;

	std::unique_ptr<Notifications::Subscriber<ChatMessage>> chat_message_subscriber_;
	std::unique_ptr<Notifications::Subscriber<LogMessage>> log_message_subscriber_;

	FxId new_message_;

	Impl()
	   : chat_message_subscriber_(
	        Notifications::subscribe<ChatMessage>([this](const ChatMessage&) { recompute(); })),
	     log_message_subscriber_(
	        Notifications::subscribe<LogMessage>([this](const LogMessage& note) {
		        log_messages_.push_back(note);
		        recompute();
	        })),
	     new_message_(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_chat")) {
	}

	void recompute();

private:
	bool has_chat_provider() {
		// The chat provider might not have been assigned a specific subclass,
		// e.g. if there was an exception thrown.
		return (chat_ != nullptr && chat_->has_been_set());
	}
};

ChatOverlay::ChatOverlay(
   UI::Panel* const parent, int32_t const x, int32_t const y, int32_t const w, int32_t const h)
   : UI::Panel(parent, UI::PanelStyle::kWui, x, y, w, h), m(new Impl()) {
	m->transparent_ = get_config_bool("transparent_chat", true);

	set_thinks(true);
}

void ChatOverlay::set_chat_provider(ChatProvider& chat) {
	m->chat_ = &chat;
	m->recompute();
}

/**
 * Check for message expiry.
 */
void ChatOverlay::think() {
	if (m->havemessages_) {
		if (time(nullptr) - m->oldest_ > CHAT_DISPLAY_TIME) {
			m->recompute();
		}
	}
}

void ChatOverlay::recompute() {
	m->recompute();
}

/**
 * Recompute the chat message display.
 */
void ChatOverlay::Impl::recompute() {
	int32_t const now = time(nullptr);

	havemessages_ = false;

	// Parse the chat message list as well as the log message list
	// and display them in chronological order
	int32_t chat_idx = has_chat_provider() ? chat_->get_messages().size() - 1 : -1;
	int32_t log_idx = log_messages_.empty() ? -1 : log_messages_.size() - 1;
	std::string richtext;

	while ((chat_idx >= 0 || log_idx >= 0)) {
		if (chat_idx < 0 ||
		    (log_idx >= 0 && chat_->get_messages()[chat_idx].time < log_messages_[log_idx].time)) {
			// Log message is more recent
			oldest_ = log_messages_[log_idx].time;
			// Do some richtext formatting here
			if (now - oldest_ < CHAT_DISPLAY_TIME) {
				richtext =
				   (boost::format("<p>%s</p>") % g_style_manager->font_style(UI::FontStyle::kChatServer)
				                                    .as_font_tag(log_messages_[log_idx].msg))
				      .str();
			}
			log_idx--;
		} else if (log_idx < 0 || (chat_idx >= 0 && chat_->get_messages()[chat_idx].time >=
		                                               log_messages_[log_idx].time)) {
			// Chat message is more recent
			oldest_ = chat_->get_messages()[chat_idx].time;
			if (now - oldest_ < CHAT_DISPLAY_TIME) {
				richtext = format_as_richtext(chat_->get_messages()[chat_idx]).append(richtext);
			}
			if (!chat_->sound_off() && sound_played_ < oldest_) {
				g_sh->play_fx(SoundType::kChat, new_message_);
				sound_played_ = oldest_;
			}
			chat_idx--;
		} else {
			NEVER_HERE();
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

void ChatOverlay::draw(RenderTarget& dst) {
	if (!m->havemessages_) {
		return;
	}

	std::shared_ptr<const UI::RenderedText> im(nullptr);
	try {
		im = UI::g_fh->render(m->all_text_, get_w());
	} catch (RT::WidthTooSmall&) {
		// Oops, maybe one long word? We render again, not limiting the width, but
		// render everything in one single line.
		im = UI::g_fh->render(m->all_text_);
	}
	assert(im != nullptr);

	// Background
	const int32_t height = im->height() > get_h() ? get_h() : im->height();
	const int32_t top = get_h() - height - 2 * MARGIN;
	const int width = std::min<int>(get_w(), im->width());

	if (!m->transparent_) {
		dst.fill_rect(Recti(0, top, width, height), RGBAColor(50, 50, 50, 128), BlendMode::Default);
	}
	const int topcrop = im->height() - height;
	im->draw(dst, Vector2i(0, top), Recti(0, topcrop, width, height));
}
