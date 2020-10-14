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

#include <iomanip>
#include <memory>

#include <SDL_timer.h>

#include "graphic/font_handler.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/message_queue.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"

constexpr int8_t kSpacing = 4;

constexpr int16_t kHeightUnit = 24;

constexpr uint8_t kMaxMessages = 4;

constexpr int16_t kMessagePreviewMaxLifetime = 15 * 1000;  // show messages for 15 seconds

MessagePreview::MessagePreview(InfoPanel& i, const std::string& text, const std::string& tooltip)
: UI::Textarea(&i, 0, 0, 0, 0,
			// Surround the text with some whitespaces for padding
			std::string("   ") + text + "   ",
			UI::g_fh->fontset()->is_rtl() ? UI::Align::kRight : UI::Align::kLeft,
			g_style_manager->font_style(UI::FontStyle::kWuiGameSpeedAndCoordinates)),
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
	r.tile(Recti(0, 0, get_w(), get_h()), g_image_cache->get(std::string(kTemplateDir) + "wui/background.png"), Vector2i(0, 0));

	// every second message is highlighted
	if (owner_.index_of(*this) % 2) {
		r.brighten_rect(Recti(0, 0, get_w(), get_h()), 16);
	}

	if (message_ && message_->icon()) {
		r.blit(Vector2i(get_w() - message_->icon()->width(), (get_h() - message_->icon()->height()) / 2), message_->icon());
	}

	UI::Textarea::draw(r);

	r.draw_rect(Recti(0, 0, get_w(), get_h()), RGBColor(0, 0, 0));

	{  // lifetime indicator
		const float w = get_w();
		const float fraction = w * (SDL_GetTicks() - creation_time_) / kMessagePreviewMaxLifetime;
		r.brighten_rect(Recti((w - fraction) / 2, get_h() - kSpacing, fraction, kSpacing), -64);
	}
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
on_top_(get_config_bool("toolbar_pos_on_top", false)),
display_mode_(DisplayMode::kPinned),
last_mouse_pos_(Vector2i(-1, -1)),
toolbar_(nullptr),
toggle_mode_(this, "mode", 0, 0, kHeightUnit, 8, kHeightUnit, _("Info Panel Visibility"),
                  UI::DropdownType::kPictorial, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiMenu),
text_time_speed_(this, 0, 0, 0, 0, "", UI::Align::kLeft, font_),
text_fps_(this, 0, 0, 0, 0, "", UI::Align::kLeft, font_),
text_coords_(this, 0, 0, 0, 0, "", UI::Align::kRight, font_),
log_message_subscriber_(Notifications::subscribe<LogMessage>([this](const LogMessage& lm) { log_message(lm.msg); })),
message_queue_(nullptr),
last_message_id_(nullptr),
draw_real_time_(get_config_bool("game_clock", true)) {
	toggle_mode_.add(_("Pin"), DisplayMode::kPinned, g_image_cache->get(std::string(kTemplateDir) + "wui/window_pin.png"), true);
	toggle_mode_.add(_("Follow mouse"), DisplayMode::kOnMouse_Visible, g_image_cache->get("images/ui_basic/fsel.png"));
	toggle_mode_.add(_("Hide"), DisplayMode::kMinimized, g_image_cache->get(std::string(kTemplateDir) +
			(on_top_ ? "wui/window_minimize.png" : "wui/window_maximize.png")));
	toggle_mode_.selected.connect([this]() { toggle_mode(); });
}

void InfoPanel::set_toolbar(MainToolbar& t) {
	assert(!toolbar_);
	toolbar_ = &t;
	toolbar_->on_top = on_top_;
	layout();
}

inline bool InfoPanel::is_mouse_over_panel() const {
	uint8_t h;
	switch (display_mode_) {
	case DisplayMode::kMinimized:
		return false;
	case DisplayMode::kOnMouse_Hidden:
		h = kSpacing;
		break;
	default:
		h = kHeightUnit;
		break;
	}
	return last_mouse_pos_.x >= 0 && last_mouse_pos_.x <= get_w() && (on_top_ ?
		(last_mouse_pos_.y >= 0 && last_mouse_pos_.y <= h)
		: (last_mouse_pos_.y <= get_h() && last_mouse_pos_.y >= get_h() - h));
}

void InfoPanel::set_textareas_visibility(bool v) {
	for (UI::Textarea* t : {&text_time_speed_, &text_fps_, &text_coords_}) {
		t->set_visible(v);
	}
}

void InfoPanel::toggle_mode() {
	display_mode_ = toggle_mode_.get_selected();

	switch (display_mode_) {
	case DisplayMode::kPinned:
		set_textareas_visibility(true);
		break;
	case DisplayMode::kMinimized:
		set_textareas_visibility(false);
		break;
	case DisplayMode::kOnMouse_Visible:
		if (is_mouse_over_panel()) {
			set_textareas_visibility(true);
		} else {
			display_mode_ = DisplayMode::kOnMouse_Hidden;
			set_textareas_visibility(false);
		}
		break;
	default:
		NEVER_HERE();
	}

	layout();
}

bool InfoPanel::handle_mousemove(uint8_t, int32_t x, int32_t y, int32_t, int32_t) {
	last_mouse_pos_ = Vector2i(x, y);

	switch (display_mode_) {
	case DisplayMode::kMinimized:
		return false;
	case DisplayMode::kPinned:
		return is_mouse_over_panel();
	case DisplayMode::kOnMouse_Visible:
		if (!is_mouse_over_panel()) {
			display_mode_ = DisplayMode::kOnMouse_Hidden;
			set_textareas_visibility(false);
			layout();
		}
		return true;
	case DisplayMode::kOnMouse_Hidden:
		if (is_mouse_over_panel()) {
			display_mode_ = DisplayMode::kOnMouse_Visible;
			set_textareas_visibility(true);
			layout();
		}
		return true;
	}

	NEVER_HERE();
}

// Consume mouse click/move events on the panel
bool InfoPanel::handle_mousepress(uint8_t, int32_t x, int32_t y) {
	last_mouse_pos_ = Vector2i(x, y);
	return is_mouse_over_panel();
}
bool InfoPanel::handle_mouserelease(uint8_t, int32_t x, int32_t y) {
	last_mouse_pos_ = Vector2i(x, y);
	return is_mouse_over_panel();
}

size_t InfoPanel::index_of(const MessagePreview& mp) const {
	size_t i = 0;
	for (const MessagePreview* m : messages_) {
		if (m == &mp) {
			return i;
		}
		++i;
	}
	NEVER_HERE();
}

void InfoPanel::log_message(const std::string& message) {
	push_message(*new MessagePreview(*this, message, ""));
}

void InfoPanel::pop_message(MessagePreview& m) {
	m.set_visible(false);
	m.die();
	for (auto it = messages_.begin(); it != messages_.end(); ++it) {
		if (*it == &m) {
			messages_.erase(it);
			layout();
			return;
		}
	}
	NEVER_HERE();
}

void InfoPanel::push_message(MessagePreview& message) {
	messages_.push_back(&message);

	if (messages_.size() > kMaxMessages) {
		pop_message(*messages_.front());
	}

	layout();
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
	std::string realtime;
	if (draw_real_time_) {
		std::time_t t = std::time(nullptr);
		std::tm tm = *std::localtime(&t);
		std::ostringstream oss("");
		oss << std::put_time(&tm, "%X");
		realtime = oss.str();
	}

	std::vector<std::string*> non_empty;
	std::string format;
	for (std::string* s : {&time_string_, &realtime, &speed_string_}) {
		if (!s->empty()) {
			non_empty.push_back(s);
		}
	}

	boost::format f;
	switch (non_empty.size()) {
	case 1:
		f = boost::format("%s");
		break;
	case 2:
		f = boost::format(_("%1$s · %2$s"));
		break;
	case 3:
		f = boost::format(_("%1$s · %2$s · %3$s"));
		break;
	default:
		NEVER_HERE();
	}

	for (std::string* s : non_empty) {
		f % *s;
	}
	text_time_speed_.set_text(f.str());
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

	if (draw_real_time_) {
		// Refresh real time on every tick
		update_time_speed_string();
	}
}

void InfoPanel::layout() {
	if (!toolbar_) {
		// Not yet initialized
		return;
	}

	const int16_t w = get_w();
	const int16_t h = get_h();
	if (w != ibase_.get_w() || h != ibase_.get_h()) {
		set_size(ibase_.get_w(), ibase_.get_h());
		// This calls layout() again
		return;
	}

	toggle_mode_.set_pos(Vector2i(0, on_top_ ? 0 : get_h() - toggle_mode_.get_h()));

	toolbar_->set_pos(Vector2i((w - toolbar_->get_w()) / 2, on_top_ ? 0 : h - toolbar_->get_h()));
	toolbar_->box.set_pos(Vector2i((toolbar_->get_w() - toolbar_->box.get_w()) / 2, on_top_ ? 0 : toolbar_->get_h() - toolbar_->box.get_h()));

	const int16_t offset_y = (kHeightUnit - font_.size()) / 3 + (on_top_ ? 0 : get_h() - kHeightUnit);

	text_coords_.set_size(w / 3, kHeightUnit);
	text_coords_.set_pos(Vector2i(w - text_coords_.get_w() - kSpacing, offset_y));

	text_time_speed_.set_size(w / 3, kHeightUnit);
	text_time_speed_.set_pos(Vector2i(toggle_mode_.get_x() + toggle_mode_.get_w() + kSpacing, offset_y));

	text_fps_.set_size(w / 3, kHeightUnit);
	text_fps_.set_pos(Vector2i(toolbar_->get_x() + toolbar_->get_w() + kSpacing, offset_y));

	toolbar_->move_to_top();

	int16_t message_offset = on_top_ ? toolbar_->box.get_h() : 0;
	for (MessagePreview* m : messages_) {
		m->move_to_top();
		m->set_size(w / 3, kHeightUnit);
		m->set_pos(Vector2i(w / 3, message_offset));
		message_offset += m->get_h();
	}
}

void InfoPanel::draw(RenderTarget& r) {
	if (display_mode_ == DisplayMode::kMinimized) {
		return;
	}
	Recti rect;
	if (display_mode_ == DisplayMode::kOnMouse_Hidden) {
		rect = Recti(0, on_top_ ? 0 : get_h() - kSpacing, get_w(), kSpacing);
	} else {
		rect = Recti(0, on_top_ ? 0 : get_h() - kHeightUnit, get_w(), kHeightUnit);
	}
	r.tile(rect, g_image_cache->get(std::string(kTemplateDir) + "wui/background.png"), Vector2i(0, 0));
}
