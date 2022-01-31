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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_TRADE_AGREEMENT_H
#define WL_LOGIC_TRADE_AGREEMENT_H

#include "logic/map_objects/tribes/bill_of_materials.h"

namespace Widelands {

// TODO(sirver,trading): Document everything in here.

// Maximum number of a single ware that can be contained in a trade batch.
constexpr int kMaxPerItemTradeBatchSize = 15;

struct Trade {
	BillOfMaterials items_to_send;
	BillOfMaterials items_to_receive;
	int num_batches;
	Serial initiator;
	Serial receiver;
};

// TODO(sirver,trading): This class should probably be private to 'Game'.
struct TradeAgreement {
	enum class State {
		kProposed,
		kRunning,
	};

	State state;
	Trade trade;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRADE_AGREEMENT_H
