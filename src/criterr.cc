/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include "graphic.h"
#include "font.h"
#include "ui.h"
#include "cursor.h"


/*
==============================================================================

Critical_Error

==============================================================================
*/

enum {
	ce_continue,
	ce_exit
};

class Critical_Error : public Panel {
public:
	Critical_Error(const char *text);

	void exit() { ::exit(-1); }

	void draw(Bitmap *dst, int ofsx, int ofsy);
};

Critical_Error::Critical_Error(const char *text)
	: Panel(0, 0, 0, g_gr.get_xres(), g_gr.get_yres())
{
	// Text
	new Textarea(this, g_gr.get_xres()/2, 150, "!! CRITICAL ERROR !!", Textarea::H_CENTER);
	new Textarea(this, g_gr.get_xres()/2, 200, text, Textarea::H_CENTER);

	// Buttons
	Button *b;

	b = new Button(this, (g_gr.get_xres()/2)-85, g_gr.get_yres()-200, 174, 24, 1);
	b->clicked.set(this, &Critical_Error::exit);
	b->set_pic(g_fh.get_string("Exit", 0));

	b = new Button(this, (g_gr.get_xres()/2)-85, g_gr.get_yres()-250, 174, 24, 1);
	b->clickedid.set(this, &Critical_Error::end_modal);
	b->set_pic(g_fh.get_string("!! Continue execution !!", 0));
}

void Critical_Error::draw(Bitmap *dst, int ofsx, int ofsy)
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
	static int in_criterr = 0;

	char buf[1024];
	va_list va;

	va_start(va, str);
	vsnprintf(buf, sizeof(buf), str, va);
	va_end(va);

	log("Critical Error%s: %s\n", in_criterr ? " (recursive)" : "", buf);
	
	if (g_gr.get_state() != Graphic::STATE_OK || in_criterr)
	{
#ifdef WIN32
		MessageBox(NULL, buf, "Wide Lands", MB_ICONINFORMATION);
#endif
		exit(0);
	}
	else
	{
		in_criterr++;
		Critical_Error ce(buf);
		ce.run();
		in_criterr--;
	}
}
