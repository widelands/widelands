/*
 * Copyright (C) 2002 Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands.h"
#include "ui.h"

/*
==============================================================================

Statebox

==============================================================================
*/

AutoPic Statebox::_gr("checkbox.bmp", STATEBOX_WIDTH*2, STATEBOX_HEIGHT);
ushort Statebox::dflt_highlightcolor;

/** Statebox::setup_ui() [static]
 *
 * Initialize default colors; called once by setup_ui
 */
void Statebox::setup_ui()
{
	dflt_highlightcolor = pack_rgb(100, 100, 80);
}

/** Statebox::Statebox(Panel *parent, int x, int y)
 *
 * Initialize a Statebox.
 * Stateboxs start out enabled and unchecked
 */
Statebox::Statebox(Panel *parent, int x, int y)
	: Panel(parent, x, y, STATEBOX_WIDTH, STATEBOX_HEIGHT)
{
	_highlighted = false;
	_enabled = true;
	_state = false;
}

/** Statebox::~Statebox()
 *
 * Clean up resources
 */
Statebox::~Statebox()
{
}

/** Statebox::set_enabled(bool enabled)
 *
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked.
 *
 * Args: enabled	true if the checkbox should be disabled
 */
void Statebox::set_enabled(bool enabled)
{
	_enabled = enabled;
	if (!_enabled)
		_highlighted = false;
	update(0, 0, get_w(), get_h());
}

/** Statebox::set_state(bool on)
 *
 * Changes the state of the checkbox.
 *
 * Args: on		true if the checkbox should be checked
 */
void Statebox::set_state(bool on)
{
	if (on == _state)
		return;

	_state = on;
	changed.call();
	changedto.call(on);
	update(0, 0, get_w(), get_h());
}

/** Statebox::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the entire checkbox
 */
void Statebox::draw(Bitmap *dst, int ofsx, int ofsy)
{
	int x;

	if (_state)
		x = STATEBOX_WIDTH;
	else
		x = 0;
	copy_pic(dst, &_gr, ofsx, ofsy, x, 0, STATEBOX_WIDTH, STATEBOX_HEIGHT);

	if (_highlighted)
	{
		dst->fill_rect(ofsx, ofsy, get_w(), 1, dflt_highlightcolor);
		dst->fill_rect(ofsx, ofsy, 1, get_h(), dflt_highlightcolor);
		dst->fill_rect(ofsx, ofsy+get_h()-1, get_w(), 1, dflt_highlightcolor);
		dst->fill_rect(ofsx+get_w()-1, ofsy, 1, get_h(), dflt_highlightcolor);
	}
}

/** Statebox::handle_mousein(bool inside)
 *
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool inside)
{
	_highlighted = inside;
	update(0, 0, get_w(), get_h());
}

/** Statebox::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0)
		return false;

	if (down) {
		if (_enabled)
			clicked();
	}

	return true;
}

/*
==============================================================================

Checkbox

==============================================================================
*/

/*
A checkbox only differs from a Statebox in that clicking on it toggles the
state
*/

/** Checkbox::clicked()
 *
 * Toggle the checkbox state
 */
void Checkbox::clicked()
{
	set_state(!get_state());
}
