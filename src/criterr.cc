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

#include <stdarg.h>

#include "criterr.h"
#include "graphic.h"
#include "input.h"
#include "ui.h"
#include "cursor.h"
#include "font.h"
#include "menuecommon.h"

/*
==============================================================================

CriticalError

==============================================================================
*/

enum {
	ce_continue,
	ce_exit
};

class CriticalError : public Panel {
public:
	CriticalError(const char *text);

	void exit() { ::exit(-1); }

	void draw(Bitmap *dst, int ofsx, int ofsy);
};

CriticalError::CriticalError(const char *text)
	: Panel(0, 0, 0, g_gr.get_xres(), g_gr.get_yres())
{
	// Text
	new Textarea(this, g_gr.get_xres()/2, 150, "!! CRITICAL ERROR !!", Textarea::H_CENTER);
	new Textarea(this, g_gr.get_xres()/2, 200, text, Textarea::H_CENTER);

	// Buttons
	Button *b;

	b = new Button(this, (g_gr.get_xres()/2)-85, g_gr.get_yres()-200, 174, 24, 1);
	b->clicked.set(this, &CriticalError::exit);
	b->set_pic(g_fh.get_string("Exit", 0));

	b = new Button(this, (g_gr.get_xres()/2)-85, g_gr.get_yres()-250, 174, 24, 1);
	b->clickedid.set(this, &CriticalError::end_modal);
	b->set_pic(g_fh.get_string("!! Continue execution !!", 0));
}

void CriticalError::draw(Bitmap *dst, int ofsx, int ofsy)
{
	dst->fill_rect(ofsx, ofsy, get_w(), get_h(), 0);
}

/** void critical_error(const char* str)
 *
 * this is a critical error in the program. It will quite surely die.
 *
 * Args:	str			String to give to the user
 * Returns:	nothing, but may die (through exit())
 */
void critical_error(const char* str, ...)
{
	CriticalError *ce;
	char buf[1024];
	va_list va;

	va_start(va, str);
	vsnprintf(buf, sizeof(buf), str, va);
	va_end(va);

	ce = new CriticalError(buf);
	ce->run();
	delete ce;
}
