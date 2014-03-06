/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef RT_ERRORS_H
#define RT_ERRORS_H

#include <exception>

namespace RT {

class Exception : public std::exception {
public:
	Exception(std::string msg) : std::exception(), m_msg(msg) {
	}
	virtual ~Exception() throw () {}
	virtual const char* what() const throw () override {return m_msg.c_str();}

private:
	std::string m_msg;
};

#define DEF_ERR(name) class name : public Exception { \
public: \
		  name(std::string msg) : Exception(msg) {} \
};

DEF_ERR(AttributeNotFound);
DEF_ERR(BadFont);
DEF_ERR(BadImage);
DEF_ERR(EOT);
DEF_ERR(InvalidColor);
DEF_ERR(RenderError);
DEF_ERR(SyntaxError);
DEF_ERR(WidthTooSmall);

#undef DEF_ERR


};

#endif /* end of include guard: RT_ERRORS_H */
