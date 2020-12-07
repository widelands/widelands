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

#include "ui_basic/color_chooser.h"

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"

namespace UI {

constexpr int16_t kSpacing = 4;
constexpr int16_t kMainDimension = 256;
constexpr int16_t kSidebarWidth = 16;

struct ColorChooserImpl : public Panel {

	ColorChooserImpl(Panel& parent, PanelStyle s, ColorChooser& c)
	   : Panel(&parent, s, 0, 0, kMainDimension + kSpacing + kSidebarWidth, kMainDimension),
	     chooser_(c),
	     selector_(*g_image_cache->get("images/ui_basic/fsel.png")),
	     texture_left_(kMainDimension, kMainDimension),
	     texture_right_(1, kMainDimension),
	     texture_left_r_(0),
	     texture_right_g_(0),
	     texture_right_b_(0) {
		update_texture_left();
		update_texture_right();
	}

	void draw(RenderTarget& dst) override {
		Panel::draw(dst);

		if (chooser_.get_color().r != texture_left_r_) {
			update_texture_left();
		}
		if (chooser_.get_color().g != texture_right_g_ ||
		    chooser_.get_color().b != texture_right_b_) {
			update_texture_right();
		}

		dst.blit(Vector2i(0, 0), &texture_left_);
		for (uint8_t i = 0; i < kSidebarWidth; ++i) {
			dst.blit(Vector2i(kMainDimension + kSpacing + i, 0), &texture_right_);
		}

		dst.blit(Vector2i(chooser_.get_color().g - selector_.width() / 2,
		                  chooser_.get_color().b - selector_.height() / 2),
		         &selector_);
		dst.blit(Vector2i(kMainDimension + kSpacing + (kSidebarWidth - selector_.width()) / 2,
		                  chooser_.get_color().r - selector_.height() / 2),
		         &selector_);
		const Vector2i mousepos = get_mouse_position();
		if (mousepos.y >= 0 && mousepos.y < kMainDimension && mousepos.x >= 0) {
			if (mousepos.x < kMainDimension) {
				dst.blit(
				   Vector2i(mousepos.x - selector_.width() / 2, mousepos.y - selector_.height() / 2),
				   &selector_);
			} else if (mousepos.x >= kMainDimension + kSpacing &&
			           mousepos.x < kMainDimension + kSpacing + kSidebarWidth) {
				dst.blit(Vector2i(kMainDimension + kSpacing + (kSidebarWidth - selector_.width()) / 2,
				                  mousepos.y - selector_.height() / 2),
				         &selector_);
			}
		}
	}

	RGBColor color_at(const int32_t x, const int32_t y) {
		if (y >= 0 && y < kMainDimension) {
			if (x >= 0 && x < kMainDimension) {
				return RGBColor(chooser_.get_color().r, x, y);
			}
			if (x >= kMainDimension + kSpacing && x < kMainDimension + kSpacing + kSidebarWidth) {
				return RGBColor(y, chooser_.get_color().g, chooser_.get_color().b);
			}
		}
		return chooser_.get_color();
	}

	bool handle_mousepress(const uint8_t btn, const int32_t x, const int32_t y) override {
		if (btn == SDL_BUTTON_LEFT) {
			chooser_.set_color(color_at(x, y));
			return true;
		}
		return Panel::handle_mousepress(btn, x, y);
	}

private:
	ColorChooser& chooser_;
	const Image& selector_;

	Texture texture_left_, texture_right_;
	uint8_t texture_left_r_, texture_right_g_, texture_right_b_;

	void update_texture_left() {
		texture_left_.lock();
		texture_left_r_ = chooser_.get_color().r;
		for (uint16_t g = 0; g < 256; ++g) {
			for (uint16_t b = 0; b < 256; ++b) {
				texture_left_.set_pixel(g, b, RGBColor(texture_left_r_, g, b));
			}
		}
		texture_left_.unlock(Texture::UnlockMode::Unlock_Update);
	}
	void update_texture_right() {
		texture_right_.lock();
		texture_right_g_ = chooser_.get_color().g;
		texture_right_b_ = chooser_.get_color().b;
		for (uint16_t r = 0; r < 256; ++r) {
			texture_right_.set_pixel(0, r, RGBColor(r, texture_right_g_, texture_right_b_));
		}
		texture_right_.unlock(Texture::UnlockMode::Unlock_Update);
	}
};

static inline const Image* preview(const RGBColor& c) {
	return playercolor_image(c, "images/players/player_position.png");
}

ColorChooser::ColorChooser(Panel* parent,
                           const WindowStyle s,
                           const RGBColor& init_color,
                           const RGBColor* default_color)
   : Window(parent, s, "choose_color", 0, 0, 0, 0, _("Choose Color…")),
     current_(init_color),
     main_box_(this, panel_style_, 0, 0, Box::Vertical),
     hbox_(&main_box_, panel_style_, 0, 0, Box::Horizontal),
     buttonsbox_(&main_box_, panel_style_, 0, 0, Box::Horizontal),
     vbox_(&hbox_, panel_style_, 0, 0, Box::Vertical),
     button_ok_(&buttonsbox_,
                "ok",
                0,
                0,
                30,
                0,
                s == WindowStyle::kWui ? ButtonStyle::kWuiPrimary : ButtonStyle::kFsMenuPrimary,
                _("OK")),
     button_cancel_(
        &buttonsbox_,
        "cancel",
        0,
        0,
        30,
        0,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        _("Cancel")),
     button_init_(
        &buttonsbox_,
        "initial_color",
        0,
        0,
        30,
        0,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        _("Initial Color")),
     button_default_(default_color ?
                        new Button(&buttonsbox_,
                                   "default_color",
                                   0,
                                   0,
                                   30,
                                   0,
                                   s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary :
                                                            ButtonStyle::kFsMenuSecondary,
                                   _("Default Color")) :
                        nullptr),
     spin_r_(&vbox_,
             0,
             0,
             300,
             200,
             init_color.r,
             0,
             255,
             panel_style_,
             _("Red"),
             SpinBox::Units::kNone,
             SpinBox::Type::kBig),
     spin_g_(&vbox_,
             0,
             0,
             300,
             200,
             init_color.g,
             0,
             255,
             panel_style_,
             _("Green"),
             SpinBox::Units::kNone,
             SpinBox::Type::kBig),
     spin_b_(&vbox_,
             0,
             0,
             300,
             200,
             init_color.b,
             0,
             255,
             panel_style_,
             _("Blue"),
             SpinBox::Units::kNone,
             SpinBox::Type::kBig),
     interactive_pane_(*new ColorChooserImpl(hbox_, panel_style_, *this)),
     icon_(
        &vbox_, panel_style_, 0, 0, kMainDimension / 2, kMainDimension / 2, preview(init_color)) {
	buttonsbox_.add_space(kSpacing);
	buttonsbox_.add(&button_cancel_, UI::Box::Resizing::kExpandBoth);
	buttonsbox_.add_space(kSpacing);
	buttonsbox_.add(&button_init_, UI::Box::Resizing::kExpandBoth);
	buttonsbox_.add_space(kSpacing);
	if (button_default_) {
		buttonsbox_.add(button_default_, UI::Box::Resizing::kExpandBoth);
		buttonsbox_.add_space(kSpacing);
		button_default_->sigclicked.connect([this, default_color]() { set_color(*default_color); });
	}
	buttonsbox_.add(&button_ok_, UI::Box::Resizing::kExpandBoth);
	buttonsbox_.add_space(kSpacing);

	button_ok_.sigclicked.connect(
	   [this]() { end_modal<Panel::Returncodes>(Panel::Returncodes::kOk); });
	button_cancel_.sigclicked.connect(
	   [this]() { end_modal<Panel::Returncodes>(Panel::Returncodes::kBack); });
	button_init_.sigclicked.connect([this, init_color]() { set_color(init_color); });
	spin_r_.changed.connect([this]() {
		set_color(RGBColor(spin_r_.get_value(), spin_g_.get_value(), spin_b_.get_value()));
	});
	spin_g_.changed.connect([this]() {
		set_color(RGBColor(spin_r_.get_value(), spin_g_.get_value(), spin_b_.get_value()));
	});
	spin_b_.changed.connect([this]() {
		set_color(RGBColor(spin_r_.get_value(), spin_g_.get_value(), spin_b_.get_value()));
	});

	vbox_.add(&spin_r_);
	vbox_.add_space(kSpacing);
	vbox_.add(&spin_g_);
	vbox_.add_space(kSpacing);
	vbox_.add(&spin_b_);
	vbox_.add_space(kSpacing);
	vbox_.add_inf_space();
	vbox_.add(&icon_, UI::Box::Resizing::kAlign, Align::kCenter);
	vbox_.add_inf_space();

	hbox_.add_space(kSpacing);
	hbox_.add(&interactive_pane_);
	hbox_.add_space(kSpacing);
	hbox_.add(&vbox_, UI::Box::Resizing::kFullSize);
	hbox_.add_space(kSpacing);

	main_box_.add_space(kSpacing);
	main_box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kSpacing);
	main_box_.add(&buttonsbox_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kSpacing);

	set_color(current_);

	set_center_panel(&main_box_);
	center_to_parent();
}

void ColorChooser::set_color(const RGBColor& color) {
	current_ = color;
	spin_r_.set_value(color.r);
	spin_g_.set_value(color.g);
	spin_b_.set_value(color.b);
	icon_.set_frame(color);
	icon_.set_icon(preview(color));
}

}  // namespace UI
