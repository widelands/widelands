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

#ifndef RT_ERRORS_IMPL_H
#define RT_ERRORS_IMPL_H

#include "graphic/text/rt_errors.h"

namespace RT {

struct SyntaxError_Impl : public SyntaxError {
	SyntaxError_Impl(size_t line, size_t col, std::string expected, std::string got, std::string next_chars)
		: SyntaxError
		  ((boost::format("Syntax error at %1%:%2%: expected %3%, got '%4%'. String continues with: '%5%'")
					% line % col % expected % got % next_chars)
			.str())
	{}
};
}

#endif /* end of include guard: RT_ERRORS_IMPL_H */



