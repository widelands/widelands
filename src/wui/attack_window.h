/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_ATTACK_WINDOW_H
#define WL_WUI_ATTACK_WINDOW_H

#include <memory>

#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

/** Provides the attack settings when clicking on an enemy building. */
class AttackWindow : public UI::UniqueWindow {
public:
	AttackWindow(
	   InteractivePlayer& parent,
	   UI::UniqueWindow::Registry&,
	   Widelands::Building& target_bld,
	   const Widelands::Coords& target_coords,  // not necessarily the building's main location
	   bool fastclick);
	~AttackWindow() override;

	static UI::Window& load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader& mol);

protected:
	void think() override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kAttackWindow;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;

private:
	const unsigned serial_;

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

	void init_slider(const std::vector<Widelands::Soldier*>&);
	void init_soldier_lists(const std::vector<Widelands::Soldier*>&);
	void init_bottombox();

	void update(bool);
	void send_less_soldiers();
	void send_more_soldiers();

	size_t count_soldiers() const;
	std::vector<Widelands::Serial> soldiers() const;

	bool get_allow_conquer() const {
		return do_not_conquer_ && !do_not_conquer_->get_state();
	}

	InteractivePlayer& iplayer_;
	const Widelands::Map& map_;
	Widelands::Building& target_building_;
	const Widelands::Coords target_coordinates_;

	// A SoldierPanel is not applicable here as it's keyed to a building and thinks too much
	struct ListOfSoldiers : public UI::Panel {
		ListOfSoldiers(UI::Panel* const parent,
		               AttackWindow* parent_box,
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
		AttackWindow* attack_box_;

		void update_desired_size() override;
	};

	void act_attack();
	void act_goto();
	void act_debug();

	/// The last time the information in this Panel got updated
	Time lastupdate_;

	UI::Box mainbox_, linebox_, columnbox_, bottombox_;

	std::unique_ptr<ListOfSoldiers> attacking_soldiers_;
	std::unique_ptr<ListOfSoldiers> remaining_soldiers_;
	std::unique_ptr<UI::Button> attack_button_;
	std::unique_ptr<UI::Checkbox> do_not_conquer_;

	std::unique_ptr<UI::Slider> soldiers_slider_;
	std::unique_ptr<UI::Textarea> soldiers_text_;

	std::unique_ptr<UI::Button> less_soldiers_;
	std::unique_ptr<UI::Button> more_soldiers_;
};

#endif  // end of include guard: WL_WUI_ATTACK_WINDOW_H
