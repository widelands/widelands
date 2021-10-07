/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#ifndef WL_WUI_ATTACK_BOX_H
#define WL_WUI_ATTACK_BOX_H

#include <memory>

#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"

using Widelands::Bob;
using Widelands::Building;
using Widelands::Soldier;

/**
 * Provides the attack settings that are part of a \ref FieldActionWindow
 * when clicking on an enemy building.
 */
struct AttackBox : public UI::Box {
	AttackBox(UI::Panel* parent,
	          Widelands::Player* player,
	          Widelands::FCoords* target,
	          uint32_t const x,
	          uint32_t const y);

	void init();

	size_t count_soldiers() const;
	std::vector<Widelands::Serial> soldiers() const;

	UI::Button* get_attack_button() const {
		return attack_button_.get();
	}

	bool get_allow_conquer() const {
		return do_not_conquer_ && !do_not_conquer_->get_state();
	}

private:
	std::vector<Widelands::Soldier*> get_max_attackers();
	std::unique_ptr<UI::HorizontalSlider> add_slider(UI::Box& parent,
	                                                 uint32_t width,
	                                                 uint32_t height,
	                                                 uint32_t min,
	                                                 uint32_t max,
	                                                 uint32_t initial,
	                                                 char const* hint);
	UI::Textarea& add_text(UI::Box& parent,
	                       const std::string& str,
	                       UI::Align alignment,
	                       const UI::FontStyle style);
	std::unique_ptr<UI::Button> add_button(UI::Box& parent,
	                                       const std::string& text,
	                                       void (AttackBox::*fn)(),
	                                       const std::string& tooltip_text);

	void think() override;
	void update_attack(bool);
	void send_less_soldiers();
	void send_more_soldiers();

public:
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

private:
	Widelands::Player* player_;
	const Widelands::Map& map_;
	Widelands::FCoords* node_coordinates_;

	std::unique_ptr<UI::Slider> soldiers_slider_;
	std::unique_ptr<UI::Textarea> soldiers_text_;

	std::unique_ptr<UI::Button> less_soldiers_;
	std::unique_ptr<UI::Button> more_soldiers_;

	// A SoldierPanel is not applicable here as it's keyed to a building and thinks too much
	struct ListOfSoldiers : public UI::Panel {
		ListOfSoldiers(UI::Panel* const parent,
		               AttackBox* parent_box,
		               int32_t const x,
		               int32_t const y,
		               int const w,
		               int const h,
		               bool restrict_rows = false);

		bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
		void handle_mousein(bool) override;
		bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;

		const Widelands::Soldier* soldier_at(int32_t x, int32_t y) const;
		void add(const Widelands::Soldier*);
		void remove(const Widelands::Soldier*);
		bool contains(const Widelands::Soldier* soldier) const {
			for (const auto& s : soldiers_) {
				if (s == soldier) {
					return true;
				}
			}
			return false;
		}

		std::vector<const Widelands::Soldier*> get_soldiers() const {
			return soldiers_;
		}
		const Widelands::Soldier* get_soldier() const {
			return soldiers_.back();
		}

		size_t count_soldiers() const {
			return soldiers_.size();
		}
		Widelands::Extent size() const;
		bool row_number_restricted() const {
			return restricted_row_number_;
		}
		void set_row_number_restricted(bool r) {
			restricted_row_number_ = r;
		}

		void draw(RenderTarget& dst) override;

		void set_complement(ListOfSoldiers* o) {
			other_ = o;
		}

	private:
		bool restricted_row_number_;
		uint16_t current_size_;  // Current number of rows or columns
		std::vector<const Widelands::Soldier*> soldiers_;

		ListOfSoldiers* other_;
		AttackBox* attack_box_;

		void update_desired_size() override;
	};

	std::unique_ptr<ListOfSoldiers> attacking_soldiers_;
	std::unique_ptr<ListOfSoldiers> remaining_soldiers_;
	std::unique_ptr<UI::Button> attack_button_;
	std::unique_ptr<UI::Checkbox> do_not_conquer_;

	/// The last time the information in this Panel got updated
	Time lastupdate_;
};

#endif  // end of include guard: WL_WUI_ATTACK_BOX_H
