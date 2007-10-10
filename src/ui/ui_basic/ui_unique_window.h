/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__UNIQUE_WINDOW_H
#define __S__UNIQUE_WINDOW_H

#include "ui_window.h"

namespace UI {
struct Panel;


/**
  can only be created once, when it is requested to
  open a second one, it will implicitly kill the old one
  */
struct UniqueWindow : public Window {

	struct Registry {
		UniqueWindow * window;
		int32_t x, y;

		Registry() : window(0), x(-1), y(-1) {}
		~Registry();
	};

      UniqueWindow(Panel* parent, Registry* reg, int32_t w, int32_t h, std::string title);
	virtual ~UniqueWindow();

	bool get_usedefaultpos() {return m_usedefaultpos;}

private:
	Registry * m_registry;
	bool       m_usedefaultpos;
};
};

#endif
