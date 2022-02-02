/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_COLOR_CHOOSER_H
#define WL_UI_BASIC_COLOR_CHOOSER_H

#include "graphic/playercolor.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/window.h"

namespace UI {

struct ColorChooserImpl;

enum class ColorAttribute { kRed, kGreen, kBlue };

class ColorChooser : public Window {
public:
	ColorChooser(Panel* parent,
	             WindowStyle,
	             const RGBColor& init_color,
	             const RGBColor* default_color);

	void set_color(const RGBColor&);
	const RGBColor& get_color() const {
		return current_;
	}

	bool handle_key(bool down, SDL_Keysym) override;

private:
	RGBColor current_;

	Box main_box_, hbox_, buttonsbox_, vbox_, box_r_, box_g_, box_b_, palette_box_1_, palette_box_2_;
	Button button_ok_, button_cancel_, button_init_, button_r_, button_g_, button_b_;
	Button* palette_buttons_[kMaxPlayers];
	Button* button_default_;
	SpinBox spin_r_, spin_g_, spin_b_;
	ColorChooserImpl& interactive_pane_;
	Icon icon_;

	void create_palette_button(unsigned index);
	void set_color_from_spinners();
	void set_sidebar_attribute(ColorAttribute);
};

}  // namespace UI

#endif  // WL_UI_BASIC_COLOR_CHOOSER_H
