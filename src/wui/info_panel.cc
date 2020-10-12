/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "wui/info_panel.h"

#include <memory>

#include <SDL_timer.h>

#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/message_queue.h"
#include "wui/interactive_player.h"

constexpr int8_t kSpacing = 4;

constexpr int16_t kHeightUnit = 24;

constexpr int16_t kMessagePreviewMaxLifetime = 20 * 1000;  // 20 seconds

MessagePreview::MessagePreview(InfoPanel& i, const std::string& text, const std::string& tooltip)
: UI::Textarea(&i, 0, 0, 0, 0, text, UI::Align::kCenter, g_style_manager->font_style(UI::FontStyle::kWuiGameSpeedAndCoordinates)),
owner_(i),
creation_time_(SDL_GetTicks()),
message_(nullptr),
id_()
{
	set_thinks(true);
	set_handle_mouse(true);
	set_tooltip(tooltip);
}

MessagePreview::MessagePreview(InfoPanel& i, const Widelands::Message& m, Widelands::MessageId id) : MessagePreview(i, m.title(), as_message(m.heading(), m.body())) {
	message_ = &m;
	id_ = id;
}

void MessagePreview::think() {
	if (!owner_.message_queue_->count(id_.value()) || SDL_GetTicks() - creation_time_ > kMessagePreviewMaxLifetime) {
		owner_.pop_message(*this);
	}
}

void MessagePreview::draw(RenderTarget& r) {
	// NOCOM need better graphic
	r.tile(Recti(0, 0, get_w(), get_h()), g_image_cache->get(std::string(kTemplateDir) + "wui/button.png"), Vector2i(0, 0));
	if ((get_y() / kHeightUnit) % 2) {
		r.brighten_rect(Recti(0, 0, get_w(), get_h()), 16);
	}

	UI::Textarea::draw(r);
}
bool MessagePreview::handle_mousepress(uint8_t b, int32_t x, int32_t y) {
	if (b == SDL_BUTTON_RIGHT) {
		// Hide the message
		owner_.pop_message(*this);
	} else if (b == SDL_BUTTON_LEFT && message_ && message_->position()) {
		// Center view
		owner_.ibase_.map_view()->scroll_to_field(message_->position(), MapView::Transition::Smooth);
	} else {
		UI::Textarea::handle_mousepress(b, x, y);
	}
	// Always consume mousepress
	return true;
}

InfoPanel::InfoPanel(InteractiveBase& ib)
: UI::Panel(&ib, 0, 0, 0, 0),
ibase_(ib),
font_(g_style_manager->font_style(UI::FontStyle::kWuiGameSpeedAndCoordinates)),
minimized_(false),
toggle_(this, "toggle", 0, 0, kHeightUnit, kHeightUnit, UI::ButtonStyle::kWuiMenu, ""),
text_time_speed_(this, 0, 0, 0, 0, "", UI::Align::kLeft, font_),
text_fps_(this, 0, 0, 0, 0, "", UI::Align::kCenter, font_),
text_coords_(this, 0, 0, 0, 0, "", UI::Align::kRight, font_),
log_message_subscriber_(Notifications::subscribe<LogMessage>([this](const LogMessage& lm) { log_message(lm.msg); })),
message_queue_(nullptr),
last_message_id_(nullptr)
{
	toggle_.sigclicked.connect([this]() { toggle_minimized(); });

	layout();
}

void InfoPanel::toggle_minimized() {
	minimized_ = !minimized_;
	for (UI::Textarea* t : {&text_time_speed_, &text_fps_, &text_coords_}) {
		t->set_visible(!minimized_);
	}
	layout();
}

void InfoPanel::log_message(const std::string& message) {
	push_message(*new MessagePreview(*this, message, ""));
}

void InfoPanel::pop_message(MessagePreview& m) {
	for (uint8_t i = 0; i < kMaxMessages; ++i) {
		if (messages_[i].get() == &m) {
			// Delete this messages, and move all further messages down
			messages_[i].release()->die();
			for (uint8_t j = i + 1; j < kMaxMessages; ++j) {
				messages_[j - 1] = std::move(messages_[j]);
			}
			return layout();
		}
	}
	NEVER_HERE();
}

void InfoPanel::push_message(MessagePreview& message) {
	for (std::unique_ptr<MessagePreview>& m : messages_) {
		if (!m) {
			m.reset(&message);
			return layout();
		}
	}

	// No space? Delete the oldest message and start over
	pop_message(*messages_[0]);
	push_message(message);
}

void InfoPanel::set_fps_string(const std::string& t) {
	text_fps_.set_text(t);
}
void InfoPanel::set_coords_string(const std::string& t) {
	text_coords_.set_text(t);
}
void InfoPanel::set_time_string(const std::string& t) {
	time_string_ = t;
	update_time_speed_string();
}
void InfoPanel::set_speed_string(const std::string& t) {
	speed_string_ = t;
	update_time_speed_string();
}

void InfoPanel::update_time_speed_string() {
	text_time_speed_.set_text(
		time_string_.empty() ? speed_string_
		: speed_string_.empty() ? time_string_
		: (boost::format(
			/** TRANSLATORS: Game Time at Game Speed (e.g. "1:23:45 at 5×"). */
			/** TRANSLATORS: This string is aligned to the left edge of the info panel. */
			_("%1$s at %2$s")) % time_string_ % speed_string_).str());
}

void InfoPanel::think() {
	if (!last_message_id_) {
		last_message_id_.reset(new Widelands::MessageId());
		if (upcast(InteractivePlayer, p, &ibase_)) {
			message_queue_ = &p->player().messages();
		}
	}

	while (message_queue_ && *last_message_id_ != message_queue_->current_message_id()) {
		*last_message_id_ = Widelands::MessageId(last_message_id_->value() + 1);
		assert(message_queue_->count(last_message_id_->value()));
		push_message(*new MessagePreview(*this, *(*message_queue_)[*last_message_id_], *last_message_id_));
	}
}

void InfoPanel::layout() {
	const int16_t w = get_w();
	if (w != ibase_.get_w()) {
		set_size(ibase_.get_w(), kHeightUnit * (kMaxMessages + (minimized_ ? 0 : 1)));
		// this calls layout() again
		return;
	}

	toggle_.set_tooltip(minimized_ ? _("Show info panel") : _("Hide info panel"));
	toggle_.set_pic(g_image_cache->get(std::string(kTemplateDir) + "wui/window_" + (minimized_ ? "maximize" : "minimize") + ".png"));

	toggle_.set_pos(Vector2i(w - toggle_.get_w(), 0));

	const int16_t offset_y = (kHeightUnit - font_.size()) / 3;

	text_time_speed_.set_size(w / 3, kHeightUnit);
	text_time_speed_.set_pos(Vector2i(kSpacing, offset_y));

	text_coords_.set_size(w / 3, kHeightUnit);
	text_coords_.set_pos(Vector2i(toggle_.get_x() - kSpacing - text_coords_.get_w(), offset_y));

	text_fps_.set_size(w / 2, kHeightUnit);
	text_fps_.set_pos(Vector2i(w / 4, offset_y));

	uint8_t line = 0;
	for (auto& m : messages_) {
		if (m) {
			m->set_pos(Vector2i(w / 3, kHeightUnit * (++line)));
			m->set_size(w / 3, kHeightUnit);
		}
	}
}

// Consume mouse click/move events on the panel
bool InfoPanel::handle_mousepress(uint8_t, int32_t, int32_t y) {
	return !minimized_ && y < kHeightUnit;
}
bool InfoPanel::handle_mouserelease(uint8_t, int32_t, int32_t y) {
	return !minimized_ && y < kHeightUnit;
}
bool InfoPanel::handle_mousemove(uint8_t, int32_t, int32_t y, int32_t, int32_t) {
	return !minimized_ && y < kHeightUnit;
}
bool InfoPanel::handle_mousewheel(uint32_t, int32_t, int32_t y) {
	return !minimized_ && y < kHeightUnit;
}

void InfoPanel::draw(RenderTarget& r) {
	if (!minimized_) {
		r.tile(Recti(0, 0, get_w(), kHeightUnit), g_image_cache->get(std::string(kTemplateDir) + "wui/background.png"), Vector2i(0, 0));
	}
}
