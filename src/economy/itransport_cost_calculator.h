/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_ECONOMY_ITRANSPORT_COST_CALCULATOR_H
#define WL_ECONOMY_ITRANSPORT_COST_CALCULATOR_H

#include "base/macros.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

/**
 * This class provides the interface to get cost and cost estimations
 * for certain transport properties (node->node).
 *
 * At the time of this writing, Map implements all of this functionality
 * but most economy code doesn't need all of maps functionality
 */
class ITransportCostCalculator {
public:
	ITransportCostCalculator() = default;
	virtual ~ITransportCostCalculator() {
	}

	virtual int32_t calc_cost_estimate(const Coords&, const Coords&) const = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(ITransportCostCalculator);
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ITRANSPORT_COST_CALCULATOR_H
