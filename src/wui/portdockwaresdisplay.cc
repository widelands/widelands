/*
 * Copyright (C) 2011-2024 by the Widelands Development Team
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

#include "wui/portdockwaresdisplay.h"

#include <cassert>

#include "economy/expedition_bootstrap.h"
#include "economy/portdock.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "ui_basic/icon.h"
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
	return as_string(count);
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

static char const* kPicWarePresent = "images/wui/buildings/high_priority_button.png";
static char const* kPicWareComing = "images/wui/buildings/normal_priority_button.png";
static char const* kPicWareMissing = "images/wui/buildings/low_priority_button.png";
static char const* kNoWare = "images/wui/editor/no_ware.png";
static const auto kEmptySlot = std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX);

struct PortDockAdditionalItemsDisplay : UI::Box {
public:
	PortDockAdditionalItemsDisplay(
	   Widelands::Game& g, Panel* parent, bool can_act, PortDock& pd, const uint32_t capacity)
	   : UI::Box(parent,
	             UI::PanelStyle::kWui,
	             "port_dock_additional_items_display",
	             0,
	             0,
	             UI::Box::Horizontal),
	     game_(g),
	     portdock_(pd),
	     capacity_(capacity) {
		assert(capacity_ > 0);
		assert(portdock_.expedition_bootstrap());
		assert(portdock_.expedition_bootstrap()->count_additional_queues() <= capacity_);
		for (uint32_t c = 0; c < capacity_; ++c) {
			UI::Box* box =
			   new UI::Box(this, UI::PanelStyle::kWui, format("box_%u", c), 0, 0, UI::Box::Vertical);

			UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>& d =
			   *new UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>(
			      box, format("additional_%u", c), 0, 0, kWareMenuPicWidth, 8, kWareMenuPicHeight,
			      _("Additional item"), UI::DropdownType::kPictorial, UI::PanelStyle::kWui,
			      UI::ButtonStyle::kWuiSecondary);
			d.add(_("(Empty)"), kEmptySlot, g_image_cache->get(kNoWare), true, _("(Empty)"));
			std::set<std::tuple<std::string, Widelands::WareWorker, Widelands::DescriptionIndex,
			                    const Image*>>
			   sorted;
			for (Widelands::DescriptionIndex i : pd.owner().tribe().wares()) {
				const Widelands::WareDescr& w = *pd.owner().tribe().get_ware_descr(i);
				sorted.insert(std::make_tuple(w.descname(), Widelands::wwWARE, i, w.icon()));
			}
			for (Widelands::DescriptionIndex i : pd.owner().tribe().workers()) {
				if (i != pd.owner().tribe().ferry()) {
					const Widelands::WorkerDescr& w = *pd.owner().tribe().get_worker_descr(i);
					sorted.insert(std::make_tuple(w.descname(), Widelands::wwWORKER, i, w.icon()));
				}
			}
			for (const auto& t : sorted) {
				d.add(std::get<0>(t), std::make_pair(std::get<1>(t), std::get<2>(t)), std::get<3>(t),
				      false, std::get<0>(t));
			}
			d.set_enabled(can_act);
			d.selected.connect([this, c]() { select(c); });

			UI::Icon* icon = new UI::Icon(
			   box, UI::PanelStyle::kWui, format("icon_%u", c), g_image_cache->get(kNoWare));
			icon->set_handle_mouse(true);
			boxes_.push_back(box);
			icons_.push_back(icon);
			dropdowns_.push_back(&d);
			box->add(&d);
			box->add(icon);
			add(box);
		}
		update_selection();
	}

	void think() override {
		UI::Box::think();
		if (portdock_.expedition_bootstrap() == nullptr) {
			return die();
		}
		update_selection();

		for (uint32_t c = 0; c < capacity_; ++c) {
			const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(c);
			assert(!iq || (iq->get_max_size() == 1 && iq->get_max_fill() == 1));
			icons_[c]->set_icon(g_image_cache->get(iq != nullptr ?
                                                   iq->get_filled() != 0u  ? kPicWarePresent :
			                                          iq->get_missing() != 0u ? kPicWareMissing :
                                                                             kPicWareComing :
                                                   kNoWare));
			icons_[c]->set_tooltip(
			   iq != nullptr ? iq->get_filled() != 0u ?
                               /** TRANSLATORS: Tooltip for a ware that is present in the building */
                               _("Present") :
			                      iq->get_missing() != 0u ?
                                  /** TRANSLATORS: Tooltip for a ware that is neither present in the
                                     building nor being transported there */
                                  _("Missing") :
                                  /** TRANSLATORS: Tooltip for a ware that is not present in the
                                     building, but already being transported there */
                                     _("Coming") :
                            "");
		}
	}

	void update_selection() {
		for (uint32_t c = 0; c < capacity_; ++c) {
			if (dropdowns_[c]->is_expanded()) {
				continue;
			}
			const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(c);
			if (iq == nullptr) {
				dropdowns_[c]->select(kEmptySlot);
			} else {
				dropdowns_[c]->select(std::make_pair(iq->get_type(), iq->get_index()));
			}
			assert(dropdowns_[c]->has_selection());
		}
	}

	void select(uint32_t index) {
		assert(index < dropdowns_.size());
		const auto& new_sel = dropdowns_[index]->get_selected();
		const InputQueue* iq = portdock_.expedition_bootstrap()->inputqueue(index);
		if (new_sel.second == Widelands::INVALID_INDEX && (iq == nullptr)) {
			return;
		}
		if ((iq != nullptr) && iq->get_type() == new_sel.first && iq->get_index() == new_sel.second) {
			return;
		}
		if (iq != nullptr) {
			game_.send_player_expedition_config(portdock_, iq->get_type(), iq->get_index(), false);
		}
		if (new_sel.second != Widelands::INVALID_INDEX) {
			game_.send_player_expedition_config(portdock_, new_sel.first, new_sel.second, true);
		}
	}

private:
	Widelands::Game& game_;
	PortDock& portdock_;
	uint32_t capacity_;
	std::vector<UI::Box*> boxes_;
	std::vector<UI::Icon*> icons_;
	std::vector<UI::Dropdown<std::pair<Widelands::WareWorker, Widelands::DescriptionIndex>>*>
	   dropdowns_;
};

static const std::string kStartExpeditionIcon = "images/wui/buildings/start_expedition.png";
static const std::string kCancelExpeditionIcon = "images/wui/buildings/cancel_expedition.png";
static const std::string kStartRefitIcon = "images/wui/ship/ship_refit_warship.png";
static const std::string kCancelRefitIcon = "images/wui/ship/cancel_refit_warship.png";

/// Create a panel that displays the wares and the builder waiting for the expedition to start.
ExpeditionDisplay::ExpeditionDisplay(UI::Panel* parent,
                                     Warehouse* wh,
                                     InteractiveGameBase* igb,
                                     BuildingWindow::CollapsedState* collapsed)
   : UI::Box(parent, UI::PanelStyle::kWui, "expedition_box", 0, 0, UI::Box::Vertical),
     warehouse_(wh),
     igbase_(igb),
     collapsed_(collapsed),
     control_box_(this, UI::PanelStyle::kWui, "expedition_controls", 0, 0, UI::Box::Horizontal),
     expeditionbtn_(&control_box_,
                    "start_or_cancel_expedition",
                    0,
                    0,
                    34,
                    34,
                    UI::ButtonStyle::kWuiMenu,
                    g_image_cache->get(kStartExpeditionIcon)),
     refitbutton_(&control_box_,
                  "start_or_cancel_refit",
                  0,
                  0,
                  34,
                  34,
                  UI::ButtonStyle::kWuiMenu,
                  g_image_cache->get(kStartRefitIcon)) {

	if (warehouse_ == nullptr) {
		return;
	}
	const Widelands::PortDock* pd = warehouse_->get_portdock();
	assert(pd != nullptr);

	// TODO(tothxa):
	//   This shouldn't be necessary (think() should do the updates), but for some reason
	//   then the input queues are not drawn if the expedition is already started when the
	//   port window is opened. This does not make sense. See also comment in update_contents().
	current_type_ =
	   pd->expedition_started() ? pd->expedition_type() : Widelands::ExpeditionType::kNone;

	ensure_box_can_hold_input_queues(*this);

	// Add main controls
	control_box_.add(&expeditionbtn_);
	control_box_.add(&refitbutton_);
	// TODO(tothxa): A target quantity spinbox could also be added to the control_box_
	//            To make it simple, maybe count down (unless infinite) the number left to produce?

	expeditionbtn_.sigclicked.connect(
	   [this]() { act_start_or_cancel(Widelands::ExpeditionType::kExpedition); });
	refitbutton_.sigclicked.connect(
	   [this]() { act_start_or_cancel(Widelands::ExpeditionType::kRefitToWarship); });

	update_buttons();
	update_contents();  // Make sure it gets called at least once
}

void ExpeditionDisplay::think() {
	if (warehouse_ == nullptr) {
		clear();
		return;
	}

	const Widelands::PortDock::ExpeditionState pd_state =
	   warehouse_->get_portdock()->expedition_state();
	if (pd_state == Widelands::PortDock::ExpeditionState::kCancelling) {
		// Transient state, disable buttons.
		expeditionbtn_.set_enabled(false);
		refitbutton_.set_enabled(false);
		clear();  // some more precaution against use after free
		add(&control_box_);
		return;
	}

	const Widelands::ExpeditionType prev_type = current_type_;
	current_type_ = (pd_state == Widelands::PortDock::ExpeditionState::kNone) ?
                      Widelands::ExpeditionType::kNone :  // just to be sure
                      warehouse_->get_portdock()->expedition_type();

	update_buttons();

	if (prev_type == current_type_) {
		// Everything should be fine.
		return;
	}

	update_contents();
}

void ExpeditionDisplay::update_buttons() {
	const bool has_expedition = current_type_ == Widelands::ExpeditionType::kExpedition;
	const bool has_refit = current_type_ == Widelands::ExpeditionType::kRefitToWarship;

	expeditionbtn_.set_pic(
	   g_image_cache->get(has_expedition ? kCancelExpeditionIcon : kStartExpeditionIcon));
	refitbutton_.set_pic(g_image_cache->get(has_refit ? kCancelRefitIcon : kStartRefitIcon));

	expeditionbtn_.set_tooltip(has_expedition ? _("Cancel the expedition") :
                                               _("Start an expedition"));
	refitbutton_.set_tooltip(has_refit ? _("Cancel refitting") :
                                        _("Start refitting a ship to a warship"));

	const bool can_act = igbase_->can_act(warehouse_->get_owner()->player_number());

	expeditionbtn_.set_enabled(can_act && !has_refit);
	refitbutton_.set_enabled(can_act && !has_expedition);
}

void ExpeditionDisplay::update_contents() {
	clear();
	if (warehouse_ == nullptr) {
		return;
	}

	add(&control_box_);

	const Widelands::ExpeditionBootstrap* expedition =
	   warehouse_->get_portdock()->expedition_bootstrap();
	if (expedition == nullptr
	    // TODO(tothxa):
	    //   It should be safer with these, but for some reason then the queues aren't drawn for
	    //   newly started expeditions, even though they are added and the space is reserved for
	    //   them. This just does not make sense.
	    //   If an expedition is already started when the building window is opened, then it's drawn,
	    //   (since I added code to properly initialise the type in the constructor), but if it's
	    //   cancelled then a new one is started, then I get an empty window.
	    // || current_type_ == Widelands::ExpeditionType::kNone ||
	    // !warehouse_->get_portdock()->expedition_started()
	) {
		// Make sure it gets updated next time if it was a transient problem
		current_type_ = Widelands::ExpeditionType::kNone;
		return;
	}

	// Add the input queues.
	int32_t capacity = igbase_->egbase()
	                      .descriptions()
	                      .get_ship_descr(warehouse_->get_owner()->tribe().ship())
	                      ->get_default_capacity();
	for (InputQueue* wq : expedition->queues(false)) {
		InputQueueDisplay* iqd =
		   new InputQueueDisplay(this, *igbase_, *warehouse_, *wq, false, true, collapsed_);
		add(iqd, UI::Box::Resizing::kFullSize);
		capacity -= wq->get_max_size();
	}

	// TODO(tothxa): This can happen due to bad lua definitions. Shouldn't this be a proper check
	//               that throws an exception on fail?
	assert(capacity >= 0);

	if (capacity > 0 && current_type_ == Widelands::ExpeditionType::kExpedition &&
	    warehouse_->owner().additional_expedition_items_allowed()) {
		add(new PortDockAdditionalItemsDisplay(
		       igbase_->game(), this, igbase_->can_act(warehouse_->get_owner()->player_number()),
		       *warehouse_->get_portdock(), capacity),
		    UI::Box::Resizing::kAlign, UI::Align::kCenter);
	}

	// TODO(tothxa): Add soldier control if kRefitToWarship
}

void ExpeditionDisplay::act_start_or_cancel(const Widelands::ExpeditionType t) {
	if (!igbase_->can_act(warehouse_->get_owner()->player_number())) {
		return;
	}

	switch (warehouse_->get_portdock()->expedition_state()) {
	case Widelands::PortDock::ExpeditionState::kCancelling:
		return;
	case Widelands::PortDock::ExpeditionState::kNone:
		igbase_->game().send_player_start_or_cancel_expedition(*warehouse_, t);
		break;
	case Widelands::PortDock::ExpeditionState::kStarted:
	case Widelands::PortDock::ExpeditionState::kReady:
		// To avoid races, we only cancel if the matching button was pressed, otherwise
		// we ignore the button event.
		if (t == warehouse_->get_portdock()->expedition_type()) {
			// Prevent use after free: ExpeditionBootstrap starts deleting input queues while
			// draw() may still try to use them.
			clear();
			add(&control_box_);
			igbase_->game().send_player_start_or_cancel_expedition(
			   *warehouse_, Widelands::ExpeditionType::kNone);
		}
	}
}
