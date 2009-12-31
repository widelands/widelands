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

#ifndef LOGIN_BOX_H
#define LOGIN_BOX_H

#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

namespace UI {

struct LoginBox : public Window {
	LoginBox(Panel *);

	std::string get_nickname() {return eb_nickname->text();}
	std::string get_password() {return eb_password->text();}
	std::string get_email()    {return eb_emailadd->text();}
	bool new_registration()    {return cb_register->get_state();}

private:
	void pressedLogin();
	void pressedCancel();

private:
	EditBox  * eb_nickname;
	EditBox  * eb_emailadd;
	EditBox  * eb_password;
	Checkbox * cb_register;
	Textarea * ta_nickname;
	Textarea * ta_emailadd;
	Textarea * ta_password;
	Textarea * ta_register;
	Textarea * pwd_warning;
};

}

#endif
