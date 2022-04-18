/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_basic/radiobutton.h"

#include "ui_basic/checkbox.h"

namespace UI {

/**
 * Initialize the radiobutton and link it into the group's linked list
 */
Radiobutton::Radiobutton(Panel* const parent,
                         PanelStyle s,
                         Vector2i const p,
                         const Image* pic,
                         Radiogroup& group,
                         int32_t const id)
   : Statebox(parent, s, p, pic), nextbtn_(group.buttons_), group_(group), id_(id) {
	group.buttons_ = this;
}

/**
 * Unlink the radiobutton from its group
 */
Radiobutton::~Radiobutton() {
	for (Radiobutton** pp = &group_.buttons_; *pp != nullptr; pp = &(*pp)->nextbtn_) {
		if (*pp == this) {
			*pp = nextbtn_;
			break;
		}
	}
}

/**
 * Inform the radiogroup about the click; the group is responsible of setting
 * button states.
 */
void Radiobutton::button_clicked() {
	group_.set_state(id_, true);
	play_click();
}

/*
==============================================================================

Radiogroup

==============================================================================
*/

/**
 * Initialize an empty radiogroup
 */
Radiogroup::Radiogroup() {
	buttons_ = nullptr;
	highestid_ = -1;
	state_ = -1;
}

/**
 * Free all associated buttons.
 */
Radiogroup::~Radiogroup() {
	// Scan-build claims this results in double free.
	// This is a false positive.
	// The reason is that the variable will be reassigned in the destructor of the deleted child.
	// This is very uncommon behavior and bad style, but will be non trivial to fix.
	while (buttons_ != nullptr) {
		delete buttons_;
	}
}

/**
 * Create a new radio button with the given attributes
 * Returns the ID of the new button.
 */
int32_t Radiogroup::add_button(Panel* const parent,
                               PanelStyle s,
                               Vector2i const p,
                               const Image* pic,
                               const std::string& tooltip,
                               Radiobutton** ret_btn) {
	++highestid_;
	Radiobutton* btn = new Radiobutton(parent, s, p, pic, *this, highestid_);
	btn->set_tooltip(tooltip);
	if (ret_btn != nullptr) {
		(*ret_btn) = btn;
	}
	return highestid_;
}

/**
 * Change the state and set button states to reflect the change.
 *
 * Args: state  the ID of the checked button (-1 means don't check any button)
 *       send_signal Whether to trigger the `clicked`, `changed` and `changedto` signals.
 */
void Radiogroup::set_state(int32_t const state, const bool send_signal) {
	if (state == state_) {
		if (send_signal) {
			clicked();
		}
		return;
	}

	for (Radiobutton* btn = buttons_; btn != nullptr; btn = btn->nextbtn_) {
		btn->set_state(btn->id_ == state);
	}
	state_ = state;
	if (send_signal) {
		changed();
		changedto(state);
	}
}

/**
 * Disable this radiogroup
 */
void Radiogroup::set_enabled(bool st) {
	for (Radiobutton* btn = buttons_; btn != nullptr; btn = btn->nextbtn_) {
		btn->set_enabled(st);
	}
}
}  // namespace UI
