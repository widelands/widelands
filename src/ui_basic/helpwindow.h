/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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

#ifndef UI_HELPWINDOW_H
#define UI_HELPWINDOW_H

#include "align.h"
#include "m_signal.h"
#include "multilinetextarea.h"
#include "window.h"

#include <boost/scoped_ptr.hpp>

namespace UI {

/**
 * Shows a help window.
 *
 * Using it is quite straightforward. To ensure, that all help windows have the
 * same formations, all richtext formating will be done via add_* functions:
 *     HelpWindow help(parent, "Caption", fontsize);
 *     help.add_heading("A minor heading");
 *     help.add_block("Some lines of text!");
 *     help.add_block("More text, just one linebreak between");
 *     help.add_paragraph("Even more text, now drawn in a new paragraph");
 *     help.add_block("More text, same paragraph, but a linebreak between!");
 *     help.add_heading("Another minor heading");
 *     ...
 *     help.run();
*/
struct HelpWindow : public Window {
	HelpWindow
		(Panel * parent,
		 const std::string & caption,
		 uint32_t fontsize,
		 uint32_t width = 0, uint32_t height = 0);
	~HelpWindow();

	bool handle_mousepress  (Uint8 btn, int32_t mx, int32_t my);
	bool handle_mouserelease(Uint8 btn, int32_t mx, int32_t my);

	void add_heading(std::string);
	void add_paragraph(std::string);
	void add_block(std::string);

protected:
	virtual void pressedOk();

private:
	enum State {
		BLOCK = 0,
		HEADING = 1
	} lastentry;

	Multiline_Textarea * textarea;
	std::string const m_h1, m_h2, m_p; // font sizes
	std::string const m_fn; // font name
	std::string m_text;
};

}

#endif
