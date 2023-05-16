/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_WUI_FLEET_OPTIONS_WINDOW_H
#define WL_WUI_FLEET_OPTIONS_WINDOW_H

#include "logic/map_objects/bob.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

class InteractiveBase;

class FleetOptionsWindow : public UI::Window {
public:
	enum class Type { kShip, kFerry };

	static FleetOptionsWindow&
	create(UI::Panel* parent, InteractiveBase& ibase, Widelands::Bob* interface);
	~FleetOptionsWindow() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kFleetOptions;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader&);

protected:
	void think() override;

private:
	FleetOptionsWindow(UI::Panel* parent, InteractiveBase& ibase, Type t, Widelands::Bob* interface);

	void set_target(Widelands::Quantity target);
	Widelands::Quantity get_current_target() const;
	Widelands::Quantity previous_target_{0U};

	InteractiveBase& ibase_;
	bool can_act_;
	Type type_;
	Widelands::OPtr<Widelands::Bob> interface_;
	bool is_updating_{false};

	UI::Box main_box_;
	UI::Box buttons_box_;
	UI::SpinBox spinbox_;
	UI::Button infinite_target_;
	UI::Textarea* txt_ships_{nullptr};
	UI::Textarea* txt_ports_{nullptr};
	UI::Textarea* txt_ferries_total_{nullptr};
	UI::Textarea* txt_ferries_unemployed_{nullptr};
	UI::Textarea* txt_waterways_total_{nullptr};
	UI::Textarea* txt_waterways_lacking_{nullptr};
};

#endif  // end of include guard: WL_WUI_FLEET_OPTIONS_WINDOW_H
