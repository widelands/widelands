/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "wui/attack_box.h"

#include <memory>
#include <string>

#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "logic/map_objects/tribes/soldier.h"

constexpr int32_t kUpdateTimeInGametimeMs = 500;  //  half a second, gametime

AttackBox::AttackBox(UI::Panel* parent,
                     Widelands::Player* player,
                     Widelands::FCoords* target,
                     uint32_t const x,
                     uint32_t const y)
   : UI::Box(parent, x, y, UI::Box::Vertical),
     player_(player),
     map_(player_->egbase().map()),
     node_coordinates_(target),
     lastupdate_(0) {
	init();
}

std::vector<Widelands::Soldier*> AttackBox::get_max_attackers() {
	assert(player_);

	if (upcast(Building, building, map_.get_immovable(*node_coordinates_))) {
		if (player_->vision(map_.get_index(building->get_position(), map_.get_width())) > 1) {
			std::vector<Widelands::Soldier*> v;
			// TODO(Nordfriese): This method decides by itself which soldier remains in the building.
			// This soldier will not show up in the result vector. Perhaps we should show all
			// available soldiers, grouped by building, so the player can choose between all soldiers
			// knowing that at least one of each group will have to stay at home. However, this
			// could clutter up the screen a lot. Especially if you have many small buildings.
			player_->find_attack_soldiers(building->base_flag(), &v);
			return v;
		}
		// Player can't see the buildings door, so it can't be attacked
		// This is the same check as done later on in send_player_enemyflagaction()
	}
	return std::vector<Widelands::Soldier*>();
}

std::unique_ptr<UI::HorizontalSlider> AttackBox::add_slider(UI::Box& parent,
                                                            uint32_t width,
                                                            uint32_t height,
                                                            uint32_t min,
                                                            uint32_t max,
                                                            uint32_t initial,
                                                            char const* hint) {
	std::unique_ptr<UI::HorizontalSlider> result(new UI::HorizontalSlider(
	   &parent, 0, 0, width, height, min, max, initial, UI::SliderStyle::kWuiDark, hint));
	parent.add(result.get());
	return result;
}

UI::Textarea&
AttackBox::add_text(UI::Box& parent, std::string str, UI::Align alignment, int fontsize) {
	UI::Textarea& result = *new UI::Textarea(&parent, str.c_str());
	result.set_fontsize(fontsize);
	parent.add(&result, UI::Box::Resizing::kAlign, alignment);
	return result;
}

std::unique_ptr<UI::Button> AttackBox::add_button(UI::Box& parent,
                                                  const std::string& text,
                                                  void (AttackBox::*fn)(),
                                                  const std::string& tooltip_text) {
	std::unique_ptr<UI::Button> button(new UI::Button(
	   &parent, text, 8, 8, 26, 26, UI::ButtonStyle::kWuiPrimary, text, tooltip_text));
	button->sigclicked.connect(boost::bind(fn, boost::ref(*this)));
	parent.add(button.get());
	return button;
}

/*
 * Update available soldiers
 */
void AttackBox::think() {
	if ((player_->egbase().get_gametime() - lastupdate_) > kUpdateTimeInGametimeMs) {
		update_attack(false);
	}
}

void AttackBox::update_attack(bool action_on_panel) {
	lastupdate_ = player_->egbase().get_gametime();

	assert(soldiers_slider_.get());
	assert(soldiers_text_.get());
	assert(less_soldiers_.get());
	assert(more_soldiers_.get());
	assert(attacking_soldiers_.get());
	assert(remaining_soldiers_.get());

	std::vector<Widelands::Soldier*> all_attackers = get_max_attackers();
	const int max_attackers = all_attackers.size();

	// Update number of available soldiers
	if (soldiers_slider_->get_max_value() != max_attackers) {
		soldiers_slider_->set_max_value(max_attackers);
	}

	// Add new soldiers and remove missing soldiers to/from the list
	for (const auto& s : all_attackers) {
		if (!attacking_soldiers_->contains(s) && !remaining_soldiers_->contains(s)) {
			remaining_soldiers_->add(s);
		}
	}
	for (const auto& s : remaining_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			remaining_soldiers_->remove(s);
		}
	}
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			attacking_soldiers_->remove(s);
		}
	}

	if (action_on_panel) {
		// The player clicked on soldiers in the list – update slider
		soldiers_slider_->set_value(attacking_soldiers_->count_soldiers());
	} else {
		// The slider was moved or we were called from think() – shift lacking/extra soldiers between the lists
		const int32_t lacking = soldiers_slider_->get_value() - attacking_soldiers_->count_soldiers();
		if (lacking > 0) {
			for (int32_t i = 0; i < lacking; ++i) {
				const Widelands::Soldier* s = remaining_soldiers_->get_soldier();
				remaining_soldiers_->remove(s);
				attacking_soldiers_->add(s);
			}
		} else if (lacking < 0) {
			for (int32_t i = 0; i > lacking; --i) {
				const Widelands::Soldier* s = attacking_soldiers_->get_soldier();
				attacking_soldiers_->remove(s);
				remaining_soldiers_->add(s);
			}
		}
	}

	// Update slider, buttons and texts
	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > soldiers_slider_->get_value());
	less_soldiers_->set_enabled(soldiers_slider_->get_value() > 0);

	soldiers_text_->set_text(
	   /** TRANSLATORS: %1% of %2% soldiers. Used in Attack box. */
	   (boost::format(_("%1% / %2%")) % soldiers_slider_->get_value() % max_attackers).str());

	more_soldiers_->set_title(std::to_string(max_attackers));
}

void AttackBox::init() {
	assert(node_coordinates_);

	std::vector<Widelands::Soldier*> all_attackers = get_max_attackers();
	const size_t max_attackers = all_attackers.size();

	UI::Box& mainbox = *new UI::Box(this, 0, 0, UI::Box::Vertical);
	add(&mainbox);

	UI::Box& linebox = *new UI::Box(&mainbox, 0, 0, UI::Box::Horizontal);
	mainbox.add(&linebox);
	add_text(linebox, _("Soldiers:"));
	linebox.add_space(8);

	less_soldiers_ =
	   add_button(linebox, "0", &AttackBox::send_less_soldiers, _("Send less soldiers"));

	//  Spliter of soldiers
	UI::Box& columnbox = *new UI::Box(&linebox, 0, 0, UI::Box::Vertical);
	linebox.add(&columnbox);

	const std::string attack_string =
	   (boost::format(_("%1% / %2%")) % (max_attackers > 0 ? 1 : 0) % max_attackers).str();

	soldiers_text_.reset(
	   &add_text(columnbox, attack_string, UI::Align::kCenter, UI_FONT_SIZE_ULTRASMALL));

	soldiers_slider_ = add_slider(
	   columnbox, 100, 10, 0, max_attackers, max_attackers > 0 ? 1 : 0, _("Number of soldiers"));

	attacking_soldiers_.reset(new ListOfSoldiers(&mainbox, this, 0, 0, 30, 30,
			(boost::format("%s<br>%s") % _("Remove this soldier from the list of attackers") %
					_("Hold down Ctrl to remove all soldiers from the list")).str()));
	remaining_soldiers_.reset(new ListOfSoldiers(&mainbox, this, 0, 0, 30, 30,
			(boost::format("%s<br>%s") % _("Add this soldier to the list of attackers") %
					_("Hold down Ctrl to add all soldiers to the list")).str()));
	attacking_soldiers_->set_complement(remaining_soldiers_.get());
	remaining_soldiers_->set_complement(attacking_soldiers_.get());

	for (const auto& s : all_attackers) {
		remaining_soldiers_->add(s);
	}

	add_text(mainbox, _("Attackers:"));
	mainbox.add(attacking_soldiers_.get(), UI::Box::Resizing::kFullSize);
	add_text(mainbox, _("Not attacking:"));
	mainbox.add(remaining_soldiers_.get(), UI::Box::Resizing::kFullSize);

	soldiers_slider_->changed.connect([this]() { update_attack(false); });
	more_soldiers_ = add_button(linebox, std::to_string(max_attackers),
	                            &AttackBox::send_more_soldiers, _("Send more soldiers"));

	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > 0);
}

void AttackBox::send_less_soldiers() {
	assert(soldiers_slider_.get());
	soldiers_slider_->set_value(soldiers_slider_->get_value() - 1);
}

void AttackBox::send_more_soldiers() {
	soldiers_slider_->set_value(soldiers_slider_->get_value() + 1);
}

size_t AttackBox::count_soldiers() const {
	return attacking_soldiers_->count_soldiers();
}

std::vector<Widelands::Serial> AttackBox::soldiers() const {
	std::vector<Widelands::Serial> result;
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		result.push_back(s->serial());
	}
	return result;
}

constexpr int kSoldierIconWidth = 30;
constexpr int kSoldierIconHeight = 29;

AttackBox::ListOfSoldiers::ListOfSoldiers(UI::Panel* const parent,
	           AttackBox* parent_box,
	           int32_t const x,
	           int32_t const y,
	           int const w,
	           int const h,
               const std::string& tooltip,
               int16_t max_size,
               bool restrict_rows)
   : UI::Panel(parent, x, y, w, h, tooltip),
     size_restriction_(max_size),
     restricted_row_number_(restrict_rows),
     attack_box_(parent_box) {
	update_desired_size();
}

bool AttackBox::ListOfSoldiers::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT || !other_) {
		return UI::Panel::handle_mousepress(btn, x, y);
	}
	if (SDL_GetModState() & KMOD_CTRL) {
		for (const auto& s : get_soldiers()) {
			remove(s);
			other_->add(s);
		}
	} else {
		const Widelands::Soldier* s = soldier_at(x, y);
		if (!s) {
			return UI::Panel::handle_mousepress(btn, x, y);
		}
		remove(s);
		other_->add(s);
	}
	attack_box_->update_attack(true);
	return true;
}

Widelands::Extent AttackBox::ListOfSoldiers::size() const {
	const size_t nr_soldiers = count_soldiers();
	if (nr_soldiers == 0) {
		// At least one icon
		return Widelands::Extent(1, 1);
	}
	uint32_t rows = nr_soldiers / size_restriction_;
	if (rows * size_restriction_ < nr_soldiers) {
		++rows;
	}
	if (restricted_row_number_) {
		return Widelands::Extent(rows, size_restriction_);
	} else {
		return Widelands::Extent(size_restriction_, rows);
	}
}

void AttackBox::ListOfSoldiers::update_desired_size() {
	const Widelands::Extent e = size();
	set_desired_size(e.w * kSoldierIconWidth, e.h * kSoldierIconHeight);
}

const Widelands::Soldier* AttackBox::ListOfSoldiers::soldier_at(int32_t x, int32_t y) const {
	if (x < 0 || y < 0 || soldiers_.empty()) {
		return nullptr;
	}
	const int32_t col = x / kSoldierIconWidth;
	const int32_t row = y / kSoldierIconHeight;
	assert(col >= 0);
	assert(row >= 0);
	if ((restricted_row_number_ ? row : col) >= size_restriction_) {
		return nullptr;
	}
	const int index = restricted_row_number_ ? size_restriction_ * col + row : size_restriction_ * row + col;
	assert(index >= 0);
	return static_cast<unsigned int>(index) < soldiers_.size() ? soldiers_[index] : nullptr;
}

void AttackBox::ListOfSoldiers::add(const Widelands::Soldier* s) {
	soldiers_.push_back(s);
	update_desired_size();
}

void AttackBox::ListOfSoldiers::remove(const Widelands::Soldier* s) {
	const auto it = std::find(soldiers_.begin(), soldiers_.end(), s);
	assert(it != soldiers_.end());
	soldiers_.erase(it);
	update_desired_size();
}

void AttackBox::ListOfSoldiers::draw(RenderTarget& dst) {
	const size_t nr_soldiers = soldiers_.size();
	int32_t column = 0;
	int32_t row = 0;
	for (uint32_t i = 0; i < nr_soldiers; ++i) {
		Vector2i location(column * kSoldierIconWidth, row * kSoldierIconHeight);
		soldiers_[i]->draw_info_icon(location, 1.0f, false, &dst);
		if (restricted_row_number_) {
			++row;
			if (row >= size_restriction_) {
				row = 0;
				++column;
			}
		} else {
			++column;
			if (column >= size_restriction_) {
				column = 0;
				++row;
			}
		}
	}
}

