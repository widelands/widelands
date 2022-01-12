/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#include "economy/ware_priority.h"

#include "io/streamread.h"
#include "io/streamwrite.h"

namespace Widelands {

// All these values are used as weighting factors by Request code.
const WarePriority WarePriority::kLow(1);
const WarePriority WarePriority::kNormal(64);
const WarePriority WarePriority::kHigh(4096);
// These two constants additionally get special treatment.
const WarePriority WarePriority::kVeryLow(0);
const WarePriority WarePriority::kVeryHigh(std::numeric_limits<uint32_t>::max());

WarePriority::WarePriority(StreamRead& fr) : value_(fr.unsigned_32()) {
}

void WarePriority::write(StreamWrite& fw) const {
	fw.unsigned_32(value_);
}

}  // namespace Widelands
