/*
 * Copyright (C) 2002 by Holger Rapp
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

#ifndef __S__MENUECOMMON_H
#define __S__MENUECOMMON_H

#include "mytypes.h"
#include "ui.h"

#define MENU_XRES	640
#define MENU_YRES	480

class BaseMenu : public Panel {
	Pic bg;
public:
	BaseMenu(const char *bgpic);

	virtual void start();
	virtual void draw(Bitmap *dst, int ofsx, int ofsy);
};


#endif // __S__MENUECOMMON_H
