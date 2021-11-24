/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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
#include "graphic/rendertarget.h"
#include "graphic/texture.h"

namespace UI {

constexpr int16_t kSpacing = 4;
constexpr int16_t kMainDimension = 256;
constexpr int16_t kSidebarWidth = 16;
constexpr int16_t kButtonSize = 30;

struct ColorChooserImpl : public Panel {

	ColorChooserImpl(Panel& parent, PanelStyle s, ColorChooser& c)
	   : Panel(&parent, s, 0, 0, kMainDimension + kSpacing + kSidebarWidth, kMainDimension),
	     chooser_(c),
	     sidebar_attribute_(ColorAttribute::kRed),
	     selector_(*g_image_cache->get("images/ui_basic/fsel.png")),
	     dragging_(false),
	     texture_left_(kMainDimension, kMainDimension),
	     texture_right_(1, kMainDimension),
	     texture_cache_r_(0),
	     texture_cache_g_(0),
	     texture_cache_b_(0) {
	}

	ColorAttribute sidebar_attribute() const {
		return sidebar_attribute_;
	}
	void set_sidebar_attribute(ColorAttribute a) {
		sidebar_attribute_ = a;
		update_textures();
	}

	void draw(RenderTarget& dst) override {
		Panel::draw(dst);

		if (chooser_.get_color().r != texture_cache_r_ ||
		    chooser_.get_color().g != texture_cache_g_ ||
		    chooser_.get_color().b != texture_cache_b_) {
			update_textures();
		}

		draw_textures(dst);

		draw_selection_cursors(dst);
		draw_mouse_cursors(dst);
	}

	RGBColor color_at(const int32_t x, const int32_t y) {
		if (y >= 0 && y < kMainDimension) {
			if (x >= 0 && x < kMainDimension) {
				return left_color_at(x, y);
			}
			if (x >= kMainDimension + kSpacing && x < kMainDimension + kSpacing + kSidebarWidth) {
				return right_color_at(y);
			}
		}
		return chooser_.get_color();
	}

	bool handle_mouserelease(const uint8_t btn, const int32_t x, const int32_t y) override {
		if (btn == SDL_BUTTON_LEFT) {
			dragging_ = false;
			return true;
		}
		return Panel::handle_mouserelease(btn, x, y);
	}
	bool handle_mousepress(const uint8_t btn, const int32_t x, const int32_t y) override {
		if (btn == SDL_BUTTON_LEFT) {
			dragging_ = true;
			set_color_at_mouse(x, y);
			return true;
		}
		return Panel::handle_mousepress(btn, x, y);
	}
	bool handle_mousemove(
	   const uint8_t s, const int32_t x, const int32_t y, int32_t dx, int32_t dy) override {
		if (dragging_) {
			set_color_at_mouse(x, y);
			return true;
		}
		return Panel::handle_mousemove(s, x, y, dx, dy);
	}

private:
	ColorChooser& chooser_;
	ColorAttribute sidebar_attribute_;
	const Image& selector_;
	bool dragging_;

	Texture texture_left_, texture_right_;
	uint8_t texture_cache_r_, texture_cache_g_, texture_cache_b_;

	inline void set_color_at_mouse(const int32_t x, const int32_t y) {
		chooser_.set_color(color_at(x, y));
	}

	RGBColor left_color(const uint16_t val1, const uint16_t val2) const {
		switch (sidebar_attribute_) {
		case ColorAttribute::kRed:
			return RGBColor(texture_cache_r_, val1, val2);
		case ColorAttribute::kGreen:
			return RGBColor(val1, texture_cache_g_, val2);
		case ColorAttribute::kBlue:
			return RGBColor(val1, val2, texture_cache_b_);
		}
		NEVER_HERE();
	}
	RGBColor right_color(const uint16_t val) const {
		switch (sidebar_attribute_) {
		case ColorAttribute::kRed:
			return RGBColor(val, texture_cache_g_, texture_cache_b_);
		case ColorAttribute::kGreen:
			return RGBColor(texture_cache_r_, val, texture_cache_b_);
		case ColorAttribute::kBlue:
			return RGBColor(texture_cache_r_, texture_cache_g_, val);
		}
		NEVER_HERE();
	}

	RGBColor left_color_at(const uint16_t x, const uint16_t y) {
		switch (sidebar_attribute_) {
		case ColorAttribute::kRed:
			return RGBColor(chooser_.get_color().r, x, y);
		case ColorAttribute::kGreen:
			return RGBColor(x, chooser_.get_color().g, y);
		case ColorAttribute::kBlue:
			return RGBColor(x, y, chooser_.get_color().b);
		}
		NEVER_HERE();
	}
	RGBColor right_color_at(const uint16_t y) {
		switch (sidebar_attribute_) {
		case ColorAttribute::kRed:
			return RGBColor(y, chooser_.get_color().g, chooser_.get_color().b);
		case ColorAttribute::kGreen:
			return RGBColor(chooser_.get_color().r, y, chooser_.get_color().b);
		case ColorAttribute::kBlue:
			return RGBColor(chooser_.get_color().r, chooser_.get_color().g, y);
		}
		NEVER_HERE();
	}

	void update_textures() {
		texture_cache_r_ = chooser_.get_color().r;
		texture_cache_g_ = chooser_.get_color().g;
		texture_cache_b_ = chooser_.get_color().b;

		texture_left_.lock();
		for (uint16_t x = 0; x < kMainDimension; ++x) {
			for (uint16_t y = 0; y < kMainDimension; ++y) {
				texture_left_.set_pixel(x, y, left_color(x, y));
			}
		}
		texture_left_.unlock(Texture::UnlockMode::Unlock_Update);

		texture_right_.lock();
		for (uint16_t y = 0; y < kMainDimension; ++y) {
			texture_right_.set_pixel(0, y, right_color(y));
		}
		texture_right_.unlock(Texture::UnlockMode::Unlock_Update);
	}

	inline void do_draw_cursor(RenderTarget& dst, int x, int y) {
		dst.blit(Vector2i(x, y), &selector_);
	}
	void draw_selection_cursors(RenderTarget& dst) {
		unsigned x = 0, y = 0, a = 0;
		switch (sidebar_attribute_) {
		case ColorAttribute::kRed:
			x = chooser_.get_color().g;
			y = chooser_.get_color().b;
			a = chooser_.get_color().r;
			break;
		case ColorAttribute::kGreen:
			x = chooser_.get_color().r;
			y = chooser_.get_color().b;
			a = chooser_.get_color().g;
			break;
		case ColorAttribute::kBlue:
			x = chooser_.get_color().r;
			y = chooser_.get_color().g;
			a = chooser_.get_color().b;
			break;
		}
		do_draw_cursor(dst, x - selector_.width() / 2, y - selector_.height() / 2);
		do_draw_cursor(dst, kMainDimension + kSpacing + (kSidebarWidth - selector_.width()) / 2,
		               a - selector_.height() / 2);
	}
	void draw_mouse_cursors(RenderTarget& dst) {
		const Vector2i mousepos = get_mouse_position();
		if (mousepos.y >= 0 && mousepos.y < kMainDimension && mousepos.x >= 0) {
			if (mousepos.x < kMainDimension) {
				do_draw_cursor(
				   dst, mousepos.x - selector_.width() / 2, mousepos.y - selector_.height() / 2);
			} else if (mousepos.x >= kMainDimension + kSpacing &&
			           mousepos.x < kMainDimension + kSpacing + kSidebarWidth) {
				do_draw_cursor(dst, kMainDimension + kSpacing + (kSidebarWidth - selector_.width()) / 2,
				               mousepos.y - selector_.height() / 2);
			}
		}
	}
	void draw_textures(RenderTarget& dst) {
		dst.blit(Vector2i(0, 0), &texture_left_);
		for (uint8_t i = 0; i < kSidebarWidth; ++i) {
			dst.blit(Vector2i(kMainDimension + kSpacing + i, 0), &texture_right_);
		}
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
     box_r_(&vbox_, panel_style_, 0, 0, Box::Horizontal),
     box_g_(&vbox_, panel_style_, 0, 0, Box::Horizontal),
     box_b_(&vbox_, panel_style_, 0, 0, Box::Horizontal),
     palette_box_1_(&vbox_, panel_style_, 0, 0, Box::Horizontal),
     palette_box_2_(&vbox_, panel_style_, 0, 0, Box::Horizontal),
     button_ok_(&buttonsbox_,
                "ok",
                0,
                0,
                kButtonSize,
                0,
                s == WindowStyle::kWui ? ButtonStyle::kWuiPrimary : ButtonStyle::kFsMenuPrimary,
                _("OK")),
     button_cancel_(
        &buttonsbox_,
        "cancel",
        0,
        0,
        kButtonSize,
        0,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        _("Cancel")),
     button_init_(
        &buttonsbox_,
        "initial_color",
        0,
        0,
        kButtonSize,
        0,
        s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
        _("Initial Color")),
     button_r_(&box_r_,
               "button_r",
               0,
               0,
               kButtonSize,
               kButtonSize,
               s == WindowStyle::kWui ? ButtonStyle::kWuiMenu : ButtonStyle::kFsMenuMenu,
               /** TRANSLATORS: First letter of the word "Red" */
               pgettext("color", "R")),
     button_g_(&box_g_,
               "button_g",
               0,
               0,
               kButtonSize,
               kButtonSize,
               s == WindowStyle::kWui ? ButtonStyle::kWuiMenu : ButtonStyle::kFsMenuMenu,
               /** TRANSLATORS: First letter of the word "Green" */
               pgettext("color", "G")),
     button_b_(&box_b_,
               "button_b",
               0,
               0,
               kButtonSize,
               kButtonSize,
               s == WindowStyle::kWui ? ButtonStyle::kWuiMenu : ButtonStyle::kFsMenuMenu,
               /** TRANSLATORS: First letter of the word "Blue" */
               pgettext("color", "B")),
     button_default_(default_color ?
                        new Button(&buttonsbox_,
                                   "default_color",
                                   0,
                                   0,
                                   kButtonSize,
                                   0,
                                   s == WindowStyle::kWui ? ButtonStyle::kWuiSecondary :
                                                            ButtonStyle::kFsMenuSecondary,
                                   _("Default Color")) :
                        nullptr),
     spin_r_(&box_r_,
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
     spin_g_(&box_g_,
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
     spin_b_(&box_b_,
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
     icon_(&vbox_, panel_style_, preview(init_color)) {
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

	button_r_.set_disable_style(ButtonDisableStyle::kPermpressed);
	button_g_.set_disable_style(ButtonDisableStyle::kPermpressed);
	button_b_.set_disable_style(ButtonDisableStyle::kPermpressed);
	button_r_.sigclicked.connect([this]() { set_sidebar_attribute(ColorAttribute::kRed); });
	button_g_.sigclicked.connect([this]() { set_sidebar_attribute(ColorAttribute::kGreen); });
	button_b_.sigclicked.connect([this]() { set_sidebar_attribute(ColorAttribute::kBlue); });
	spin_r_.changed.connect([this]() { set_color_from_spinners(); });
	spin_g_.changed.connect([this]() { set_color_from_spinners(); });
	spin_b_.changed.connect([this]() { set_color_from_spinners(); });

	for (unsigned i = 0; i < kMaxPlayers; ++i) {
		create_palette_button(i);
	}

	box_r_.add(&button_r_);
	box_r_.add_space(kSpacing);
	box_r_.add(&spin_r_, UI::Box::Resizing::kAlign, Align::kCenter);

	box_g_.add(&button_g_);
	box_g_.add_space(kSpacing);
	box_g_.add(&spin_g_, UI::Box::Resizing::kAlign, Align::kCenter);

	box_b_.add(&button_b_);
	box_b_.add_space(kSpacing);
	box_b_.add(&spin_b_, UI::Box::Resizing::kAlign, Align::kCenter);

	vbox_.add(&box_r_);
	vbox_.add_space(kSpacing);
	vbox_.add(&box_g_);
	vbox_.add_space(kSpacing);
	vbox_.add(&box_b_);
	vbox_.add_inf_space();
	vbox_.add(&icon_, UI::Box::Resizing::kAlign, Align::kCenter);
	vbox_.add_inf_space();
	vbox_.add(&palette_box_1_, UI::Box::Resizing::kAlign, Align::kCenter);
	vbox_.add_space(kSpacing);
	vbox_.add(&palette_box_2_, UI::Box::Resizing::kAlign, Align::kCenter);

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
	set_sidebar_attribute(ColorAttribute::kRed);

	set_center_panel(&main_box_);
	center_to_parent();
	initialization_complete();
}

void ColorChooser::create_palette_button(const unsigned index) {
	UI::Box* box = index < kMaxPlayers / 2 ? &palette_box_1_ : &palette_box_2_;

	palette_buttons_[index] = new Button(
	   box, "palette_" + std::to_string(index), 0, 0, kButtonSize, kButtonSize,
	   panel_style_ == PanelStyle::kWui ? ButtonStyle::kWuiMenu : ButtonStyle::kFsMenuMenu,
	   playercolor_image(kPlayerColors[index], "images/ui_basic/square.png"));
	palette_buttons_[index]->sigclicked.connect(
	   [this, index]() { set_color(kPlayerColors[index]); });

	if (index != 0 && index != kMaxPlayers / 2) {
		box->add_space(kSpacing);
	}
	box->add(palette_buttons_[index]);
}

void ColorChooser::set_color_from_spinners() {
	set_color(RGBColor(spin_r_.get_value(), spin_g_.get_value(), spin_b_.get_value()));
}
void ColorChooser::set_sidebar_attribute(const ColorAttribute a) {
	interactive_pane_.set_sidebar_attribute(a);
	button_r_.set_enabled(a != ColorAttribute::kRed);
	button_g_.set_enabled(a != ColorAttribute::kGreen);
	button_b_.set_enabled(a != ColorAttribute::kBlue);
}

void ColorChooser::set_color(const RGBColor& color) {
	current_ = color;
	spin_r_.set_value(color.r);
	spin_g_.set_value(color.g);
	spin_b_.set_value(color.b);
	icon_.set_frame(color);
	icon_.set_icon(preview(color));
}

bool ColorChooser::handle_key(const bool down, const SDL_Keysym code) {
	if (down && code.sym == SDLK_RETURN) {
		end_modal<Panel::Returncodes>(Panel::Returncodes::kOk);
		return true;
	}
	return Window::handle_key(down, code);
}

}  // namespace UI
