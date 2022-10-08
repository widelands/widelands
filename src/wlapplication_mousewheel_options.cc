/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#include "wlapplication_mousewheel_options.h"

#include <cassert>
#include <cstdlib>
#include <map>

#include "wlapplication_options.h"

enum class MousewheelOptionType { kBool, kKeymod };

struct MousewheelOption {
	const std::string internal_name_;
	const MousewheelOptionType type_;
	const uint16_t default_;

private:
	MousewheelOption(std::string n, MousewheelOptionType t, uint16_t def)
	   : internal_name_(n), type_(t), default_(def) {
	}

public:
	static inline MousewheelOption create_mod(std::string n, uint16_t def) {
		return MousewheelOption(n, MousewheelOptionType::kKeymod, def);
	}
	static inline MousewheelOption create_bool(std::string n, bool def) {
		return MousewheelOption(n, MousewheelOptionType::kBool, def ? 1 : 0);
	}

	bool get_bool() const {
		assert(type_ == MousewheelOptionType::kBool);
		const bool def = (default_ != 0);
		if (internal_name_.empty()) {
			return def;
		}
		return get_config_bool("mousewheel", internal_name_, def);
	}
	uint16_t get_keymod() const {
		assert(type_ == MousewheelOptionType::kKeymod);
		if (internal_name_.empty()) {
			return default_;
		}
		return get_config_int("mousewheel", internal_name_, default_);
	}
	void set_bool(bool value) const {
		assert((type_ == MousewheelOptionType::kBool) && !(internal_name_.empty()));
		set_config_bool("mousewheel", internal_name_, value);
	}
	void set_keymod(uint16_t value) const {
		assert((type_ == MousewheelOptionType::kKeymod) && !(internal_name_.empty()));
		set_config_int("mousewheel", internal_name_, value);
	}
	void reset() const {
		if (!(internal_name_.empty())) {
			if (type_ == MousewheelOptionType::kBool) {
				set_bool(default_ != 0);
			} else {
				assert(type_ == MousewheelOptionType::kKeymod);
				set_keymod(default_);
			}
		}
	}
};

using Sign2D = Vector2i;

struct MousewheelHandlerOptions {
	const MousewheelOptionID keymod_id_;
	uint16_t current_keymod_ = KMOD_NONE;
	const MousewheelOptionID use_x_;
	const MousewheelOptionID invert_x_;
	const int32_t default_sign_x_;
	int32_t current_sign_x_ = 0;
	const MousewheelOptionID use_y_;
	const MousewheelOptionID invert_y_;
	const int32_t default_sign_y_;
	int32_t current_sign_y_ = 0;

	MousewheelHandlerOptions(MousewheelOptionID keymod_id,
	                         MousewheelOptionID use_x,
	                         MousewheelOptionID invert_x,
	                         MousewheelOptionID use_y,
	                         MousewheelOptionID invert_y,
	                         Sign2D def_signs)
	   : keymod_id_(keymod_id),
	     use_x_(use_x),
	     invert_x_(invert_x),
	     default_sign_x_(def_signs.x),
	     use_y_(use_y),
	     invert_y_(invert_y),
	     default_sign_y_(def_signs.y) {
	}

	void update_settings() {
		current_keymod_ = get_mousewheel_keymod(keymod_id_);
		if (get_mousewheel_option_bool(use_x_)) {
			current_sign_x_ =
			   default_sign_x_ * (get_mousewheel_option_bool(invert_x_) ? -1 : 1) *
			   (get_mousewheel_option_bool(MousewheelOptionID::kInvertedXDetected) !=
			          get_mousewheel_option_bool(MousewheelOptionID::kOverrideInvertedX) ?
                -1 :
                1);
		} else {
			current_sign_x_ = 0;
		}
		if (get_mousewheel_option_bool(use_y_)) {
			current_sign_y_ = default_sign_y_ * (get_mousewheel_option_bool(invert_y_) ? -1 : 1);
		} else {
			current_sign_y_ = 0;
		}
	}

	bool can_handle(const int32_t x, const int32_t y, const uint16_t modstate) const {
		return ((((y != 0) && (current_sign_y_ != 0)) || ((x != 0) && (current_sign_x_ != 0))) &&
		        matches_keymod(current_keymod_, modstate));
	}
	int32_t get_change(const int32_t x, const int32_t y, const uint16_t modstate) const {
		if (can_handle(x, y, modstate)) {
			return (x * current_sign_x_ + y * current_sign_y_);
		}
		return 0;
	}
	Vector2i get_change_2D(const int32_t x, const int32_t y, const uint16_t modstate) const {
		if (can_handle(x, y, modstate)) {
			return Vector2i(x * current_sign_x_, y * current_sign_y_);
		}
		return Vector2i(0, 0);
	}
};

static const std::map<MousewheelOptionID, MousewheelOption> mousewheel_options = {
   {MousewheelOptionID::kUIChangeValueInvertX,
    MousewheelOption::create_bool("change_value_x_invert", false)},
   {MousewheelOptionID::kUIChangeValueInvertY,
    MousewheelOption::create_bool("change_value_y_invert", false)},

   {MousewheelOptionID::kUITabInvertX, MousewheelOption::create_bool("tabpanel_x_invert", false)},
   {MousewheelOptionID::kUITabInvertY, MousewheelOption::create_bool("tabpanel_y_invert", false)},

   {MousewheelOptionID::kMapZoomMod, MousewheelOption::create_mod("zoom_modifier", KMOD_NONE)},
   {MousewheelOptionID::kMapZoomX,  //
    MousewheelOption::create_bool("zoom_x", true)},
   {MousewheelOptionID::kMapZoomY,  //
    MousewheelOption::create_bool("zoom_y", true)},
   {MousewheelOptionID::kMapZoomInvertX, MousewheelOption::create_bool("zoom_x_invert", false)},
   {MousewheelOptionID::kMapZoomInvertY, MousewheelOption::create_bool("zoom_y_invert", false)},

   {MousewheelOptionID::kMapScrollMod,
    MousewheelOption::create_mod("move_map_modifier", KMOD_NONE)},
   {MousewheelOptionID::kMapScroll, MousewheelOption::create_bool("move_map", false)},

   {MousewheelOptionID::kGameSpeedMod,
    MousewheelOption::create_mod("gamespeed_modifier", KMOD_ALT)},
   {MousewheelOptionID::kGameSpeedX, MousewheelOption::create_bool("gamespeed_x", true)},
   {MousewheelOptionID::kGameSpeedY, MousewheelOption::create_bool("gamespeed_y", true)},

   {MousewheelOptionID::kEditorToolsizeMod,
    MousewheelOption::create_mod("editor_toolsize_modifier", KMOD_ALT)},
   {MousewheelOptionID::kEditorToolsizeX, MousewheelOption::create_bool("editor_toolsize_x", true)},
   {MousewheelOptionID::kEditorToolsizeY, MousewheelOption::create_bool("editor_toolsize_y", true)},

   {MousewheelOptionID::kAlwaysOn, MousewheelOption::create_bool("", true)},
   {MousewheelOptionID::kDisabled, MousewheelOption::create_bool("", false)},
   {MousewheelOptionID::kNoMod, MousewheelOption::create_mod("", KMOD_NONE)},

   {MousewheelOptionID::kInvertedXDetected,  //
    MousewheelOption::create_bool("last_autoinvert_x", false)},
   {MousewheelOptionID::kOverrideInvertedX,  //
    MousewheelOption::create_bool("override_inverted_x_detection", false)},
};

// Default signs
constexpr int32_t kSignIncreaseRight = 1;
constexpr int32_t kSignIncreaseUp = 1;
constexpr int32_t kSignNextRight = 1;
constexpr int32_t kSignNextDown = -1;
constexpr int32_t kSignScrollX = 1;
constexpr int32_t kSignScrollY = -1;

static const Sign2D kDefaultSignValue(kSignIncreaseRight, kSignIncreaseUp);
static const Sign2D kDefaultSignMove(kSignNextRight, kSignNextDown);
static const Sign2D kDefaultSignScroll(kSignScrollX, kSignScrollY);

static std::map<MousewheelHandlerConfigID, MousewheelHandlerOptions> mousewheel_handlers = {
   {MousewheelHandlerConfigID::kChangeValue,
    MousewheelHandlerOptions(MousewheelOptionID::kNoMod,
                             MousewheelOptionID::kAlwaysOn,
                             MousewheelOptionID::kUIChangeValueInvertX,
                             MousewheelOptionID::kAlwaysOn,
                             MousewheelOptionID::kUIChangeValueInvertY,
                             kDefaultSignValue)},
   {MousewheelHandlerConfigID::kTabBar,  //
    MousewheelHandlerOptions(MousewheelOptionID::kNoMod,
                             MousewheelOptionID::kAlwaysOn,
                             MousewheelOptionID::kUITabInvertX,
                             MousewheelOptionID::kAlwaysOn,
                             MousewheelOptionID::kUITabInvertY,
                             kDefaultSignMove)},
   {MousewheelHandlerConfigID::kZoom,  //
    MousewheelHandlerOptions(MousewheelOptionID::kMapZoomMod,
                             MousewheelOptionID::kMapZoomX,
                             MousewheelOptionID::kMapZoomInvertX,
                             MousewheelOptionID::kMapZoomY,
                             MousewheelOptionID::kMapZoomInvertY,
                             kDefaultSignValue)},
   {MousewheelHandlerConfigID::kMapScroll,  //
    MousewheelHandlerOptions(MousewheelOptionID::kMapScrollMod,
                             MousewheelOptionID::kMapScroll,
                             MousewheelOptionID::kDisabled,  // always use system scroll direction
                             MousewheelOptionID::kMapScroll,
                             MousewheelOptionID::kDisabled,  // always use system scroll direction
                             kDefaultSignScroll)},
   {MousewheelHandlerConfigID::kGameSpeed,  //
    MousewheelHandlerOptions(MousewheelOptionID::kGameSpeedMod,
                             MousewheelOptionID::kGameSpeedX,
                             MousewheelOptionID::kUIChangeValueInvertX,
                             MousewheelOptionID::kGameSpeedY,
                             MousewheelOptionID::kUIChangeValueInvertY,
                             kDefaultSignValue)},
   {MousewheelHandlerConfigID::kEditorToolsize,
    MousewheelHandlerOptions(MousewheelOptionID::kEditorToolsizeMod,
                             MousewheelOptionID::kEditorToolsizeX,
                             MousewheelOptionID::kUIChangeValueInvertX,
                             MousewheelOptionID::kEditorToolsizeY,
                             MousewheelOptionID::kUIChangeValueInvertY,
                             kDefaultSignValue)},
   {MousewheelHandlerConfigID::kScrollbarVertical,  //
    MousewheelHandlerOptions(MousewheelOptionID::kNoMod,
                             MousewheelOptionID::kDisabled,
                             MousewheelOptionID::kDisabled,  // always use system scroll direction
                             MousewheelOptionID::kAlwaysOn,
                             MousewheelOptionID::kDisabled,  // always use system scroll direction
                             kDefaultSignScroll)}

};

void set_mousewheel_option_bool(const MousewheelOptionID opt_id, bool value) {
	assert(mousewheel_options.at(opt_id).type_ == MousewheelOptionType::kBool);
	mousewheel_options.at(opt_id).set_bool(value);
}
bool get_mousewheel_option_bool(const MousewheelOptionID opt_id) {
	assert(mousewheel_options.at(opt_id).type_ == MousewheelOptionType::kBool);
	return mousewheel_options.at(opt_id).get_bool();
}

void set_mousewheel_keymod(const MousewheelOptionID opt_id, uint16_t keymod) {
	assert(mousewheel_options.at(opt_id).type_ == MousewheelOptionType::kKeymod);
	mousewheel_options.at(opt_id).set_keymod(keymod);
}
uint16_t get_mousewheel_keymod(const MousewheelOptionID opt_id) {
	assert(mousewheel_options.at(opt_id).type_ == MousewheelOptionType::kKeymod);
	return mousewheel_options.at(opt_id).get_keymod();
}

int32_t get_mousewheel_change(MousewheelHandlerConfigID handler_id,
                              int32_t x,
                              int32_t y,
                              uint16_t modstate) {
	return mousewheel_handlers.at(handler_id).get_change(x, y, modstate);
}

Vector2i get_mousewheel_change_2D(MousewheelHandlerConfigID handler_id,
                                  int32_t x,
                                  int32_t y,
                                  uint16_t modstate) {
	return mousewheel_handlers.at(handler_id).get_change_2D(x, y, modstate);
}

void update_mousewheel_settings() {
	for (MousewheelHandlerConfigID i = MousewheelHandlerConfigID::k_Begin;
	     i <= MousewheelHandlerConfigID::k_End;
	     i = static_cast<MousewheelHandlerConfigID>(static_cast<uint16_t>(i) + 1)) {
		mousewheel_handlers.at(i).update_settings();
	}
}

void reset_mousewheel_settings() {
	for (MousewheelOptionID i = MousewheelOptionID::k_Begin; i <= MousewheelOptionID::k_End;
	     i = static_cast<MousewheelOptionID>(static_cast<uint16_t>(i) + 1)) {
		mousewheel_options.at(i).reset();
	}
	update_mousewheel_settings();
}
