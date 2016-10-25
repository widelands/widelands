/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/workerpanel.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "economy/workers_queue.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/player.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "wlapplication.h"
#include "wui/interactive_gamebase.h"
#include "wui/workercapacitycontrol.h"


using Widelands::Worker;
using Widelands::WorkersQueue;
using Widelands::DescriptionIndex;
using Widelands::Building;

namespace {

constexpr uint32_t kMaxColumns = 6;
constexpr uint32_t kAnimateSpeed = 300; ///< in pixels per second
constexpr uint32_t kIconBorder = 2;

} // namespace

/**
 * Iconic representation of workers in a queue.
 * Adapted copy of \ref SoldierPanel
 */
struct WorkerPanel : UI::Panel {
	using WorkerFn = boost::function<void (const Worker *)>;

	WorkerPanel(UI::Panel & parent, Widelands::EditorGameBase & egbase, WorkersQueue & workers_queue);

	Widelands::EditorGameBase & egbase() const {return egbase_;}

	void think() override;
	void draw(RenderTarget &) override;

	void set_mouseover(const WorkerFn & fn);
	void set_click(const WorkerFn & fn);

protected:
	void handle_mousein(bool inside) override;
	bool handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

private:
	Vector2i calc_pos(uint32_t row, uint32_t col) const;
	const Worker * find_worker(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Worker> worker;
		uint32_t row;
		uint32_t col;
		Vector2i pos;

		/**
		 * Experience when last drawing
		 */
		uint32_t cache_experience;
        // The experience is currently not shown on top of the icon,
        // this remains as further work.
	};

	Widelands::EditorGameBase & egbase_;
	WorkersQueue& workers_;

	WorkerFn mouseover_fn_;
	WorkerFn click_fn_;

	std::vector<Icon> icons_;

	uint32_t rows_;
	uint32_t cols_;

	uint32_t icon_width_;
	uint32_t icon_height_;

	int32_t last_animate_time_;
};

WorkerPanel::WorkerPanel
	(UI::Panel & parent,
	 Widelands::EditorGameBase & gegbase,
	 WorkersQueue & workers_queue)
:
Panel(&parent, 0, 0, 0, 0),
egbase_(gegbase),
workers_(workers_queue),
last_animate_time_(0)
{
	{
	    // Fetch the icon of some worker and retrieve its size
	    assert(egbase_.tribes().nrtribes() > 0);
	    assert(egbase_.tribes().nrworkers() > 0);
	    // ID=0 should be the first worker created and anyone is good enough
        const Image * some_icon = egbase_.tribes().get_worker_descr(0)->icon();
        icon_width_ = some_icon->width();
        icon_height_ = some_icon->height();
	}
	icon_width_ += 2 * kIconBorder;
	icon_height_ += 2 * kIconBorder;

	Widelands::Quantity maxcapacity = workers_.max_capacity();
	if (maxcapacity <= kMaxColumns) {
		cols_ = maxcapacity;
		rows_ = 1;
	} else {
		cols_ = kMaxColumns;
		rows_ = (maxcapacity + cols_ - 1) / cols_;
	}

	set_size(cols_ * icon_width_, rows_ * icon_height_);
	set_desired_size(cols_ * icon_width_, rows_ * icon_height_);
	set_thinks(true);

	// Initialize the icons
	uint32_t row = 0;
	uint32_t col = 0;
	for (Worker * worker : workers_.workers()) {
		Icon icon;
		icon.worker = worker;
		icon.row = row;
		icon.col = col;
		icon.pos = calc_pos(row, col);
		icons_.push_back(icon);

		if (++col >= cols_) {
			col = 0;
			row++;
		}
	}
}

/**
 * Set the callback function that indicates which worker the mouse is over.
 */
void WorkerPanel::set_mouseover(const WorkerPanel::WorkerFn & fn)
{
	mouseover_fn_ = fn;
}

/**
 * Set the callback function that is called when a worker is clicked.
 */
void WorkerPanel::set_click(const WorkerPanel::WorkerFn & fn)
{
	click_fn_ = fn;
}

void WorkerPanel::think()
{
	bool changes = false;
	uint32_t capacity = workers_.capacity();

	// Update worker list and target row/col:
	std::vector<Worker *> workerlist = workers_.workers();
	std::vector<uint32_t> row_occupancy;
	row_occupancy.resize(rows_);

	// First pass: check whether existing icons are still valid, and compact them
	for (uint32_t idx = 0; idx < icons_.size(); ++idx) {
		Icon & icon = icons_[idx];
		Worker * worker = icon.worker.get(egbase());
		if (worker) {
			std::vector<Worker *>::iterator it = std::find(workerlist.begin(), workerlist.end(), worker);
			if (it != workerlist.end())
				workerlist.erase(it);
			else
				worker = nullptr;
		}

		if (!worker) {
			icons_.erase(icons_.begin() + idx);
			idx--;
			changes = true;
			continue;
		}

		while
			(icon.row &&
			 (row_occupancy[icon.row] >= kMaxColumns ||
			  icon.row * kMaxColumns + row_occupancy[icon.row] >= capacity))
			icon.row--;

		icon.col = row_occupancy[icon.row]++;
	}

	// Second pass: add new workers
	while (!workerlist.empty()) {
		Icon icon;
		icon.worker = workerlist.back();
		workerlist.pop_back();
		icon.row = 0;
		while (row_occupancy[icon.row] >= kMaxColumns)
			icon.row++;
		icon.col = row_occupancy[icon.row]++;
		icon.pos = calc_pos(icon.row, icon.col);

		// Let workers slide in from the right border
		icon.pos.x = get_w();

		std::vector<Icon>::iterator insertpos = icons_.begin();

		for (std::vector<Icon>::iterator icon_iter = icons_.begin();
			  icon_iter != icons_.end();
			  ++icon_iter) {

			if (icon_iter->row <= icon.row)
				insertpos = icon_iter + 1;

			icon.pos.x = std::max<int32_t>(icon.pos.x, icon_iter->pos.x + icon_width_);
		}

		icon.cache_experience = 0;

		icons_.insert(insertpos, icon);
		changes = true;
	}

	// Third pass: animate icons
	int32_t curtime = SDL_GetTicks();
	int32_t dt = std::min(std::max(curtime - last_animate_time_, 0), 1000);
	int32_t maxdist = dt * kAnimateSpeed / 1000;
	last_animate_time_ = curtime;

	for (Icon& icon : icons_) {
		Vector2i goal = calc_pos(icon.row, icon.col);
		Vector2i dp = goal - icon.pos;

		dp.x = std::min(std::max(dp.x, -maxdist), maxdist);
		dp.y = std::min(std::max(dp.y, -maxdist), maxdist);

		if (dp.x != 0 || dp.y != 0)
			changes = true;

		icon.pos += dp;

		// Check whether experience of the worker has changed
		Worker * worker = icon.worker.get(egbase());
		uint32_t experience = worker->get_current_experience();

		if (experience != icon.cache_experience) {
			icon.cache_experience = experience;
			changes = true;
		}
	}

	if (changes) {
		Vector2i mousepos = get_mouse_position();
		mouseover_fn_(find_worker(mousepos.x, mousepos.y));
	}
}

void WorkerPanel::draw(RenderTarget & dst)
{
	// Fill a region matching the current site capacity with black
	uint32_t capacity = workers_.capacity();
	uint32_t fullrows = capacity / kMaxColumns;

	if (fullrows)
		dst.fill_rect(Rectf(0, 0, get_w(), icon_height_ * fullrows), RGBAColor(0, 0, 0, 0));
	if (capacity % kMaxColumns)
		dst.fill_rect(Rectf(0, icon_height_ * fullrows,
			icon_width_ * (capacity % kMaxColumns), icon_height_), RGBAColor(0, 0, 0, 0));

	// Draw icons
	for (const Icon& icon : icons_) {
		const Worker * worker = icon.worker.get(egbase());
		if (!worker)
			continue;

		// This should probably call something similar to soldier::draw_info_icon()
		dst.blit(icon.pos.cast<float>() + Vector2f(kIconBorder, kIconBorder), worker->descr().icon());
//		constexpr float kNoZoom = 1.f;
//		worker->draw_info_icon(
//		   icon.pos.cast<float>() + Vector2f(kIconBorder, kIconBorder), kNoZoom, false, &dst);
		// TODO(Notabilis): Print experience on top of icon
	}
}

Vector2i WorkerPanel::calc_pos(uint32_t row, uint32_t col) const
{
	return Vector2i(col * icon_width_, row * icon_height_);
}

/**
 * Return the worker (if any) at the given coordinates.
 */
const Worker * WorkerPanel::find_worker(int32_t x, int32_t y) const
{
	for (const Icon& icon : icons_) {
		Rectf r(icon.pos, icon_width_, icon_height_);
		if (r.contains(Vector2i(x, y))) {
			return icon.worker.get(egbase());
		}
	}

	return nullptr;
}

void WorkerPanel::handle_mousein(bool inside)
{
	if (!inside && mouseover_fn_)
		mouseover_fn_(nullptr);
}

bool WorkerPanel::handle_mousemove
	(uint8_t /* state */,
	 int32_t x,
	 int32_t y,
	 int32_t /* xdiff */,
	 int32_t /* ydiff */)
{
	if (mouseover_fn_)
		mouseover_fn_(find_worker(x, y));
	return true;
}

bool WorkerPanel::handle_mousepress(uint8_t btn, int32_t x, int32_t y)
{
	if (btn == SDL_BUTTON_LEFT) {
		if (click_fn_) {
			if (const Worker * worker = find_worker(x, y))
				click_fn_(worker);
		}
		return true;
	}

	return false;
}

/**
 * List of workers
 */
struct WorkerList : UI::Box {
	WorkerList
		(UI::Panel & parent,
		 InteractiveGameBase & igb,
		 Building & building,
		 DescriptionIndex index,
		 WorkersQueue & workers_queue);

private:
	void mouseover(const Worker * worker);
	void eject(const Worker * worker);

	InteractiveGameBase& igbase_;
	Building& building_;
	DescriptionIndex index_;
	WorkersQueue & workers_queue_;
	WorkerPanel workerpanel_;
	UI::Textarea infotext_;
};

WorkerList::WorkerList
	(UI::Panel & parent,
	 InteractiveGameBase & igb,
	 Building & building,
	 DescriptionIndex index,
	 WorkersQueue & workers_queue)
:
UI::Box(&parent, 0, 0, UI::Box::Vertical),

igbase_(igb),
building_(building),
index_(index),
workers_queue_(workers_queue),
workerpanel_(*this, igb.egbase(), workers_queue),
infotext_(this, _("Click worker to send away"))
{
	add(&workerpanel_, UI::Align::kHCenter);

	add_space(2);

	add(&infotext_, UI::Align::kHCenter);

	workerpanel_.set_mouseover(boost::bind(&WorkerList::mouseover, this, _1));
	workerpanel_.set_click(boost::bind(&WorkerList::eject, this, _1));

	// We don't want translators to translate this twice, so it's a bit involved.
	int w = UI::g_fh1->render(
				  as_uifont((boost::format("%s ") // We need some extra space to fix bug 724169
								 /** TRANSLATORS: Workertype (current experience / required experience) */
								 % (boost::format(_("%1$s (Exp: %2$u/%3$u)"))
									 % 8 % 8 % 8)).str()))->width();
	uint32_t maxtextwidth = std::max(w,
												UI::g_fh1->render(as_uifont(_("Click worker to send away")))->width());
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box * buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	buttons->add_inf_space();
	buttons->add
		(create_worker_capacity_control(*buttons, igb, building, index_, workers_queue_),
		 UI::Align::kRight);

	add(buttons, UI::Align::kHCenter, true);
}

void WorkerList::mouseover(const Worker * worker)
{
	if (!worker) {
		infotext_.set_text(_("Click worker to send away"));
		return;
	}

	if (worker->needs_experience()) {
		infotext_.set_text(
			(boost::format(_("%1$s (Exp: %2$u/%3$u)"))
				% worker->descr().descname()
				% worker->get_current_experience()
				% worker->descr().get_needed_experience()
			).str()
		);
	} else {
		infotext_.set_text(
			(boost::format(_("%1$s (Exp: -/-)"))
				% worker->descr().descname()
		).str());
	}
}

void WorkerList::eject(const Worker * worker)
{
	uint32_t const capacity_min = 0;
	bool can_act = igbase_.can_act(building_.owner().player_number());
	bool over_min = capacity_min < workers_queue_.workers().size();

	if (can_act && over_min)
		igbase_.game().send_player_drop_worker(building_, worker->serial());
}

void add_worker_panel
	(UI::TabPanel * parent,
	 InteractiveGameBase & igb,
	 Widelands::Building & building,
	 DescriptionIndex index,
	 Widelands::WorkersQueue & workers_queue)
{

    const Widelands::WorkerDescr * desc = igb.egbase().tribes().get_worker_descr(workers_queue.get_worker());
	parent->add(desc->name(), desc->icon(),
                 new WorkerList(*parent, igb, building, index, workers_queue),
                 desc->descname());
}
