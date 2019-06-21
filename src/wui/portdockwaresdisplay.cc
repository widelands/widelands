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

struct PortDockAdditionalItemsDisplay : UI::Panel {
	PortDockAdditionalItemsDisplay(Panel* parent,
			int32_t x,
			int32_t y,
			uint32_t max_w,
			bool can_act,
			PortDock& pd,
			const uint32_t capacity)
		: UI::Panel(parent, x, y, 0, 0),
		  portdock_(pd),
		  capacity_(capacity),
		  items_per_row_(max_w / kItemSize),
		  nr_rows_(capacity_ / items_per_row_),
		  can_act_(can_act) {
		assert(capacity_ > 0);
		assert(items_per_row_ > 0);
		assert(nr_rows_ > 0);
		queues_.resize(capacity_);
		if (nr_rows_ * items_per_row_ < capacity_) {
			++nr_rows_;
			assert(nr_rows_ * items_per_row_ >= capacity_);
		}
		assert(portdock_.expedition_bootstrap());
		for (uint32_t i = 0; i < capacity_; ++i) {
			queues_[i] = portdock_.expedition_bootstrap()->inputqueue(i);
		}
		set_desired_size(kItemSize * items_per_row_ + 2 * kAdditionalItemsBorder,
				kItemSize * nr_rows_ + 2 * kAdditionalItemsBorder);
	}

	void draw(RenderTarget& dst) override {
		Panel::draw(dst);
		Vector2i grid_coords(0, 0);
		for (uint32_t i = 0; i < capacity_; ++i) {
			dst.brighten_rect(Recti(grid_coords.x * kItemSize + kAdditionalItemsBorder,
					grid_coords.y * kItemSize + kAdditionalItemsBorder, kItemSize, kItemSize),
					(grid_coords.x + grid_coords.y) % 2 == 0 ? 64 : 32);
			InputQueue* q = portdock_.expedition_bootstrap()->inputqueue(i);
			queues_[i] = q;
			if (q) {
				const Widelands::MapObjectDescr* descr;
				if (q->get_type() == Widelands::wwWARE) {
					descr = portdock_.get_owner()->tribe().get_ware_descr(q->get_index());
				} else {
					descr = portdock_.get_owner()->tribe().get_worker_descr(q->get_index());
				}
				Vector2i point(grid_coords.x * kItemSize +
						(kItemSize - descr->icon()->width()) / 2 + kAdditionalItemsBorder,
						grid_coords.y * kItemSize + (kItemSize - descr->icon()->height()) / 2 + kAdditionalItemsBorder);
				if (q->get_filled()) {
					dst.blit(point, descr->icon());
				} else {
					dst.blit_monochrome(point, descr->icon(), q->get_missing() ?
							RGBAColor(191, 191, 191, 127) : RGBAColor(127, 127, 127, 191));
				}
			}
			++grid_coords.x;
			if (grid_coords.x >= static_cast<int32_t>(items_per_row_)) {
				grid_coords.x = 0;
				++grid_coords.y;
			}
		}
	}

	void set_default_tooltip() {
		set_tooltip((boost::format("<p>%s%s%s%s</p>") %
				g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(
				_("Click on an item to remove it from the list.")) %
		    as_listitem(
		       _("Hold down Ctrl to remove all items"), UI::FontStyle::kTooltip) %
		    as_listitem(
		       _("Hold down Shift to remove all items up from the one you’re pointing at"), UI::FontStyle::kTooltip) %
		    as_listitem(
		    	_("Hold down Ctrl and Shift to remove all items of the same type as the one you’re pointing at"),
		    		UI::FontStyle::kTooltip))
			.str());
	}

	int32_t item_at(int32_t x, int32_t y) const {
		if (x < 0 || y < 0) {
			return -1;
		}
		uint32_t col = (x - kAdditionalItemsBorder) / kItemSize;
		uint32_t row = (y - kAdditionalItemsBorder) / kItemSize;
		if (col >= items_per_row_ || row >= nr_rows_) {
			return -1;
		}
		uint32_t index = row * items_per_row_ + col;
		return index < capacity_ ? index : -1;
	}

	void handle_mousein(bool) override {
		set_default_tooltip();
	}

	bool handle_mousemove(uint8_t, int32_t x, int32_t y, int32_t, int32_t) override {
		const int32_t index = item_at(x, y);
		if (index >= 0) {
			if (InputQueue* q = queues_[index]) {
				const Widelands::MapObjectDescr* descr;
				if (q->get_type() == Widelands::wwWARE) {
					descr = portdock_.get_owner()->tribe().get_ware_descr(q->get_index());
				} else {
					descr = portdock_.get_owner()->tribe().get_worker_descr(q->get_index());
				}
				set_tooltip(descr->descname());
				return true;
			}
		}
		set_default_tooltip();
		return true;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
		if (btn != SDL_BUTTON_LEFT || !can_act_) {
			return UI::Panel::handle_mousepress(btn, x, y);
		}
		const int32_t index = item_at(x, y);
		if (index >= 0) {
			if (InputQueue* q = queues_[index]) {
				Widelands::Game& game = dynamic_cast<Widelands::Game&>(portdock_.get_owner()->egbase());
				if ((SDL_GetModState() & KMOD_CTRL)) {
					if ((SDL_GetModState() & KMOD_SHIFT)) {
						for (uint32_t i = 0; i < capacity_; ++i) {
							if (InputQueue* queue = queues_[i]) {
								Widelands::WareWorker ww = q->get_type();
								Widelands::DescriptionIndex di = q->get_index();
								if (queue->get_type() == ww && queue->get_index() == di) {
									game.send_player_expedition_config(portdock_, ww, di, false);
								}
							}
						}
					} else {
						for (uint32_t i = 0; i < capacity_; ++i) {
							if (InputQueue* queue = queues_[i]) {
								game.send_player_expedition_config(
										portdock_, queue->get_type(), queue->get_index(), false);
							}
						}
					}
				} else {
					if ((SDL_GetModState() & KMOD_SHIFT)) {
						for (uint32_t i = index; i < capacity_; ++i) {
							if (InputQueue* queue = queues_[i]) {
								game.send_player_expedition_config(
										portdock_, queue->get_type(), queue->get_index(), false);
							}
						}
					} else {
						game.send_player_expedition_config(portdock_, q->get_type(), q->get_index(), false);
					}
				}
				return true;
			}
		}
		return UI::Panel::handle_mousepress(btn, x, y);
	}

private:
	PortDock& portdock_;
	uint32_t capacity_;
	uint32_t items_per_row_;
	uint32_t nr_rows_;
	bool can_act_;
	std::vector<InputQueue*> queues_;
};

struct PortDockAdditionalItemsChooser : UI::Panel {
	PortDockAdditionalItemsChooser(Panel* parent,
			int32_t x,
			int32_t y,
			uint32_t max_w,
			PortDock& pd)
		: UI::Panel(parent, x, y, 0, 0),
		  portdock_(pd),
		  nr_items_(portdock_.get_owner()->tribe().get_nrwares() + portdock_.get_owner()->tribe().get_nrworkers()),
		  items_per_row_(max_w / kItemSize),
		  nr_rows_(nr_items_ / items_per_row_) {
		assert(portdock_.expedition_bootstrap());
		assert(items_per_row_ > 0);
		assert(nr_rows_ > 0);
		if (nr_rows_ * items_per_row_ < nr_items_) {
			++nr_rows_;
			assert(nr_rows_ * items_per_row_ >= nr_items_);
		}
		set_desired_size(kItemSize * items_per_row_ + 2 * kAdditionalItemsBorder,
				kItemSize * nr_rows_ + 2 * kAdditionalItemsBorder);
	}

	void draw(RenderTarget& r) override {
		Panel::draw(r);
		Vector2i gc(0, 0);
		auto blit = [this](const Widelands::MapObjectDescr& descr, Vector2i& grid_coords, RenderTarget& dst) {
			dst.brighten_rect(Recti(grid_coords.x * kItemSize + kAdditionalItemsBorder,
					grid_coords.y * kItemSize + kAdditionalItemsBorder, kItemSize, kItemSize),
					(grid_coords.x + grid_coords.y) % 2 == 0 ? 64 : 32);
			Vector2i point(grid_coords.x * kItemSize + (kItemSize - descr.icon()->width()) / 2 + kAdditionalItemsBorder,
					grid_coords.y * kItemSize + (kItemSize - descr.icon()->height()) / 2 + kAdditionalItemsBorder);
			dst.blit(point, descr.icon());
			++grid_coords.x;
			if (grid_coords.x >= static_cast<int32_t>(items_per_row_)) {
				grid_coords.x = 0;
				++grid_coords.y;
			}
		};
		const Widelands::TribeDescr& tribe = portdock_.get_owner()->tribe();
		for (Widelands::DescriptionIndex di : tribe.wares()) {
			blit(*tribe.get_ware_descr(di), gc, r);
		}
		for (Widelands::DescriptionIndex di : tribe.workers()) {
			blit(*tribe.get_worker_descr(di), gc, r);
		}
	}

	void set_default_tooltip() {
		set_tooltip((boost::format("<p>%s%s</p>") %
				g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(
				_("Click on an item to add it to the list.")) %
		    as_listitem(
		    	_("Hold down Ctrl to add as many instances of this item as possible"),
		    		UI::FontStyle::kTooltip))
			.str());
	}

	std::pair<Widelands::WareWorker, Widelands::DescriptionIndex> item_at(int32_t x, int32_t y) const {
		if (x < 0 || y < 0) {
			return std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX);
		}
		uint32_t col = (x - kAdditionalItemsBorder) / kItemSize;
		uint32_t row = (y - kAdditionalItemsBorder) / kItemSize;
		if (col >= items_per_row_ || row >= nr_rows_) {
			return std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX);
		}
		uint32_t index = row * items_per_row_ + col;
		const Widelands::TribeDescr& tribe = portdock_.get_owner()->tribe();
		for (Widelands::DescriptionIndex di : tribe.wares()) {
			if (index == 0) {
				return std::make_pair(Widelands::wwWARE, di);
			}
			--index;
		}
		for (Widelands::DescriptionIndex di : tribe.workers()) {
			if (index == 0) {
				return std::make_pair(Widelands::wwWORKER, di);
			}
			--index;
		}
		return std::make_pair(Widelands::wwWARE, Widelands::INVALID_INDEX);
	}

	void handle_mousein(bool) override {
		set_default_tooltip();
	}

	bool handle_mousemove(uint8_t, int32_t x, int32_t y, int32_t, int32_t) override {
		const std::pair<Widelands::WareWorker, Widelands::DescriptionIndex> pair = item_at(x, y);
		if (pair.second != Widelands::INVALID_INDEX) {
			const Widelands::MapObjectDescr* descr;
			if (pair.first == Widelands::wwWARE) {
				descr = portdock_.get_owner()->tribe().get_ware_descr(pair.second);
			} else {
				descr = portdock_.get_owner()->tribe().get_worker_descr(pair.second);
			}
			set_tooltip(descr->descname());
			return true;
		}
		set_default_tooltip();
		return true;
	}

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
		if (btn != SDL_BUTTON_LEFT) {
			return UI::Panel::handle_mousepress(btn, x, y);
		}
		const std::pair<Widelands::WareWorker, Widelands::DescriptionIndex> pair = item_at(x, y);
		if (pair.second != Widelands::INVALID_INDEX) {
			Widelands::Game& game = dynamic_cast<Widelands::Game&>(portdock_.get_owner()->egbase());
			int32_t capacity = portdock_.get_owner()->egbase().tribes().get_ship_descr(
					portdock_.get_owner()->tribe().ship())->get_default_capacity();
			for (InputQueue* q : portdock_.expedition_bootstrap()->queues(true)) {
				capacity -= q->get_max_size();
			}
			assert(capacity >= 0);
			if ((SDL_GetModState() & KMOD_CTRL)) {
				for (; capacity > 0; --capacity) {
					game.send_player_expedition_config(portdock_, pair.first, pair.second, true);
				}
			} else {
				if (capacity > 0) {
					game.send_player_expedition_config(portdock_, pair.first, pair.second, true);
				}
			}
			return true;
		}
		return UI::Panel::handle_mousepress(btn, x, y);
	}

private:
	PortDock& portdock_;
	uint32_t nr_items_;
	uint32_t items_per_row_;
	uint32_t nr_rows_;
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
		box.add(new PortDockAdditionalItemsDisplay(&box, 0, 0, parent->get_w(),
				can_act, *wh.get_portdock(), capacity));
		if (can_act) {
			box.add(new PortDockAdditionalItemsChooser(&box, 0, 0, parent->get_w(), *wh.get_portdock()));
		}
	}

	return &box;
}
