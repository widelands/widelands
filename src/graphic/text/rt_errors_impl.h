/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_RT_ERRORS_IMPL_H
#define WL_GRAPHIC_TEXT_RT_ERRORS_IMPL_H

#include "base/string.h"
#include "graphic/text/rt_errors.h"

namespace RT {

struct SyntaxErrorImpl : public SyntaxError {
	SyntaxErrorImpl(size_t line,
	                size_t col,
	                const std::string& expected,
	                const std::string& got,
	                const std::string& next_chars)
	   : SyntaxError(
	        format("Syntax error at %1%:%2%: expected %3%, got '%4%'. String continues with: '%5%'",
	               line,
	               col,
	               expected,
	               got,
	               next_chars)) {
	}
};
}  // namespace RT

#endif  // end of include guard: WL_GRAPHIC_TEXT_RT_ERRORS_IMPL_H
