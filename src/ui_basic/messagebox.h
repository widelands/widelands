/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_MESSAGEBOX_H
#define WL_UI_BASIC_MESSAGEBOX_H

#include <memory>

#include <boost/signals2.hpp>

#include "graphic/align.h"
#include "ui_basic/window.h"

namespace UI {

struct WLMessageBoxImpl;

/**
 * Shows a standard messagebox. The message box can be used either as a modal
 * or as a non-modal dialog box.
 *
 * Using it as a modal dialog box is very straightforward:
 *     WLMessageBox mb(parent, "Caption", "Text", OK);
 *     UI::Panel::Returncodes code = mb.run<UI::Panel::Returncodes>();
 * The return code is ok_code if the "OK" button has been pressed in a \ref YESNO
 * dialog box. Otherwise, it is dying_code (or negative, if the modal messagebox has
 * been interrupted in an unusual way).
 *
 * Using it as a non-modal dialog box is slightly more complicated. You have
 * to add this dialog box as a child to the current fullscreen panel, and
 * connect the signals \ref ok and \ref no or \ref ok, depending on the
 * messagebox type, to a function that deletes the message box.
 * \note this function is named "WLMessageBox" instead of simply "MessageBox"
 *       because else linking on Windows (even with #undef MessageBox) will
 *       not work.
*/
struct WLMessageBox : public Window {
	enum class MBoxType {
		kOk,
		kOkCancel
	};
	WLMessageBox
		(Panel * parent,
		 const std::string & caption,
		 const std::string & text,
		 MBoxType,
		 Align = Align_Center);
	~WLMessageBox();

	boost::signals2::signal<void ()> ok;
	boost::signals2::signal<void ()> cancel;

	bool handle_mousepress  (uint8_t btn, int32_t mx, int32_t my) override;
	bool handle_mouserelease(uint8_t btn, int32_t mx, int32_t my) override;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	virtual void clicked_ok();
	virtual void clicked_back();

private:
	std::unique_ptr<WLMessageBoxImpl> d;
};

}

#endif  // end of include guard: WL_UI_BASIC_MESSAGEBOX_H
