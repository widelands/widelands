/*
 * Copyright (C) 2011-2019 by the Widelands Development Team
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
#include "graphic/text_layout.h"
#include "logic/player.h"
#include "wui/inputqueuedisplay.h"

using Widelands::InputQueue;
using Widelands::PortDock;
using Widelands::Warehouse;

namespace {

/**
 * Display wares or workers that are waiting to be shipped from a port.
 */
struct PortDockWaresDisplay : AbstractWaresDisplay {
	PortDockWaresDisplay(Panel* parent,
	                     uint32_t width,
	                     const PortDock& pd,
	                     Widelands::WareWorker type);

	std::string info_for_ware(Widelands::DescriptionIndex ware) override;

private:
	const PortDock& portdock_;
};

PortDockWaresDisplay::PortDockWaresDisplay(Panel* parent,
                                           uint32_t width,
                                           const PortDock& pd,
                                           Widelands::WareWorker type)
   : AbstractWaresDisplay(parent, 0, 0, pd.owner().tribe(), type, false), portdock_(pd) {
	set_inner_size(width, 0);
}

std::string PortDockWaresDisplay::info_for_ware(Widelands::DescriptionIndex ware) {
	const uint32_t count = portdock_.count_waiting(get_type(), ware);
	return boost::lexical_cast<std::string>(count);
}

}  // anonymous namespace

/**
 * Create a panel that displays the wares or workers that are waiting to be shipped from a port.
 */
AbstractWaresDisplay* create_portdock_wares_display(UI::Panel* parent,
                                                    uint32_t width,
                                                    const PortDock& pd,
                                                    Widelands::WareWorker type) {
	return new PortDockWaresDisplay(parent, width, pd, type);
}

constexpr uint32_t kItemSize = 28;
constexpr uint32_t kAdditionalItemsBorder = 4;

struct PortDockAdditionalItemsDisplay : UI::Box {
	PortDockAdditionalItemsDisplay(
			Game& g,
			Panel* parent,
			bool can_act,
			PortDock& pd,
			const uint32_t capacity)
		: UI::Box(parent, 0, 0, UI::Box::Horizontal),
		  game_(g), portdock_(pd) {
		assert(capacity > 0);
		assert(portdock_.expedition_bootstrap());
		assert(portdock_.expedition_bootstrap()->count_additional_queues() == capacity);
		for (uint32_t c = 0; c < capacity; ++c) {
			const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(c);
			UI::Dropdown& d = *new UI::Dropdown(this, (boost::format("additional_%u") % c).str(), 0, 0,
					kWareMenuPicWidth, 8, kWareMenuPicHeight, "", UI::DropdownType::kPictorial,
					UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary);
			d.add(_("(Empty)"), std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX), nullptr, !iq, _("(Empty)"));
			for (Widelands::DescriptionIndex i : pd.owner().tribe().wares()) {
				const Widelands::WareDescription& w = pd.owner().tribe().get_ware_description(i);
				d.add(i.descname(), std::make_pair(Widelands::wwWARE, i), i.icon(),
						iq && iq->get_type() == Widelands::wwWARE && iq->get_index() == i, i.descname());
			}
			for (Widelands::DescriptionIndex i : pd.owner().tribe().workers()) {
				const Widelands::WorkerDescription& w = pd.owner().tribe().get_worker_description(i);
				d.add(i.descname(), std::make_pair(Widelands::wwWORKER, i), i.icon(), false, i.descname());
			}
			d.set_enabled(can_act);
			d.selected.connect(boost::bind(&PortDockAdditionalItemsDisplay::select, this, c));
			assert(d.has_selection());
			dropdowns_.push_back(&d);
			add(&d);
		}
	}

	void select(uint32_t index) {
		assert(index < dropdowns_.size());
		const auto& new_sel = dropdowns_[index]->get_selected();
		const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(index);
		if (new_sel.second == Widelands::INVALID_INDEX && !iq) {
			return;
		}
		if (iq && iq->get_type() == new_sel.first && iq->get_index() == new_sel.second) {
			return;
		}
		if (iq) {
			game_.send_player_expedition_config(portdock_, iq->get_type(), iq->get_index(), false);
		}
		if (new_sel.second != Widelands::INVALID_INDEX) {
			game_.send_player_expedition_config(portdock_, new_sel.first, new_sel.second, true);
		}
	}

private:
	Game& game_;
	PortDock& portdock_;
	std::vector<UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>*> dropdowns_;
};

/// Create a panel that displays the wares and the builder waiting for the expedition to start.
UI::Box*
create_portdock_expedition_display(UI::Panel* parent, Warehouse& wh, InteractiveGameBase& igb) {
	UI::Box& box = *new UI::Box(parent, 0, 0, UI::Box::Vertical);

	// Add the input queues.
	int32_t capacity = igb.egbase().tribes().get_ship_descr(wh.get_owner()->tribe().ship())->get_default_capacity();
	for (const InputQueue* wq : wh.get_portdock()->expedition_bootstrap()->queues(false)) {
		InputQueueDisplay* iqd = new InputQueueDisplay(&box, 0, 0, igb, wh, *wq, true);
		box.add(iqd);
		capacity -= wq->get_max_size();
	}
	assert(capacity >= 0);

	if (capacity > 0) {
		const bool can_act = igb.can_act(wh.get_owner()->player_number());
		box.add(new PortDockAdditionalItemsDisplay(igb.game(), &box, 0, 0, parent->get_w(),
				can_act, *wh.get_portdock(), capacity), UI::Box::Resizing::kAlign, UI::Align::kCenter);
	}

	return &box;
}
