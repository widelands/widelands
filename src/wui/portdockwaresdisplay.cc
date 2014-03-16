/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "economy/portdock.h"
#include "logic/expedition_bootstrap.h"
#include "logic/player.h"
#include "wui/waresdisplay.h"
#include "wui/waresqueuedisplay.h"

using Widelands::PortDock;
using Widelands::Warehouse;
using Widelands::WaresQueue;

namespace {

/**
 * Display wares or workers that are waiting to be shipped from a port.
 */
struct PortDockWaresDisplay : AbstractWaresDisplay {
	PortDockWaresDisplay(Panel * parent, uint32_t width, PortDock & pd, Widelands::WareWorker type);

	virtual std::string info_for_ware(Widelands::Ware_Index ware) override;

private:
	PortDock & m_portdock;
};

PortDockWaresDisplay::PortDockWaresDisplay
	(Panel * parent, uint32_t width, PortDock & pd, Widelands::WareWorker type) :
	AbstractWaresDisplay(parent, 0, 0, pd.owner().tribe(), type, false),
	m_portdock(pd)
{
	set_inner_size(width, 0);
}

std::string PortDockWaresDisplay::info_for_ware(Widelands::Ware_Index ware)
{
	uint32_t count = m_portdock.count_waiting(get_type(), ware);
	return boost::lexical_cast<std::string>(count);
}

} // anonymous namespace

/**
 * Create a panel that displays the wares or workers that are waiting to be shipped from a port.
 */
AbstractWaresDisplay * create_portdock_wares_display
	(UI::Panel * parent, uint32_t width, PortDock & pd, Widelands::WareWorker type)
{
	return new PortDockWaresDisplay(parent, width, pd, type);
}

/// Create a panel that displays the wares and the builder waiting for the expedition to start.
UI::Box * create_portdock_expedition_display(UI::Panel * parent, Warehouse & wh, Interactive_GameBase & igb)
{
	UI::Box & box = *new UI::Box(parent, 0, 0, UI::Box::Vertical);

	// Add the wares queues.
	BOOST_FOREACH(WaresQueue* wq, wh.get_portdock()->expedition_bootstrap()->wares()) {
		box.add(new WaresQueueDisplay(&box, 0, 0, igb, wh, wq, true), UI::Box::AlignLeft);
	}

/* FIXME Implement UI for Builder + Soldiers
	UI::Box & workers = *new UI::Box(&box, 0, 0, UI::Box::Horizontal);
	box.add(&workers, UI::Box::AlignLeft);

	//for (uint32_t i = 0; i < wh.get_expedition_workers().size(); ++i)
		//workers.add(icon of worker, UI::Box::AlignLeft);
*/

	return &box;
}
