/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef UI_MESSAGEBOX_H
#define UI_MESSAGEBOX_H
#undef MessageBox //Don't ask, this is just because of Windows ;-)

#include <boost/scoped_ptr.hpp>

#include "font_handler.h"
#include "ui_signal.h"
#include "ui_window.h"

namespace UI {

struct MessageBoxImpl;

/**
 * Shows a standard messagebox. The message box can be used either as a modal
 * or as a non-modal dialog box.
 *
 * Using it as a modal dialog box is very straightforward:
 *     MessageBox mb(parent, "Caption", "Text", OK);
 *     int32_t code = mb.run();
 * The return code is 1 if the "Yes" button has been pressed in a \ref YESNO
 * dialog box. Otherwise, it is 0 (or negative, if the modal messagebox has
 * been interrupted in an unusual way).
 *
 * Using it as a non-modal dialog box is slightly more complicated. You have
 * to add this dialog box as a child to the current fullscreen panel, and
 * connect the signals \ref yes and \ref no or \ref ok, depending on the
 * messagebox type, to a function that deletes the message box.
*/
struct MessageBox : public Window {
	enum MB_Type {
		OK,
		YESNO
	};
	MessageBox
		(Panel * parent,
		 const std::string & caption,
		 const std::string & text,
		 MB_Type);
	~MessageBox();

	Signal ok;
	Signal yes;
	Signal no;

	void set_align(Align);

	bool handle_mousepress  (Uint8 btn, int32_t mx, int32_t my);
	bool handle_mouserelease(Uint8 btn, int32_t mx, int32_t my);

protected:
	virtual void pressedOk();
	virtual void pressedYes();
	virtual void pressedNo();

private:
	boost::scoped_ptr<MessageBoxImpl> d;
};

}

#endif // UI_MESSAGEBOX
