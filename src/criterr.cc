/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#include "error.h"
#include "graphic.h"
#include "i18n.h"
#include "profile.h"
#include "rendertarget.h"

#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_panel.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif

enum {
	ce_continue,
	ce_exit
};

class Critical_Error : public UI::Panel {
public:
	Critical_Error(const char *text);

	__attribute__ ((noreturn)) void exit() {::exit(-1);}
	__attribute__ ((noreturn)) void crash();

	void draw(RenderTarget* dst);
private:
	UI::Textarea title, message;
	UI::Button<Critical_Error> exit_button;
	UI::IDButton<Critical_Error, int> continue_button;

};

Critical_Error::Critical_Error(const char *text) :
UI::Panel(0, 0, 0, g_gr->get_xres(), g_gr->get_yres()),

	// Text
title
(this, g_gr->get_xres() / 2, 150, _("!! CRITICAL ERROR !!"), Align_HCenter),
message(this, g_gr->get_xres() / 2, 200, text, Align_HCenter),

	// UI::Buttons
exit_button
(this,
 g_gr->get_xres() / 2 - 85, g_gr->get_yres() - 200, 174, 24,
 1,
 &Critical_Error::exit, this,
 _("Exit")),

continue_button
(this,
 g_gr->get_xres() / 2 - 85, g_gr->get_yres() - 250, 174, 24,
 1,
 &Critical_Error::end_modal, this, 0,
 _("!! Continue execution !!"))

{
	if (g_options.pull_section("global")->get_bool("coredump", false)) {
		new UI::Button<Critical_Error>
			(this,
			 g_gr->get_xres() / 2 - 85, g_gr->get_yres() - 100, 174, 24,
			 1,
			 &Critical_Error::crash, this,
			 _("Crash"));
	}
}

void Critical_Error::draw(RenderTarget* dst)
{dst->fill_rect(Rect(Point(0, 0), get_w(), get_h()), RGBColor(0, 0, 0));}

void Critical_Error::crash()
{
	log("Trigger a segmentation fault\n");
	abort();
}


/**
 * A critical error occurred in the program. It will quite surely die.
 *
 * \param str String to give to the user
 */
void critical_error(const char* str, ...)
{
	static int in_criterr = 0;

	char buffer[1024];
	va_list va;

	va_start(va, str);
	vsnprintf(buffer, sizeof(buffer), str, va);
	va_end(va);

	log("Critical Error%s: %s\n", in_criterr ? " (recursive)" : "", buffer);

	if (in_criterr || !g_gr)
	{
#ifdef WIN32
		MessageBox(NULL, buffer, _("Widelands").c_str(), MB_ICONINFORMATION);
#endif

		if (in_criterr <= 1)
		{
			Section *s = g_options.pull_section("global");

			if (s->get_bool("coredump", false))
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
		Critical_Error ce(buffer);
		ce.run();
		in_criterr--;
	}
}
