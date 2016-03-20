/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "ui_basic/radiobutton.h"

#include "ui_basic/checkbox.h"

namespace UI {

/**
 * Initialize the radiobutton and link it into the group's linked list
*/
Radiobutton::Radiobutton
	(Panel      * const parent,
	 Point        const p,
	 const Image* pic,
	 Radiogroup &       group,
	 int32_t      const id)
	:
	Statebox (parent, p, pic),
	nextbtn_(group.buttons_),
	group_  (group),
	id_     (id)
{
	group.buttons_ = this;
}

/**
 * Unlink the radiobutton from its group
 */
Radiobutton::~Radiobutton()
{
	for (Radiobutton * * pp = &group_.buttons_; *pp; pp = &(*pp)->nextbtn_) {
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
void Radiobutton::clicked()
{
	group_.set_state(id_);
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
Radiogroup::Radiogroup()
{
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
	// See https://bugs.launchpad.net/widelands/+bug/1198928
	while (buttons_) delete buttons_;
}


/**
 * Create a new radio button with the given attributes
 * Returns the ID of the new button.
*/
int32_t Radiogroup::add_button
	(Panel      * const parent,
	 Point        const p,
	 const Image* pic,
	 const std::string& tooltip,
	 Radiobutton **     ret_btn)
{
	++highestid_;
	Radiobutton * btn = new Radiobutton(parent, p, pic, *this, highestid_);
	btn->set_tooltip(tooltip);
	if (ret_btn) (*ret_btn) = btn;
	return highestid_;
}


/**
 * Change the state and set button states to reflect the change.
 *
 * Args: state  the ID of the checked button (-1 means don't check any button)
 */
void Radiogroup::set_state(int32_t const state) {
	if (state == state_) {
		clicked();
		return;
	}

	for (Radiobutton * btn = buttons_; btn; btn = btn->nextbtn_)
		btn->set_state(btn->id_ == state);
	state_ = state;
	changed();
	changedto(state);
}

/**
 * Disable this radiogroup
 */
void Radiogroup::set_enabled(bool st) {
	for (Radiobutton * btn = buttons_; btn; btn = btn->nextbtn_)
		btn->set_enabled(st);
}

}
