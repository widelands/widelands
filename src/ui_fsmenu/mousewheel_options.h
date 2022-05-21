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

#ifndef WL_UI_FSMENU_MOUSEWHEEL_OPTIONS_H
#define WL_UI_FSMENU_MOUSEWHEEL_OPTIONS_H

#include <list>
#include <map>
#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"

namespace FsMenu {

struct MousewheelConfigSettings {
	// Handling of mousewheel events works on the focus follows mouse
	// basis, see: ui_basic/panel.cc, so there's not much room for
	// changing its behaviour for UI widgets. They can't even be passed
	// on to the main window easily, because all mousewheel events are
	// claimed in ui_basic/window.cc to avoid unexpected propagation
	// to underlying windows.
	// Therefore configuration is only needed for the actions in the
	// main game and editor windows.
	uint16_t zoom_mod_, map_scroll_mod_, speed_mod_, toolsize_mod_;
	uint8_t zoom_dir_, speed_dir_, toolsize_dir_;

	// Will be converted from/to bool to allow common handler
	uint8_t enable_map_scroll_;

	// We suppose that the user set the system configuration for
	// scrolling direction to their preference, so scrollbar and
	// map scrolling always uses that. However "natural" scrolling
	// inverts the direction of all scroll events, including those
	// where it should be absolute, and SDL does not always report
	// its setting correctly. Furthermore, user preferences may be
	// different for zoom direction, and for converting scroll
	// direction to movement in the orthogonal direction.
	uint8_t value_invert_, tab_invert_, zoom_invert_;

	// Earlier SDL versions on Linux reported X scrolling events
	// with inverted sign. We try to detect known buggy versions,
	// but if we get it wrong, this option allows it to be overridden.
	// TODO(tothxa): Consider removing this if there are no reports
	// of wrong detections for a release cycle
	bool inverted_x_;

	void read();
	void apply() const;

	MousewheelConfigSettings() {
		read();
	}
};

/**********************************************************/

class MousewheelOptionsDialog;

struct KeymodDropdown : public UI::Dropdown<uint16_t> {
	KeymodDropdown(UI::Panel* parent);
};

struct DirDropdown : public UI::Dropdown<uint8_t> {
	DirDropdown(Panel* parent, bool two_d = false);
};

struct InvertDirDropdown : public UI::Dropdown<uint8_t> {
	InvertDirDropdown(Panel* parent);
};

// Box for options of a single handler
struct KeymodAndDirBox : public UI::Box {
	KeymodAndDirBox(UI::Panel* parent,
	                const std::string& title,
	                const std::list<KeymodAndDirBox*> shared_scope_list,
	                uint16_t* keymod,
	                uint8_t* dir,
	                bool two_d = false);
	void update_sel();

	// Disable keymod dropdown when neither direction is used
	void check_dir();

	// Show info window if conflict is found then return false,
	// return true if there is no conflict
	bool check_available(uint16_t keymod, uint8_t dir);

	bool conflicts(uint16_t keymod, uint8_t dir);
	std::string get_title() {
		return title_;
	}

	void set_width(int w);

private:
	UI::Textarea title_area_;
	KeymodDropdown keymod_dropdown_;
	DirDropdown dir_dropdown_;
	const std::string title_;
	const std::list<KeymodAndDirBox*> shared_scope_list_;
	uint16_t* keymod_;
	uint8_t* dir_;
};

// Box for invert options
struct InvertDirBox : public UI::Box {
	InvertDirBox(UI::Panel* parent, const std::string& title, uint8_t* dir);

	void update_sel();
	void set_width(int w);

private:
	UI::Textarea title_area_;
	InvertDirDropdown dir_dropdown_;
	uint8_t* dir_;
};

struct ScrollOptionsButtonBox : public UI::Box {
	ScrollOptionsButtonBox(MousewheelOptionsDialog* parent);

private:
	UI::Button reset_button_;
	UI::Button touchpad_button_;
	UI::Button apply_button_;
};

/**********************************************************/

// Main dialog box
class MousewheelOptionsDialog : public UI::Box {
public:
	MousewheelOptionsDialog(UI::Panel* parent);

	void update_settings();
	void apply_settings();
	void reset();
	void set_touchpad();

	void set_size(int w, int h) override;

private:
	MousewheelConfigSettings settings_;

	// Option controls
	KeymodAndDirBox zoom_box_;
	KeymodAndDirBox mapscroll_box_;
	KeymodAndDirBox speed_box_;
	KeymodAndDirBox toolsize_box_;
	InvertDirBox zoom_invert_box_;
	InvertDirBox tab_invert_box_;
	InvertDirBox value_invert_box_;
	UI::Box horiz_override_box_;
	UI::Checkbox inverted_x_checkbox_;
	UI::Button feedback_button_;
	ScrollOptionsButtonBox button_box_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_MOUSEWHEEL_OPTIONS_H
