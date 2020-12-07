/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_COLOR_CHOOSER_H
#define WL_UI_BASIC_COLOR_CHOOSER_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/window.h"

namespace UI {

struct ColorChooserImpl;

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

private:
	RGBColor current_;

	Box main_box_, hbox_, buttonsbox_, vbox_;
	Button button_ok_;
	Button button_cancel_;
	Button button_init_;
	Button* button_default_;
	SpinBox spin_r_, spin_g_, spin_b_;
	ColorChooserImpl& interactive_pane_;
	Icon icon_;
};

}  // namespace UI

#endif  // WL_UI_BASIC_COLOR_CHOOSER_H
