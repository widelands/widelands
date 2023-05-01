/*
 * Copyright (C) 2020-2023 by the Widelands Development Team
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

#include "wui/info_panel.h"

#include <iomanip>
#include <memory>
#include <sstream>

#include <SDL_timer.h>

#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "logic/message_queue.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/toolbar.h"

constexpr int8_t kSpacing = 4;

constexpr int16_t kMessagePreviewMaxLifetime = 15 * 1000;  // show messages for 15 seconds

MessagePreview::MessagePreview(InfoPanel* i, const std::string& text, const std::string& tooltip)
   : UI::Textarea(i,
                  UI::PanelStyle::kWui,
                  UI::FontStyle::kWuiGameSpeedAndCoordinates,
                  0,
                  0,
                  0,
                  0,
                  // Surround the text with some whitespaces for padding
                  std::string("   ") + text + "   ",
                  UI::g_fh->fontset()->is_rtl() ? UI::Align::kRight : UI::Align::kLeft),
     owner_(*i),
     creation_time_(SDL_GetTicks()) {
	set_thinks(true);
	set_handle_mouse(true);
	set_tooltip(tooltip);
	initialization_complete();
}

MessagePreview::MessagePreview(InfoPanel* i, const Widelands::Message* m, Widelands::MessageId id)
   : MessagePreview(i, m->title(), as_message(m->heading(), m->body())) {
	message_ = m;
	id_ = id;
}

inline bool MessagePreview::message_still_exists() const {
	return !(id_.operator bool()) || (owner_.message_queue_ == nullptr) ||
	       (owner_.message_queue_->count(id_.value()) != 0u);
}

void MessagePreview::think() {
	if (!message_still_exists() || SDL_GetTicks() - creation_time_ > kMessagePreviewMaxLifetime) {
		owner_.pop_message(this);
	}
}

void MessagePreview::draw(RenderTarget& r) {
	if (!message_still_exists()) {
		return;
	}

	r.tile(Recti(0, 0, get_w(), get_h()), g_image_cache->get("wui/windows/background.png"),
	       Vector2i(0, 0));

	// every second message is highlighted
	if ((owner_.index_of(this) % 2) != 0u) {
		r.brighten_rect(Recti(0, 0, get_w(), get_h()), 16);
	}

	if ((message_ != nullptr) && (message_->icon() != nullptr)) {
		r.blit(
		   Vector2i(get_w() - message_->icon()->width(), (get_h() - message_->icon()->height()) / 2),
		   message_->icon());
	}

	UI::Textarea::draw(r);

	r.draw_rect(Recti(0, 0, get_w(), get_h()), RGBColor(0, 0, 0));

	{  // lifetime indicator
		const float w = get_w();
		const float fraction = w * (SDL_GetTicks() - creation_time_) / kMessagePreviewMaxLifetime;
		r.brighten_rect(Recti((w - fraction) / 2, get_h() - kSpacing, fraction, kSpacing), -64);
	}
}
bool MessagePreview::handle_mousepress(const uint8_t button, int32_t /* x */, int32_t /* y */) {
	switch (button) {
	case SDL_BUTTON_LEFT:  // center view
		if ((message_ != nullptr) && (message_->position().operator bool())) {
			owner_.ibase_.map_view()->scroll_to_field(
			   message_->position(), MapView::Transition::Smooth);
		}
		break;
	case SDL_BUTTON_MIDDLE:  // hide message
		owner_.pop_message(this);
		break;
	case SDL_BUTTON_RIGHT: {  // open message menu
		if ((owner_.iplayer_ != nullptr) && (message_ != nullptr)) {
			owner_.iplayer_->popup_message(id_, *message_);
		}
		break;
	}
	default:
		break;
	}
	// Always consume mousepress
	return true;
}

InfoPanel::InfoPanel(InteractiveBase& ib)
   : UI::Panel(&ib, UI::PanelStyle::kWui, 0, 0, 0, 0),
     ibase_(ib),
     snap_target_panel_(&ibase_, UI::PanelStyle::kWui, 0, 0, 0, 0),
     toggle_mode_(this,
                  "mode",
                  0,
                  0,
                  UI::main_toolbar_button_size(),
                  8,
                  UI::main_toolbar_button_size(),
                  _("Info Panel Visibility"),
                  UI::DropdownType::kPictorial,
                  UI::PanelStyle::kWui,
                  UI::ButtonStyle::kWuiMenu),
     text_time_speed_(this,
                      UI::PanelStyle::kWui,
                      UI::FontStyle::kWuiGameSpeedAndCoordinates,
                      0,
                      0,
                      0,
                      0,
                      "",
                      UI::Align::kLeft),
     text_fps_(this,
               UI::PanelStyle::kWui,
               UI::FontStyle::kWuiGameSpeedAndCoordinates,
               0,
               0,
               0,
               0,
               "",
               UI::Align::kLeft),
     text_coords_(this,
                  UI::PanelStyle::kWui,
                  UI::FontStyle::kWuiGameSpeedAndCoordinates,
                  0,
                  0,
                  0,
                  0,
                  "",
                  UI::Align::kRight),

     draw_real_time_(get_config_bool("game_clock", true)) {
	set_z(UI::Panel::ZOrder::kInfoPanel);
	text_fps_.set_handle_mouse(true);
	snap_target_panel_.set_snap_target(true);
	int mode = get_config_int("toolbar_pos", 0);
	on_top_ = !UI::main_toolbar_at_bottom();
	if ((mode & (UI::ToolbarDisplayMode::kOnMouse_Visible |
	             UI::ToolbarDisplayMode::kOnMouse_Hidden)) != 0) {
		display_mode_ = UI::ToolbarDisplayMode::kOnMouse_Visible;
	} else if ((mode & UI::ToolbarDisplayMode::kMinimized) != 0) {
		display_mode_ = UI::ToolbarDisplayMode::kMinimized;
	} else {
		display_mode_ = UI::ToolbarDisplayMode::kPinned;
	}
	rebuild_dropdown();
	update_mode();
}

void InfoPanel::rebuild_dropdown() {
	toggle_mode_.clear();

	toggle_mode_.add(_("Pin"), UI::ToolbarDisplayMode::kPinned,
	                 g_image_cache->get("wui/windows/pin.png"),
	                 display_mode_ == UI::ToolbarDisplayMode::kPinned);
	toggle_mode_.add(_("Follow mouse"), UI::ToolbarDisplayMode::kOnMouse_Visible,
	                 g_image_cache->get("images/ui_basic/fsel.png"),
	                 display_mode_ == UI::ToolbarDisplayMode::kOnMouse_Visible ||
	                    display_mode_ == UI::ToolbarDisplayMode::kOnMouse_Hidden);
	toggle_mode_.add(
	   _("Hide"), UI::ToolbarDisplayMode::kMinimized,
	   g_image_cache->get(on_top_ ? "wui/windows/minimize.png" : "wui/windows/maximize.png"),
	   display_mode_ == UI::ToolbarDisplayMode::kMinimized);

	toggle_mode_.add(
	   on_top_ ? _("Move panel to bottom") : _("Move panel to top"),
	   UI::ToolbarDisplayMode::kCmdSwap,
	   g_image_cache->get(on_top_ ? "wui/windows/maximize.png" : "wui/windows/minimize.png"));

	toggle_mode_.selected.connect([this]() {
		update_mode();
		set_config_int("toolbar_pos",
		               on_top_ ? (display_mode_ | UI::ToolbarDisplayMode::kCmdSwap) : display_mode_);
	});

	layout();
}

void InfoPanel::update_mode() {
	const UI::ToolbarDisplayMode d = toggle_mode_.get_selected();
	if (d == UI::ToolbarDisplayMode::kCmdSwap) {
		on_top_ = !on_top_;
		toolbar_->on_top = on_top_;
		rebuild_dropdown();
		return;
	}

	display_mode_ = d;

	switch (display_mode_) {
	case UI::ToolbarDisplayMode::kPinned:
		set_textareas_visibility(true);
		break;
	case UI::ToolbarDisplayMode::kMinimized:
		set_textareas_visibility(false);
		break;
	case UI::ToolbarDisplayMode::kOnMouse_Visible:
		if (is_mouse_over_panel()) {
			set_textareas_visibility(true);
		} else {
			display_mode_ = UI::ToolbarDisplayMode::kOnMouse_Hidden;
			set_textareas_visibility(false);
		}
		break;
	default:
		NEVER_HERE();
	}

	layout();
}

void InfoPanel::set_toolbar(MainToolbar& t) {
	assert(!toolbar_);
	toolbar_ = &t;
	toolbar_->on_top = on_top_;
	toolbar_->draw_background = false;
	layout();
}

inline bool InfoPanel::is_mouse_over_panel(int32_t x, int32_t y) const {
	uint8_t h;
	switch (display_mode_) {
	case UI::ToolbarDisplayMode::kMinimized:
		return false;
	case UI::ToolbarDisplayMode::kOnMouse_Hidden:
		h = kSpacing;
		break;
	default:
		h = UI::main_toolbar_button_size();
		break;
	}
	return x >= 0 && x <= get_w() &&
	       (on_top_ ? (y >= 0 && y <= h) : (y <= get_h() && y >= get_h() - h));
}

void InfoPanel::set_textareas_visibility(bool v) {
	for (UI::Textarea* t : {&text_time_speed_, &text_fps_, &text_coords_}) {
		t->set_visible(v);
	}
	if (toolbar_ != nullptr) {
		toolbar_->draw_background = !v;
	}
}

bool InfoPanel::handle_mousemove(
   uint8_t /*state*/, int32_t x, int32_t y, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	last_mouse_pos_ = Vector2i(x, y);

	switch (display_mode_) {
	case UI::ToolbarDisplayMode::kMinimized:
		return false;
	case UI::ToolbarDisplayMode::kPinned:
		return is_mouse_over_panel();
	case UI::ToolbarDisplayMode::kOnMouse_Visible:
		if (!is_mouse_over_panel()) {
			display_mode_ = UI::ToolbarDisplayMode::kOnMouse_Hidden;
			set_textareas_visibility(false);
			layout();
			return false;
		}
		return is_mouse_over_panel();
	case UI::ToolbarDisplayMode::kOnMouse_Hidden:
		if (is_mouse_over_panel()) {
			display_mode_ = UI::ToolbarDisplayMode::kOnMouse_Visible;
			set_textareas_visibility(true);
			layout();
			return true;
		}
		return false;
	default:
		NEVER_HERE();
	}
}

// Consume mouse click/move events on the panel
bool InfoPanel::handle_mousepress(uint8_t /*btn*/, int32_t x, int32_t y) {
	last_mouse_pos_ = Vector2i(x, y);
	return is_mouse_over_panel();
}
bool InfoPanel::handle_mouserelease(uint8_t /*btn*/, int32_t x, int32_t y) {
	last_mouse_pos_ = Vector2i(x, y);
	return is_mouse_over_panel();
}

bool InfoPanel::check_handles_mouse(int32_t x, int32_t y) {
	last_mouse_pos_ = Vector2i(x, y);
	if (is_mouse_over_panel() || display_mode_ == UI::ToolbarDisplayMode::kOnMouse_Visible) {
		return true;
	}

	for (Panel* p = get_first_child(); p != nullptr; p = p->get_next_sibling()) {
		if (x >= p->get_x() && y >= p->get_y() && x < p->get_x() + p->get_w() &&
		    y < p->get_y() + p->get_h() && p->check_handles_mouse(x - p->get_x(), y - p->get_y())) {
			return true;
		}
	}

	return false;
}

size_t InfoPanel::index_of(const MessagePreview* mp) const {
	size_t i = 0;
	for (const MessagePreview* m : messages_) {
		if (m == mp) {
			return i;
		}
		++i;
	}

	// Not found. Does not happen except in a race condition after message
	// preview deletion, in which case the result is not of importance.
	return 0;
}

void InfoPanel::log_message(const std::string& message, const std::string& tooltip) {
	// There is never more than 1 system message visible
	for (MessagePreview* m : messages_) {
		if (m->is_system_message()) {
			pop_message(m);
			break;
		}
	}

	push_message(new MessagePreview(this, message, tooltip));
}

void InfoPanel::pop_message(MessagePreview* m) {
	m->set_visible(false);
	m->die();
	for (auto it = messages_.begin(); it != messages_.end(); ++it) {
		if (*it == m) {
			messages_.erase(it);
			layout();
			return;
		}
	}
	NEVER_HERE();
}

void InfoPanel::push_message(MessagePreview* message) {
	messages_.push_back(message);

	if (messages_.size() > UI::kMaxPopupMessages) {
		pop_message(messages_.front());
	}

	layout();
}

void InfoPanel::set_fps_string(const bool show,
                               const bool cheating,
                               const double fps,
                               const double average) {
	if (!show && !cheating) {
		text_fps_.set_text("");
		text_fps_.set_tooltip("");
	} else {
		const std::string text = format_l("%5.1f fps (avg: %5.1f fps)", fps, average);
		// The FPS string overlaps with the coords string at low resolution.
		// Therefore abbreviate it if the available width is less than an arbitrary threshold.
		if (cheating) {
			text_fps_.set_text(_("Cheat mode enabled"));
			text_fps_.set_tooltip(text);
		} else if (get_w() < 970) {
			text_fps_.set_text(format("%.1f / %.1f", fps, average));
			text_fps_.set_tooltip(text);
		} else {
			text_fps_.set_text(text);
			text_fps_.set_tooltip("");
		}
	}
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
	for (std::string* s : {&time_string_, &realtime, &speed_string_}) {
		if (!s->empty()) {
			non_empty.push_back(s);
		}
	}

	switch (non_empty.size()) {
	case 0:
		text_time_speed_.set_text("");
		break;
	case 1:
		text_time_speed_.set_text(*non_empty.back());
		break;
	case 2:
		text_time_speed_.set_text(format(
		   /** TRANSLATORS: (Gametime · Realtime) or (Gametime · Gamespeed) or (Realtime · Gamespeed)
		    */
		   _("%1$s · %2$s"), *non_empty[0], *non_empty[1]));
		break;
	case 3:
		text_time_speed_.set_text(format(
		   /** TRANSLATORS: Gametime · Realtime · Gamespeed */
		   _("%1$s · %2$s · %3$s"), *non_empty[0], *non_empty[1], *non_empty[2]));
		break;
	default:
		NEVER_HERE();
	}
}

void InfoPanel::fast_forward_message_queue() {
	iplayer_ = dynamic_cast<InteractivePlayer*>(&ibase_);
	assert(iplayer_);
	last_message_id_.reset(
	   new Widelands::MessageId(iplayer_->player().messages().current_message_id()));
}

void InfoPanel::think() {
	if (!last_message_id_) {
		last_message_id_.reset(new Widelands::MessageId());
		iplayer_ = dynamic_cast<InteractivePlayer*>(&ibase_);
	}
	if ((iplayer_ != nullptr) && (message_queue_ == nullptr)) {
		message_queue_ = &iplayer_->player().messages();
	}

	while ((message_queue_ != nullptr) &&
	       *last_message_id_ != message_queue_->current_message_id()) {
		*last_message_id_ = Widelands::MessageId(last_message_id_->value() + 1);
		if (message_queue_->count(last_message_id_->value()) != 0u) {
			push_message(
			   new MessagePreview(this, (*message_queue_)[*last_message_id_], *last_message_id_));
		}
	}

	if (draw_real_time_) {
		// Refresh real time on every tick
		update_time_speed_string();
	}

	for (UI::Panel* p = ibase_.get_first_child(); p != nullptr; p = p->get_next_sibling()) {
		if (static_cast<uint8_t>(p->get_z()) < static_cast<uint8_t>(UI::Panel::ZOrder::kInfoPanel) &&
		    p->get_x() < snap_target_panel_.get_w() &&
		    (on_top_ ? (p->get_y() < snap_target_panel_.get_y() + snap_target_panel_.get_h()) :
                     (p->get_y() + p->get_h() > snap_target_panel_.get_y()))) {
			if (UI::Window* w = dynamic_cast<UI::Window*>(p); w != nullptr && !w->moved_by_user()) {
				w->set_pos(Vector2i(
				   w->get_x(), on_top_ ? snap_target_panel_.get_y() + snap_target_panel_.get_h() :
                                     snap_target_panel_.get_y() - w->get_h()));
			}
		}
	}
}

void InfoPanel::layout() {
	if (toolbar_ == nullptr) {
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
	toolbar_->box.set_pos(Vector2i((toolbar_->get_w() - toolbar_->box.get_w()) / 2,
	                               on_top_ ? 0 : toolbar_->get_h() - toolbar_->box.get_h()));

	const int16_t offset_y = (UI::main_toolbar_button_size() - 20 /* font size estimate */) / 4 +
	                         kSpacing + (on_top_ ? 0 : get_h() - UI::main_toolbar_button_size());

	text_coords_.set_size(w / 3, UI::main_toolbar_button_size());
	text_coords_.set_pos(Vector2i(w - text_coords_.get_w() - kSpacing, offset_y));

	text_time_speed_.set_size(w / 3, UI::main_toolbar_button_size());
	text_time_speed_.set_pos(
	   Vector2i(toggle_mode_.get_x() + toggle_mode_.get_w() + kSpacing, offset_y));

	text_fps_.set_size(w / 3, UI::main_toolbar_button_size());
	text_fps_.set_pos(Vector2i(toolbar_->get_x() + toolbar_->get_w() + kSpacing, offset_y));

	toolbar_->move_to_top();

	if (on_top_) {
		int16_t message_offset = toolbar_->box.get_h();
		for (MessagePreview* m : messages_) {
			m->move_to_top();
			m->set_size(toolbar_->box.get_w(), UI::main_toolbar_button_size());
			m->set_pos(Vector2i((get_w() - m->get_w()) / 2, message_offset));
			message_offset += m->get_h();
		}
	} else {
		int16_t message_offset = get_h() - toolbar_->box.get_h();
		for (MessagePreview* m : messages_) {
			m->move_to_top();
			m->set_size(toolbar_->box.get_w(), UI::main_toolbar_button_size());
			message_offset -= m->get_h();
			m->set_pos(Vector2i((get_w() - m->get_w()) / 2, message_offset));
		}
	}

	snap_target_panel_.set_pos(Vector2i(0, toggle_mode_.get_y()));
	snap_target_panel_.set_size((display_mode_ == UI::ToolbarDisplayMode::kMinimized ||
	                             display_mode_ == UI::ToolbarDisplayMode::kOnMouse_Hidden) ?
                                  toggle_mode_.get_w() :
                                  w,
	                            toggle_mode_.get_h());
}

void InfoPanel::draw(RenderTarget& r) {
	if (display_mode_ == UI::ToolbarDisplayMode::kMinimized) {
		return;
	}

	const int h = display_mode_ == UI::ToolbarDisplayMode::kOnMouse_Hidden ?
                    kSpacing :
                    UI::main_toolbar_button_size();
	r.brighten_rect(Recti(0, on_top_ ? 0 : get_h() - h, get_w(), h), -100);

	r.draw_rect(Recti(0, on_top_ ? h : get_h() - h - 1, get_w(), 1), RGBColor(0, 0, 0));
}
