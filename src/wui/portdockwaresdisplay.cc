/*
 * Copyright (C) 2011-2017 by the Widelands Development Team
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

#include "wui/portdockwaresdisplay.h"

#include <boost/lexical_cast.hpp>

#include "economy/expedition_bootstrap.h"
#include "economy/portdock.h"
#include "logic/player.h"
#include "wui/inputqueuedisplay.h"

using Widelands::PortDock;
using Widelands::Warehouse;
using Widelands::InputQueue;

namespace {

/**
 * Display wares or workers that are waiting to be shipped from a port.
 */
struct PortDockWaresDisplay : AbstractWaresDisplay {
	PortDockWaresDisplay(Panel* parent, uint32_t width, PortDock& pd, Widelands::WareWorker type);

	std::string info_for_ware(Widelands::DescriptionIndex ware) override;

private:
	PortDock& portdock_;
};

PortDockWaresDisplay::PortDockWaresDisplay(Panel* parent,
                                           uint32_t width,
                                           PortDock& pd,
                                           Widelands::WareWorker type)
   : AbstractWaresDisplay(parent, 0, 0, pd.owner().tribe(), type, false), portdock_(pd) {
	set_inner_size(width, 0);
}

std::string PortDockWaresDisplay::info_for_ware(Widelands::DescriptionIndex ware) {
	uint32_t count = portdock_.count_waiting(get_type(), ware);
	return boost::lexical_cast<std::string>(count);
}

}  // anonymous namespace

/**
 * Create a panel that displays the wares or workers that are waiting to be shipped from a port.
 */
AbstractWaresDisplay* create_portdock_wares_display(UI::Panel* parent,
                                                    uint32_t width,
                                                    PortDock& pd,
                                                    Widelands::WareWorker type) {
	return new PortDockWaresDisplay(parent, width, pd, type);
}

/// Create a panel that displays the wares and the builder waiting for the expedition to start.
UI::Box*
create_portdock_expedition_display(UI::Panel* parent, Warehouse& wh, InteractiveGameBase& igb) {
	UI::Box& box = *new UI::Box(parent, 0, 0, UI::Box::Vertical);

	// Add the input queues.
	for (InputQueue* wq : wh.get_portdock()->expedition_bootstrap()->queues()) {
		box.add(new InputQueueDisplay(&box, 0, 0, igb, wh, wq, true));
	}

	return &box;
}
