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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "graphic.h"
#include "i18n.h"
#include "profile.h"
#include "rendertarget.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_panel.h"

#ifdef WIN32
#include <windows.h>
#endif

/*
==============================================================================

Critical_Error

==============================================================================
*/

enum {
	ce_continue,
	ce_exit
};

class Critical_Error : public UIPanel {
public:
	Critical_Error(const char *text);

	void exit() { ::exit(-1); }
	void crash();

	void draw(RenderTarget* dst);
};

Critical_Error::Critical_Error(const char *text)
	: UIPanel(0, 0, 0, g_gr->get_xres(), g_gr->get_yres())
{
	// Text
	new UITextarea(this, g_gr->get_xres()/2, 150, _("!! CRITICAL ERROR !!"), Align_HCenter);
	new UITextarea(this, g_gr->get_xres()/2, 200, text, Align_HCenter);

	// UIButtons
	UIButton *b;

	b = new UIButton(this, (g_gr->get_xres()/2)-85, g_gr->get_yres()-200, 174, 24, 1);
	b->clicked.set(this, &Critical_Error::exit);
	b->set_title(_("Exit"));

	b = new UIButton(this, (g_gr->get_xres()/2)-85, g_gr->get_yres()-250, 174, 24, 1);
	b->clickedid.set(this, &Critical_Error::end_modal);
	b->set_title(_("!! Continue execution !!"));

	Section *s = g_options.pull_section("global");

	if(s->get_bool("coredump", false)) {
		b = new UIButton(this, (g_gr->get_xres()/2)-85, g_gr->get_yres()-100, 174, 24, 1);
		b->clicked.set(this, &Critical_Error::crash);
		b->set_title(_("Crash"));
	}
}

void Critical_Error::draw(RenderTarget* dst)
{
	dst->fill_rect(0, 0, get_w(), get_h(), RGBColor(0, 0, 0));
}

void Critical_Error::crash()
{
	log("Trigger a segmentation fault\n");
	abort();
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

	if (in_criterr || !g_gr)
	{
#ifdef WIN32
		MessageBox(NULL, buf, _("Widelands"), MB_ICONINFORMATION);
#endif

		if (in_criterr <= 1)
		{
			Section *s = g_options.pull_section("global");

			if(s->get_bool("coredump", false))
			{
				log("Trigger a segmentation fault\n");
				abort();
			}
		}

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
