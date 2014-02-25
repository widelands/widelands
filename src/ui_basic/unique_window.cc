/*
 * Copyright (C) 2002, 2006-2007, 2009 by the Widelands Development Team
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

#include "ui_basic/unique_window.h"

namespace UI {
/*
==============================================================================

UniqueWindow IMPLEMENTATION

==============================================================================
*/

/**
 * Creates the window, if it does not exist.
*/
void UniqueWindow::Registry::create() {
	if (not window) {
		constr();
	}
}

/**
 * Destroys the window, if it eixsts.
*/
void UniqueWindow::Registry::destroy() {
	if (window) {
		window->die();
	}
}

/**
 * Either destroys or creates the window.
*/
void UniqueWindow::Registry::toggle() {
	if (window) {
		window->die();
	} else {
		constr();
	}
}



/**
 * In order to avoid dangling pointers, we need to kill our contained window
 * here.
*/
UniqueWindow::Registry::~Registry() {delete window;}


/**
 * Register, position according to the registry information.
*/
UniqueWindow::UniqueWindow
	(Panel                  * const parent,
	 const std::string & name,
	 UniqueWindow::Registry * const reg,
	 int32_t const w, int32_t const h,
	 const std::string      & title)
	:
	Window         (parent, name, 0, 0, w, h, title.c_str()),
	m_registry     (reg),
	m_usedefaultpos(true)
{
	if (m_registry) {
		delete m_registry->window;

		m_registry->window = this;
		if (m_registry->valid_pos) {
			set_pos(Point(m_registry->x, m_registry->y));
			m_usedefaultpos = false;
		}
		if (m_registry->onCreate) {
			m_registry->onCreate();
		}
	}
}


/**
 * Unregister, save latest position.
*/
UniqueWindow::~UniqueWindow()
{
	if (m_registry) {
		assert(m_registry->window == this);

		m_registry->window = nullptr;
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->valid_pos = true;

		if (m_registry->onDelete) {
			m_registry->onDelete();
		}
	}
}

}
