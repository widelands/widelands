/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#include "ui_fsmenu/mousewheel_options.h"

#include <list>

#include <SDL_keycode.h>
#include <boost/format.hpp>

#include "base/i18n.h"
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

// clang-format off
constexpr int
	kButtonSize = 24,
	kDividerSpace = 8,
	kDirDdMaxW = 200,
	kResetBtnMaxW = 300;
// clang-format on

// Scroll Directions
enum SD : uint8_t {
	kDisabled = 0,
	kNeither = kDisabled,
	kVertical = 1,
	kHorizontal = 2,
	kAny = kVertical | kHorizontal,
	kBoth = kAny
};

static const std::string sd_names[] = {gettext_noop("Disabled"), gettext_noop("Vertical scroll"),
                                       gettext_noop("Horizontal scroll"),
                                       gettext_noop("Any scroll")};

#define READ_MOD(option)                                                                           \
	normalize_keymod(get_mousewheel_keymod(MousewheelOptionID::option##Mod))

#define DIR_COMBINE(x, y)                                                                          \
	((x ? SD::kHorizontal : SD::kDisabled) | (y ? SD::kVertical : SD::kDisabled))
#define READ_DIR(option)                                                                           \
	DIR_COMBINE(get_mousewheel_option_bool(MousewheelOptionID::option##X),                         \
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
#undef DIR_COMBINE

#define DIR_X(dir) ((dir & SD::kHorizontal) != 0)
#define DIR_Y(dir) ((dir & SD::kVertical) != 0)
#define APPLY_DIR(option, dir)                                                                     \
	set_mousewheel_option_bool(MousewheelOptionID::option##X, DIR_X(dir));                          \
	set_mousewheel_option_bool(MousewheelOptionID::option##Y, DIR_Y(dir));

void MousewheelConfigSettings::apply() {
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
#undef DIR_X
#undef DIR_Y

KeymodDropdown::KeymodDropdown(UI::Panel* parent)
   : UI::Dropdown<uint16_t>(parent,
                            std::string(),
                            0,
                            0,
                            kDirDdMaxW,
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
                           kDirDdMaxW,
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
                           kDirDdMaxW,
                           4,
                           kButtonSize,
                           std::string(),
                           UI::DropdownType::kTextual,
                           UI::PanelStyle::kFsMenu,
                           UI::ButtonStyle::kFsMenuMenu) {
	add(_("Neither"), SD::kNeither);
	add(_("Vertical"), SD::kVertical);
	add(_("Horizontal"), SD::kHorizontal);
	add(_("Both"), SD::kBoth);
}

KeymodAndDirBox::KeymodAndDirBox(UI::Panel* parent,
                                 const std::string& title,
                                 const std::list<KeymodAndDirBox*> shared_scope_list,
                                 uint16_t* keymod,
                                 uint8_t* dir,
                                 bool two_d)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     title_area_(this,
                 UI::PanelStyle::kFsMenu,
                 UI::FontStyle::kFsMenuLabel,
                 (boost::format(_("%1%:")) % title).str()),
     keymod_dropdown_(this),
     dir_dropdown_(this, two_d),
     title_(title),
     shared_scope_list_(shared_scope_list),
     keymod_(keymod),
     dir_(dir) {
	add(&title_area_, Resizing::kFillSpace);
	add(&keymod_dropdown_, Resizing::kAlign, UI::Align::kRight);
	add(&dir_dropdown_, Resizing::kAlign, UI::Align::kRight);
	update_sel();
	keymod_dropdown_.selected.connect([this]() {
		// Doesn't close before the selected signal. Bug?
		keymod_dropdown_.set_list_visibility(false);
		if (check_available(keymod_dropdown_.get_selected(), *dir_)) {
			*keymod_ = keymod_dropdown_.get_selected();
		} else {
			keymod_dropdown_.select(*keymod_);
		}
	});
	dir_dropdown_.selected.connect([this]() {
		// Doesn't close before the selected signal. Bug?
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
			   get_parent(), UI::WindowStyle::kFsMenu, _("Scroll Setting Conflict"),
			   as_richtext_paragraph(
			      (boost::format(_("‘%1$s%2$s’ conflicts with ‘%3$s’. "
			                       "Please select a different combination or "
			                       "change the conflicting setting first.")) %
			       keymod_string_for(keymod) % _(sd_names[dir]) % other->get_title())
			         .str(),
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
		const int butt_w = std::min(kDirDdMaxW, w / 3 - kPadding);
		keymod_dropdown_.set_desired_size(butt_w, kButtonSize);
		dir_dropdown_.set_desired_size(butt_w, kButtonSize);
	}
	set_desired_size(w, kButtonSize);
}

InvertDirBox::InvertDirBox(UI::Panel* parent, const std::string& title, uint8_t* dir)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     title_area_(this, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, title),
     dir_dropdown_(this),
     dir_(dir) {
	add(&title_area_, Resizing::kFillSpace);
	add(&dir_dropdown_, Resizing::kAlign, UI::Align::kRight);
	update_sel();
	dir_dropdown_.selected.connect([this, dir]() { *dir = dir_dropdown_.get_selected(); });
}
void InvertDirBox::update_sel() {
	dir_dropdown_.select(*dir_);
}
void InvertDirBox::set_width(int w) {
	if (w > 3 * kPadding) {
		const int butt_w = std::min(kDirDdMaxW, w / 3 - kPadding);
		dir_dropdown_.set_desired_size(butt_w, kButtonSize);
	}
	set_desired_size(w, kButtonSize);
}

DefaultsBox::DefaultsBox(MousewheelOptionsDialog* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, kButtonSize, kPadding),
     reset_button_(this,
                   std::string(),
                   0,
                   0,
                   kResetBtnMaxW,
                   kButtonSize,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("Reset all")) {
	add_inf_space();
	add(&reset_button_, Resizing::kAlign, UI::Align::kCenter);

	reset_button_.sigclicked.connect([parent]() {
		reset_mousewheel_settings();
		parent->update_settings();
	});
}

MousewheelOptionsDialog::MousewheelOptionsDialog(UI::Panel* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, kPadding),
     settings_(),
     zoom_box_(this,
               _("Zoom Map"),
               {&mapscroll_box_, &speed_box_, &toolsize_box_},
               &(settings_.zoom_mod_),
               &(settings_.zoom_dir_)),
     mapscroll_box_(this,
                    _("Scroll Map"),
                    {&zoom_box_, &speed_box_, &toolsize_box_},
                    &(settings_.map_scroll_mod_),
                    &(settings_.enable_map_scroll_),
                    true),
     speed_box_(this,
                _("Change Game Speed"),
                {&zoom_box_, &mapscroll_box_},
                &(settings_.speed_mod_),
                &(settings_.speed_dir_)),
     toolsize_box_(this,
                   _("Change Editor Toolsize"),
                   {&zoom_box_, &mapscroll_box_},
                   &(settings_.toolsize_mod_),
                   &(settings_.toolsize_dir_)),
     zoom_invert_box_(
        this, _("Invert scroll direction for map zooming:"), &(settings_.zoom_invert_)),
     tab_invert_box_(
        this, _("Invert scroll direction for tab switching:"), &(settings_.tab_invert_)),
     value_invert_box_(
        this, _("Invert scroll direction for increase/decrease:"), &(settings_.value_invert_)),
     defaults_box_(this) {
	add(&zoom_box_);
	add(&mapscroll_box_);
	add(&speed_box_);
	add(&toolsize_box_);
	add_space(kDividerSpace);
	add(&zoom_invert_box_);
	add(&tab_invert_box_);
	add(&value_invert_box_);
	add_space(kDividerSpace);
	add(&defaults_box_);
}
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
void MousewheelOptionsDialog::set_width(int w) {
	if ((w > 0) && (w != get_w())) {
		zoom_box_.set_width(w);
		mapscroll_box_.set_width(w);
		speed_box_.set_width(w);
		toolsize_box_.set_width(w);
		zoom_invert_box_.set_width(w);
		tab_invert_box_.set_width(w);
		value_invert_box_.set_width(w);
	}
}
}  // namespace FsMenu
