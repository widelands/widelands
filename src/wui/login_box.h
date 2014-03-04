/*
 * Copyright (C) 2002-2004, 2006, 2008, 2010 by the Widelands Development Team
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

#ifndef LOGIN_BOX_H
#define LOGIN_BOX_H

#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

struct LoginBox : public UI::Window {
	LoginBox(UI::Panel &);

	std::string get_nickname() {return eb_nickname->text();}
	std::string get_password() {return eb_password->text();}
	bool registered()          {return cb_register->get_state();}
	bool set_automaticlog()    {return cb_auto_log->get_state();}

private:
	void pressedLogin();
	void pressedCancel();

private:
	UI::EditBox  * eb_nickname;
	UI::EditBox  * eb_password;
	UI::Checkbox * cb_register;
	UI::Checkbox * cb_auto_log;
	UI::Textarea * ta_nickname;
	UI::Textarea * ta_password;
	UI::Textarea * ta_register;
	UI::Textarea * ta_auto_log;
	UI::Multiline_Textarea * pwd_warning;
};

#endif
