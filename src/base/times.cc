/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "base/times.h"

#include "io/fileread.h"
#include "io/filewrite.h"

Duration::Duration(FileRead& fr) : value_(fr.unsigned_32()) {
}
void Duration::save(FileWrite& fw) const {
	fw.unsigned_32(value_);
}

Time::Time(FileRead& fr) : value_(fr.unsigned_32()) {
}
void Time::save(FileWrite& fw) const {
	fw.unsigned_32(value_);
}
