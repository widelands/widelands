/*
 * Copyright (C) 2010-2023 by the Widelands Development Team
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

#include "wui/quicknavigation.h"

#include "graphic/text_layout.h"
#include "logic/game_data_error.h"
#include "ui_basic/editbox.h"
#include "wlapplication_options.h"
#include "wui/interactive_gamebase.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/watchwindow.h"

/* Logic backend. */

QuickNavigation::QuickNavigation(MapView* map_view) : map_view_(map_view) {
	landmarks_.resize(kQuicknavSlots);
	map_view->changeview.connect([this] { view_changed(); });
	map_view->jump.connect([this] { jumped(); });
	havefirst_ = false;
}

inline void QuickNavigation::insert_if_applicable(std::list<MapView::View>& l) {
	if (l.empty() || !l.back().view_roughly_near(current_)) {
		l.push_back(current_);
	}
}

void QuickNavigation::jumped() {
	if (!location_jumping_started_) {
		assert(previous_locations_.empty());
		assert(next_locations_.empty());
		location_jumping_started_ = true;
		return;
	}
	next_locations_.clear();
	insert_if_applicable(previous_locations_);
}

void QuickNavigation::view_changed() {
	current_ = map_view_->get_centered_view();
	havefirst_ = true;
}

void QuickNavigation::set_landmark(size_t index, const MapView::View& view) {
	assert(index < landmarks_.size());
	landmarks_[index].view = view;
	landmarks_[index].set = true;
	Notifications::publish(NoteQuicknavChangedEvent());
}

void QuickNavigation::unset_landmark(size_t index) {
	assert(index < landmarks_.size());
	landmarks_[index].set = false;
	Notifications::publish(NoteQuicknavChangedEvent());
}

void QuickNavigation::add_landmark() {
	landmarks_.emplace_back();
	Notifications::publish(NoteQuicknavChangedEvent());
}

void QuickNavigation::remove_landmark(size_t index) {
	assert(index >= kQuicknavSlots);
	assert(index < landmarks_.size());
	landmarks_.erase(landmarks_.begin() + index);
	Notifications::publish(NoteQuicknavChangedEvent());
}

bool QuickNavigation::handle_key(bool down, SDL_Keysym key) {
	if (!havefirst_ || !down) {
		return false;
	}

	auto check_landmark = [this, key](const uint8_t i) {
		// This function assumes that the shortcut entries are ordered
		// Set1,Goto1,Set2,Goto2,Set3,Goto3,etc
		if (matches_shortcut(KeyboardShortcut::kInGameQuicknavSet1 + 2 * i, key)) {
			set_landmark_to_current(i);
			return true;
		}
		if (landmarks_[i].set &&
		    matches_shortcut(KeyboardShortcut::kInGameQuicknavGoto1 + 2 * i, key)) {
			goto_landmark(i);
			return true;
		}
		return false;
	};
	for (uint8_t i = 0; i < 9; ++i) {
		if (check_landmark(i)) {
			return true;
		}
	}

	if (matches_shortcut(KeyboardShortcut::kCommonQuicknavPrev, key) && can_goto_prev()) {
		// go to previous location
		goto_prev();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kCommonQuicknavNext, key) && can_goto_next()) {
		// go to next location
		goto_next();
		return true;
	}

	return false;
}

bool QuickNavigation::can_goto_prev() const {
	return !previous_locations_.empty();
}

bool QuickNavigation::can_goto_next() const {
	return !next_locations_.empty();
}

void QuickNavigation::goto_landmark(int index) {
	map_view_->set_centered_view(landmarks_[index].view, MapView::Transition::Smooth);
}

void QuickNavigation::goto_prev() {
	if (!can_goto_prev()) {
		return;
	}
	insert_if_applicable(next_locations_);
	map_view_->set_centered_view(previous_locations_.back(), MapView::Transition::Smooth);
	previous_locations_.pop_back();
}

void QuickNavigation::goto_next() {
	if (!can_goto_next()) {
		return;
	}
	insert_if_applicable(previous_locations_);
	map_view_->set_centered_view(next_locations_.back(), MapView::Transition::Smooth);
	next_locations_.pop_back();
}

/* GUI window. */

constexpr int kButtonSize = 34;
constexpr int kSpacing = 4;

QuickNavigationWindow::QuickNavigationWindow(InteractiveBase& ibase, UI::UniqueWindow::Registry& r)
   : UI::UniqueWindow(
        &ibase, UI::WindowStyle::kWui, "quicknav", &r, 100, 100, _("Quick Navigation")),
     ibase_(ibase),
     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     buttons_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     prev_(&buttons_box_,
           "prev",
           0,
           0,
           kButtonSize,
           kButtonSize,
           UI::ButtonStyle::kWuiSecondary,
           g_image_cache->get("images/ui_basic/scrollbar_left.png"),
           as_tooltip_text_with_hotkey(
              _("Go to previous location"),
              shortcut_string_for(KeyboardShortcut::kCommonQuicknavPrev, true),
              UI::PanelStyle::kWui)),
     next_(&buttons_box_,
           "next",
           0,
           0,
           kButtonSize,
           kButtonSize,
           UI::ButtonStyle::kWuiSecondary,
           g_image_cache->get("images/ui_basic/scrollbar_right.png"),
           as_tooltip_text_with_hotkey(
              _("Go to next location"),
              shortcut_string_for(KeyboardShortcut::kCommonQuicknavNext, true),
              UI::PanelStyle::kWui)),
     new_(&buttons_box_,
          "new",
          0,
          0,
          kButtonSize,
          kButtonSize,
          UI::ButtonStyle::kWuiSecondary,
          _("+"),
          _("Add a new landmark")) {
	prev_.sigclicked.connect([this]() { ibase_.quick_navigation().goto_prev(); });
	next_.sigclicked.connect([this]() { ibase_.quick_navigation().goto_next(); });
	new_.sigclicked.connect([this]() { ibase_.quick_navigation().add_landmark(); });
	subscriber_ = Notifications::subscribe<NoteQuicknavChangedEvent>(
	   [this](const NoteQuicknavChangedEvent& /* note */) { rebuild(); });

	buttons_box_.add_inf_space();
	buttons_box_.add(&prev_, UI::Box::Resizing::kFillSpace);
	buttons_box_.add_inf_space();
	buttons_box_.add(&new_, UI::Box::Resizing::kFillSpace);
	buttons_box_.add_inf_space();
	buttons_box_.add(&next_, UI::Box::Resizing::kFillSpace);
	buttons_box_.add_inf_space();

	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kSpacing);

	set_center_panel(&main_box_);
	if (get_usedefaultpos()) {
		center_to_parent();
	}
	rebuild();
}

void QuickNavigationWindow::rebuild() {
	if (content_box_ != nullptr) {
		content_box_.release()->do_delete();
	}
	content_box_.reset(
	   new UI::Box(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, kSpacing));

	QuickNavigation& q = ibase_.quick_navigation();
	for (unsigned i = 0; i < q.landmarks().size(); ++i) {
		UI::Box& box = *new UI::Box(
		   content_box_.get(), UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, kSpacing);

		UI::Button* b = new UI::Button(
		   &box, format("goto_%u", i), 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menus/goto.png"),
		   i < kQuicknavSlots ?
            as_tooltip_text_with_hotkey(
		         _("Go to this landmark"),
		         shortcut_string_for(KeyboardShortcut::kInGameQuicknavGoto1 + 2 * i, true),
		         UI::PanelStyle::kWui) :
            _("Go to this landmark"));
		b->set_enabled(q.landmarks()[i].set);
		b->sigclicked.connect([&q, i]() { q.goto_landmark(i); });
		box.add(b);

		b = new UI::Button(&box, format("watch_%u", i), 0, 0, kButtonSize, kButtonSize,
		                   UI::ButtonStyle::kWuiSecondary,
		                   g_image_cache->get("images/wui/fieldaction/menu_watch_field.png"),
		                   _("View this landmark in a watch window"));
		b->set_enabled(q.landmarks()[i].set);
		b->sigclicked.connect([this, &q, i]() {
			show_watch_window(
			   dynamic_cast<InteractiveGameBase&>(ibase_),
			   MapviewPixelFunctions::calc_node_and_triangle(
			      ibase_.egbase().map(), static_cast<int>(q.landmarks()[i].view.viewpoint.x),
			      static_cast<int>(q.landmarks()[i].view.viewpoint.y))
			      .node);
		});
		box.add(b);

		b = new UI::Button(&box, format("clear_%u", i), 0, 0, kButtonSize, kButtonSize,
		                   UI::ButtonStyle::kWuiSecondary,
		                   g_image_cache->get("images/wui/menu_abort.png"), _("Unset this landmark"));
		b->set_enabled(q.landmarks()[i].set);
		b->sigclicked.connect([&q, i]() { q.unset_landmark(i); });
		box.add(b);

		b = new UI::Button(
		   &box, format("set_%u", i), 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
		   g_image_cache->get("images/wui/menus/quicknav_set.png"),
		   i < kQuicknavSlots ?
            as_tooltip_text_with_hotkey(
		         _("Set this landmark to the current map view location"),
		         shortcut_string_for(KeyboardShortcut::kInGameQuicknavSet1 + 2 * i, true),
		         UI::PanelStyle::kWui) :
            _("Set this landmark to the current map view location"));
		b->sigclicked.connect([&q, i]() { q.set_landmark_to_current(i); });
		box.add(b);

		if (i < kQuicknavSlots) {
			box.add_space(kButtonSize);
		} else {
			b = new UI::Button(&box, format("remove_%u", i), 0, 0, kButtonSize, kButtonSize,
			                   UI::ButtonStyle::kWuiSecondary, _("–"), _("Remove this landmark"));
			b->sigclicked.connect([&q, i]() { q.remove_landmark(i); });
			box.add(b);
		}

		UI::EditBox& e = *new UI::EditBox(&box, 0, 0, 300, UI::PanelStyle::kWui);
		e.set_text(q.landmarks()[i].name);
		e.changed.connect([&q, &e, i]() { q.landmarks()[i].name = e.text(); });
		box.add(&e, UI::Box::Resizing::kExpandBoth);

		content_box_->add(&box, UI::Box::Resizing::kFullSize);
	}

	content_box_->set_scrolling(true);
	main_box_.add(content_box_.get(), UI::Box::Resizing::kExpandBoth);
	initialization_complete();
}

void QuickNavigationWindow::think() {
	UI::UniqueWindow::think();

	prev_.set_enabled(ibase_.quick_navigation().can_goto_prev());
	next_.set_enabled(ibase_.quick_navigation().can_goto_next());
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& QuickNavigationWindow::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = ib.quicknav_registry_;
			r.create();
			assert(r.window);
			QuickNavigationWindow& m = dynamic_cast<QuickNavigationWindow&>(*r.window);
			return m;
		}
		throw Widelands::UnhandledVersionError(
		   "Quick Navigation", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("quick navigation: %s", e.what());
	}
}
void QuickNavigationWindow::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	// Nothing to save currently.
}
