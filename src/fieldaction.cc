/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#include "os.h"
#include "ui.h"
#include "fieldaction.h"
#include "map.h"
#include "IntPlayer.h"

class FieldActionWindow : public Window {
public:
	FieldActionWindow(Interactive_Player *plr, int fx, int fy, Window **registry);
	~FieldActionWindow();

private:
	Interactive_Player *_player;
	int _fx, _fy;
	Window **_registry;
};

/** FieldActionWindow(Interactive_Player *plr, int fx, int fy, Window **registry)
 *
 * Initialize a field action window
 *
 * Args: plr	the controlling player
 *       fx		referenced field coordinates
 *       fy
 */
FieldActionWindow::FieldActionWindow(Interactive_Player *plr, int fx, int fy, Window **registry)
	: Window(plr, 0, 0, 68, 34, "Action")
{
	_player = plr;
	_fx = fx;
	_fy = fy;
	_registry = registry;

	if (_registry)
		*_registry = this;

	// Buttons
	Button *b;

	b = new Button(this, 0, 0, 34, 34, 2);

	b = new Button(this, 34, 0, 34, 34, 2);
}

/** FieldActionWindow::~FieldActionWindow()
 *
 * Free allocated resources, remove from registry.
 */
FieldActionWindow::~FieldActionWindow()
{
	if (_registry)
		*_registry = 0;
}

/** show_field_action(Interactive_Player *parent, int fx, int fy, Window **registry)
 *
 * Bring up a field action window
 *
 * Args: parent	the interactive player
 *       fx		field coordinates
 *       fy
 *       registry pointer to ensure only one fieldaction window exists at a time
 */
void show_field_action(Interactive_Player *parent, int fx, int fy, Window **registry)
{
	if (registry) {
		if (*registry)
			delete *registry;
	}

	FieldActionWindow *w = new FieldActionWindow(parent, fx, fy, registry);
	w->move_to_mouse();
}
