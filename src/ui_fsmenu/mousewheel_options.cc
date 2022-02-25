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

#include "ui_fsmenu/mousewheel_options.h"

#include <list>

#include <SDL_keycode.h>

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/menu.h"
#include "wlapplication_mousewheel_options.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int kButtonSize = 24;
constexpr int kDividerSpace = 8;
constexpr int kModDirDropdownMaxWidth = 200;
constexpr int kMousewheelBoxMaxWidth = 700;

// Scroll Directions
enum SD : uint8_t {
	kDisabled = 0,
	kNeither = kDisabled,
	kVertical = 1,
	kHorizontal = 2,
	kAny = kVertical | kHorizontal,
	kBoth = kAny
};

static const std::string sd_names[] = {
   /** TRANSLATORS: Placeholder for scroll direction when function should not use scroll wheel */
   gettext_noop("Disabled"),
   /** TRANSLATORS: e.g. Ctrl+Vertical scroll */
   gettext_noop("Vertical scroll"),
   /** TRANSLATORS: e.g. Ctrl+Horizontal scroll */
   gettext_noop("Horizontal scroll"),
   /** TRANSLATORS: Used when a function works with either vertical or horizontal scrolling.
       It may be used in combination with modifier keys, e.g. "Ctrl+Any scroll" */
   gettext_noop("Any scroll")};

/**********************************************************/

#define READ_MOD(option) normalize_keymod(get_mousewheel_keymod(MousewheelOptionID::option##Mod))

inline uint8_t dir_combine(const bool x, const bool y) {
	return ((x ? SD::kHorizontal : SD::kDisabled) | (y ? SD::kVertical : SD::kDisabled));
}

#define READ_DIR(option)                                                                           \
	dir_combine(get_mousewheel_option_bool(MousewheelOptionID::option##X),                          \
	            get_mousewheel_option_bool(MousewheelOptionID::option##Y))

void MousewheelConfigSettings::read() {
	enable_map_scroll_ =
	   (get_mousewheel_option_bool(MousewheelOptionID::kMapScroll) ? SD::kAny : SD::kDisabled);
	zoom_mod_ = READ_MOD(kMapZoom);
	map_scroll_mod_ = READ_MOD(kMapScroll);
	speed_mod_ = READ_MOD(kGameSpeed);
	toolsize_mod_ = READ_MOD(kEditorToolsize);
	zoom_dir_ = READ_DIR(kMapZoom);
	speed_dir_ = READ_DIR(kGameSpeed);
	toolsize_dir_ = READ_DIR(kEditorToolsize);
	value_invert_ = READ_DIR(kUIChangeValueInvert);
	tab_invert_ = READ_DIR(kUITabInvert);
	zoom_invert_ = READ_DIR(kMapZoomInvert);
}

#undef READ_MOD
#undef READ_DIR

inline bool dir_x(const uint8_t dir) {
	return (dir & SD::kHorizontal) != 0;
}
inline bool dir_y(const uint8_t dir) {
	return (dir & SD::kVertical) != 0;
}

#define APPLY_DIR(option, dir)                                                                     \
	set_mousewheel_option_bool(MousewheelOptionID::option##X, dir_x(dir));                          \
	set_mousewheel_option_bool(MousewheelOptionID::option##Y, dir_y(dir));

void MousewheelConfigSettings::apply() const {
	set_mousewheel_option_bool(MousewheelOptionID::kMapScroll, enable_map_scroll_ != 0);
	set_mousewheel_keymod(MousewheelOptionID::kMapZoomMod, zoom_mod_);
	set_mousewheel_keymod(MousewheelOptionID::kMapScrollMod, map_scroll_mod_);
	set_mousewheel_keymod(MousewheelOptionID::kGameSpeedMod, speed_mod_);
	set_mousewheel_keymod(MousewheelOptionID::kEditorToolsizeMod, toolsize_mod_);
	APPLY_DIR(kMapZoom, zoom_dir_)
	APPLY_DIR(kGameSpeed, speed_dir_)
	APPLY_DIR(kEditorToolsize, toolsize_dir_)
	APPLY_DIR(kUIChangeValueInvert, value_invert_)
	APPLY_DIR(kUITabInvert, tab_invert_)
	APPLY_DIR(kMapZoomInvert, zoom_invert_)

	update_mousewheel_settings();
}

#undef APPLY_DIR

/**********************************************************/

KeymodDropdown::KeymodDropdown(UI::Panel* parent)
   : UI::Dropdown<uint16_t>(parent,
                            std::string(),
                            0,
                            0,
                            kModDirDropdownMaxWidth,
                            20,
                            kButtonSize,
                            std::string(),
                            UI::DropdownType::kTextual,
                            UI::PanelStyle::kFsMenu,
                            UI::ButtonStyle::kFsMenuMenu) {
	// Same order as in keymod_string_for(), otherwise the list gets messed up
	uint16_t mods[] = {KMOD_CTRL, KMOD_GUI, KMOD_ALT, KMOD_SHIFT};
	int nmods = 4;
	uint16_t combo;
	uint16_t allfour = KMOD_CTRL | KMOD_GUI | KMOD_ALT | KMOD_SHIFT;
	/** TRANSLATORS: Placeholder when no modifier key is used, e.g. "(plain) Horizontal scroll" */
	add(_("(plain)"), KMOD_NONE);
	for (int i = 0; i < nmods; ++i) {
		add(keymod_string_for(mods[i]), mods[i]);
	}
	for (int i = 0; i < nmods - 1; ++i) {
		for (int j = i + 1; j < nmods; ++j) {
			combo = mods[i] | mods[j];
			add(keymod_string_for(combo), combo);
		}
	}
	for (int i = nmods - 1; i >= 0; --i) {
		combo = allfour & ~mods[i];
		add(keymod_string_for(combo), combo);
	}
	add(keymod_string_for(allfour), allfour);
}

#define SDOPT(i) _(sd_names[SD::i]), SD::i

DirDropdown::DirDropdown(UI::Panel* parent, bool two_d)
   : UI::Dropdown<uint8_t>(parent,
                           std::string(),
                           0,
                           0,
                           kModDirDropdownMaxWidth,
                           4,
                           kButtonSize,
                           std::string(),
                           UI::DropdownType::kTextual,
                           UI::PanelStyle::kFsMenu,
                           UI::ButtonStyle::kFsMenuMenu) {
	add(SDOPT(kDisabled));
	if (!two_d) {
		add(SDOPT(kVertical));
		add(SDOPT(kHorizontal));
	}
	add(SDOPT(kAny));
}

#undef SDOPT

InvertDirDropdown::InvertDirDropdown(UI::Panel* parent)
   : UI::Dropdown<uint8_t>(parent,
                           std::string(),
                           0,
                           0,
                           kModDirDropdownMaxWidth,
                           4,
                           kButtonSize,
                           std::string(),
                           UI::DropdownType::kTextual,
                           UI::PanelStyle::kFsMenu,
                           UI::ButtonStyle::kFsMenuMenu) {
	/** TRANSLATORS: Used as: "Invert scroll direction: Neither" */
	add(_("Neither"), SD::kNeither);
	/** TRANSLATORS: Used as: "Invert scroll direction: Vertical" */
	add(_("Vertical"), SD::kVertical);
	/** TRANSLATORS: Used as: "Invert scroll direction: Horizontal" */
	add(_("Horizontal"), SD::kHorizontal);
	/** TRANSLATORS: Used as: "Invert scroll direction: Both" */
	add(_("Both"), SD::kBoth);
}

/**********************************************************/

KeymodAndDirBox::KeymodAndDirBox(UI::Panel* parent,
                                 const std::string& title,
                                 const std::list<KeymodAndDirBox*> shared_scope_list,
                                 uint16_t* keymod,
                                 uint8_t* dir,
                                 bool two_d)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     title_area_(
        this, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, format(_("%1%:"), title)),
     keymod_dropdown_(this),
     dir_dropdown_(this, two_d),
     title_(title),
     shared_scope_list_(shared_scope_list),
     keymod_(keymod),
     dir_(dir) {
	if (UI::g_fh->fontset()->is_rtl()) {
		add(&dir_dropdown_, Resizing::kAlign, UI::Align::kLeft);
		add(&keymod_dropdown_, Resizing::kAlign, UI::Align::kLeft);
		add_inf_space();
		add(&title_area_, Resizing::kAlign, UI::Align::kRight);
	} else {
		add(&title_area_, Resizing::kFillSpace, UI::Align::kLeft);
		add(&keymod_dropdown_, Resizing::kAlign, UI::Align::kRight);
		add(&dir_dropdown_, Resizing::kAlign, UI::Align::kRight);
	}
	update_sel();
	keymod_dropdown_.selected.connect([this]() {
		// Doesn't close before sending the selected signal. Bug?
		keymod_dropdown_.set_list_visibility(false);
		if (check_available(keymod_dropdown_.get_selected(), *dir_)) {
			*keymod_ = keymod_dropdown_.get_selected();
		} else {
			keymod_dropdown_.select(*keymod_);
		}
	});
	dir_dropdown_.selected.connect([this]() {
		// Doesn't close before sending the selected signal. Bug?
		dir_dropdown_.set_list_visibility(false);
		if (check_available(*keymod_, dir_dropdown_.get_selected())) {
			*dir_ = dir_dropdown_.get_selected();
			check_dir();
		} else {
			dir_dropdown_.select(*dir_);
		}
	});
}
void KeymodAndDirBox::update_sel() {
	keymod_dropdown_.select(*keymod_);
	dir_dropdown_.select(*dir_);
	check_dir();
}
void KeymodAndDirBox::check_dir() {
	if (*dir_ == 0) {
		keymod_dropdown_.set_enabled(false);
	} else {
		keymod_dropdown_.set_enabled(true);
	}
}
bool KeymodAndDirBox::conflicts(uint16_t keymod, uint8_t dir) {
	return (dir & *dir_) && (matches_keymod(keymod, *keymod_));
}
bool KeymodAndDirBox::check_available(uint16_t keymod, uint8_t dir) {
	for (KeymodAndDirBox* other : shared_scope_list_) {
		if (other->conflicts(keymod, dir)) {
			UI::WLMessageBox warning(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Scroll Setting Conflict"),
			   as_richtext_paragraph(
			      /** TRANSLATORS: %1 is a modifier key combination, e.g. "Ctrl+", or
			                         empty if none is used. %2 is scrolling direction.
			                         %3 is the name of the conflicting function. */
			      format(_("‘%1$s%2$s’ conflicts with ‘%3$s’. "
			               "Please select a different combination or "
			               "change the conflicting setting first."),
			             keymod_string_for(keymod), _(sd_names[dir]), other->get_title()),
			      UI::FontStyle::kFsMenuLabel, UI::Align::kCenter),
			   UI::WLMessageBox::MBoxType::kOk);
			warning.run<UI::Panel::Returncodes>();
			return false;
		}
	}
	return true;
}
void KeymodAndDirBox::set_width(int w) {
	if (w > 3 * kPadding) {
		const int butt_w = std::min(kModDirDropdownMaxWidth, w / 3 - kPadding);
		keymod_dropdown_.set_desired_size(butt_w, kButtonSize);
		dir_dropdown_.set_desired_size(butt_w, kButtonSize);
	}
	set_desired_size(w, kButtonSize);
}
/***** End of KeymodAndDirBox members *****/

InvertDirBox::InvertDirBox(UI::Panel* parent, const std::string& title, uint8_t* dir)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     title_area_(this, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, title),
     dir_dropdown_(this),
     dir_(dir) {
	if (UI::g_fh->fontset()->is_rtl()) {
		add(&dir_dropdown_, Resizing::kAlign, UI::Align::kLeft);
		add_inf_space();
		add(&title_area_, Resizing::kAlign, UI::Align::kRight);
	} else {
		add(&title_area_, Resizing::kFillSpace, UI::Align::kLeft);
		add(&dir_dropdown_, Resizing::kAlign, UI::Align::kRight);
	}
	update_sel();
	dir_dropdown_.selected.connect([this, dir]() { *dir = dir_dropdown_.get_selected(); });
}
void InvertDirBox::update_sel() {
	dir_dropdown_.select(*dir_);
}
void InvertDirBox::set_width(int w) {
	if (w > 3 * kPadding) {
		const int butt_w = std::min(kModDirDropdownMaxWidth, w / 3 - kPadding);
		dir_dropdown_.set_desired_size(butt_w, kButtonSize);
	}
	set_desired_size(w, kButtonSize);
}

ScrollOptionsButtonBox::ScrollOptionsButtonBox(MousewheelOptionsDialog* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     reset_button_(this,
                   std::string(),
                   0,
                   0,
                   0,
                   0,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("Reset Scrolling"),
                   _("Reset scroll settings to the defaults recommended for a standard single "
                     "wheel mouse")),
     touchpad_button_(this,
                      std::string(),
                      0,
                      0,
                      0,
                      0,
                      UI::ButtonStyle::kFsMenuSecondary,
                      _("Set for Touchpad"),
                      _("Sets ‘Zoom Map’ and ‘Scroll Map’ to the recommended settings for a "
                        "touchpad or other pointing device that can scroll horizontally as well "
                        "as vertically.")),
     apply_button_(this,
                   std::string(),
                   0,
                   0,
                   0,
                   0,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("Apply Scrolling"),
                   _("Apply scroll settings")) {
	add_inf_space();
	add(&reset_button_, Resizing::kAlign, UI::Align::kCenter);
	add_inf_space();
	add(&touchpad_button_, Resizing::kAlign, UI::Align::kCenter);
	add_inf_space();
	add(&apply_button_, Resizing::kAlign, UI::Align::kCenter);
	add_inf_space();

	reset_button_.sigclicked.connect([parent]() { parent->reset(); });
	touchpad_button_.sigclicked.connect([parent]() { parent->set_touchpad(); });
	apply_button_.sigclicked.connect([parent]() { parent->apply_settings(); });
}

/**********************************************************/

// The main scrolling options dialog box
MousewheelOptionsDialog::MousewheelOptionsDialog(UI::Panel* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     zoom_box_(this,
               /** TRANSLATORS: Name of a function for the scroll wheel.
                   Used as e.g.: "Zoom Map: Ctrl+Any scroll"
                   The ':' will be added by another format string. */
               _("Zoom Map"),
               {&mapscroll_box_, &speed_box_, &toolsize_box_},
               &(settings_.zoom_mod_),
               &(settings_.zoom_dir_)),
     mapscroll_box_(this,
                    /** TRANSLATORS: Name of a function for the scroll wheel.
                        Used as e.g.: "Scroll Map: Ctrl+Any scroll"
                        The ':' will be added by another format string. */
                    _("Scroll Map"),
                    {&zoom_box_, &speed_box_, &toolsize_box_},
                    &(settings_.map_scroll_mod_),
                    &(settings_.enable_map_scroll_),
                    true),
     speed_box_(this,
                /** TRANSLATORS: Name of a function for the scroll wheel.
                    Used as e.g.: "Change Game Speed: Ctrl+Any scroll"
                    The ':' will be added by another format string. */
                _("Change Game Speed"),
                {&zoom_box_, &mapscroll_box_},
                &(settings_.speed_mod_),
                &(settings_.speed_dir_)),
     toolsize_box_(this,
                   /** TRANSLATORS: Name of a function for the scroll wheel.
                       Used as e.g.: "Change Editor Toolsize: Ctrl+Any scroll"
                       The ':' will be added by another format string. */
                   _("Change Editor Toolsize"),
                   {&zoom_box_, &mapscroll_box_},
                   &(settings_.toolsize_mod_),
                   &(settings_.toolsize_dir_)),
     zoom_invert_box_(
        this,
        /** TRANSLATORS: Used as e.g. "Invert scroll direction for map zooming: Vertical" */
        _("Invert scroll direction for map zooming:"),
        &(settings_.zoom_invert_)),
     tab_invert_box_(
        this,
        /** TRANSLATORS: Used as e.g. "Invert scroll direction for tab switching: Vertical" */
        _("Invert scroll direction for tab switching:"),
        &(settings_.tab_invert_)),
     value_invert_box_(
        this,
        /** TRANSLATORS: Used as e.g. "Invert scroll direction for increase/decrease: Vertical" */
        _("Invert scroll direction for increase/decrease:"),
        &(settings_.value_invert_)),
     button_box_(this) {
	add(&zoom_box_);
	add(&mapscroll_box_);
	add(&speed_box_);
	add(&toolsize_box_);
	add_space(kDividerSpace);
	add(&zoom_invert_box_);
	add(&tab_invert_box_);
	add(&value_invert_box_);
	add_space(kDividerSpace);
	add(&button_box_);
}

// Functions for handling settings_
void MousewheelOptionsDialog::update_settings() {
	settings_.read();
	zoom_box_.update_sel();
	mapscroll_box_.update_sel();
	speed_box_.update_sel();
	toolsize_box_.update_sel();
	zoom_invert_box_.update_sel();
	tab_invert_box_.update_sel();
	value_invert_box_.update_sel();
}
void MousewheelOptionsDialog::apply_settings() {
	settings_.apply();
}
void MousewheelOptionsDialog::reset() {
	reset_mousewheel_settings();
	update_settings();
}

// Touchpad recommended settings
void MousewheelOptionsDialog::set_touchpad() {
	const bool conflict_speed =
	   speed_box_.conflicts(KMOD_NONE, SD::kAny) || speed_box_.conflicts(KMOD_CTRL, SD::kVertical);
	const bool conflict_toolsize = toolsize_box_.conflicts(KMOD_NONE, SD::kAny) ||
	                               toolsize_box_.conflicts(KMOD_CTRL, SD::kVertical);
	if (conflict_speed || conflict_toolsize) {
		UI::WLMessageBox warning(
		   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Scroll Settings Conflict"),
		   as_richtext_paragraph(format(_("‘%1$s’ or ‘%2$s’ conflicts with the recommended "
		                                  "settings. Change the conflicting setting(s) too?"),
		                                speed_box_.get_title(), toolsize_box_.get_title()),
		                         UI::FontStyle::kFsMenuLabel, UI::Align::kCenter),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (warning.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
		if (conflict_speed) {
			settings_.speed_mod_ = KMOD_ALT;
			speed_box_.update_sel();
		}
		if (conflict_toolsize) {
			settings_.toolsize_mod_ = KMOD_ALT;
			toolsize_box_.update_sel();
		}
	}

	// Set recommended values
	settings_.map_scroll_mod_ = KMOD_NONE;
	settings_.enable_map_scroll_ = SD::kAny;
	mapscroll_box_.update_sel();
	settings_.zoom_mod_ = KMOD_CTRL;
	settings_.zoom_dir_ = (speed_box_.conflicts(KMOD_CTRL, SD::kHorizontal) ||
	                       toolsize_box_.conflicts(KMOD_CTRL, SD::kHorizontal)) ?
                            SD::kVertical :
                            SD::kAny;
	zoom_box_.update_sel();
	apply_settings();
	update_settings();
}

// Set sizes for layouting
void MousewheelOptionsDialog::set_size(int w, int h) {
	if (w <= 0 || h <= 0) {
		return;
	}
	const int w_real = std::min(w, kMousewheelBoxMaxWidth);
	set_pos(Vector2i((w - w_real) / 2, get_y()));
	if (w_real != get_w()) {
		UI::Panel::set_size(w_real, h);
		const int w_hbox = w_real - 3 * kPadding;
		zoom_box_.set_width(w_hbox);
		mapscroll_box_.set_width(w_hbox);
		speed_box_.set_width(w_hbox);
		toolsize_box_.set_width(w_hbox);
		zoom_invert_box_.set_width(w_hbox);
		tab_invert_box_.set_width(w_hbox);
		value_invert_box_.set_width(w_hbox);
		button_box_.set_size(w_hbox, kButtonSize);
	}
}

}  // namespace FsMenu
