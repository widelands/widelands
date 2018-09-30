/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

constexpr int32_t kUpdateTimeInGametimeMs = 1000;  //  1 second, gametime

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

uint32_t AttackBox::get_max_attackers() {
	assert(player_);

	if (upcast(Building, building, map_.get_immovable(*node_coordinates_))) {
		if (player_->vision(map_.get_index(building->get_position(), map_.get_width())) <= 1) {
			// Player can't see the buildings door, so it can't be attacked
			// This is the same check as done later on in send_player_enemyflagaction()
			return 0;
		}

		return player_->find_attack_soldiers(building->base_flag());
	}
	return 0;
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
	const int32_t gametime = player_->egbase().get_gametime();
	if ((gametime - lastupdate_) > kUpdateTimeInGametimeMs) {
		update_attack();
		lastupdate_ = gametime;
	}
}

void AttackBox::update_attack() {
	assert(soldiers_slider_.get());
	assert(soldiers_text_.get());
	assert(less_soldiers_.get());
	assert(more_soldiers_.get());

	int32_t max_attackers = get_max_attackers();

	if (soldiers_slider_->get_max_value() != max_attackers) {
		soldiers_slider_->set_max_value(max_attackers);
	}

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

	uint32_t max_attackers = get_max_attackers();

	UI::Box& linebox = *new UI::Box(this, 0, 0, UI::Box::Horizontal);
	add(&linebox);
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

	soldiers_slider_->changed.connect(boost::bind(&AttackBox::update_attack, this));
	more_soldiers_ = add_button(linebox, std::to_string(max_attackers),
	                            &AttackBox::send_more_soldiers, _("Send more soldiers"));

	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > 0);
	less_soldiers_->set_enabled(max_attackers > 0);
}

void AttackBox::send_less_soldiers() {
	assert(soldiers_slider_.get());
	soldiers_slider_->set_value(soldiers_slider_->get_value() - 1);
}

void AttackBox::send_more_soldiers() {
	soldiers_slider_->set_value(soldiers_slider_->get_value() + 1);
}

uint32_t AttackBox::soldiers() const {
	assert(soldiers_slider_.get());
	return soldiers_slider_->get_value();
}
