/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MAIN_H
#define WL_UI_FSMENU_MAIN_H

#include <memory>

#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/menu_target.h"

/**
 * This runs the main menu. There, you can select
 * between different playmodes, exit and so on.
 */
class FullscreenMenuMain : public UI::Panel {
public:
	explicit FullscreenMenuMain(bool first_ever_init);

	const std::string& get_filename_for_continue() const {
		return filename_for_continue_;
	}

	// Internet login stuff
	void show_internet_login();
	void internet_login();
	std::string get_nickname() const {
		return nickname_;
	}
	std::string get_password() const {
		return password_;
	}
	bool registered() const {
		return register_;
	}

	void draw(RenderTarget&) override;
	void draw_overlay(RenderTarget&) override;
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;
	bool handle_key(bool, SDL_Keysym) override;

	// Set the labels for all buttons etc. This needs to be called after language switching.
	void set_labels();

	int16_t calc_desired_window_width(const std::string& window_name);
	int16_t calc_desired_window_height(const std::string& window_name);

private:
	void layout() override;

	Recti box_rect_;
	uint32_t butw_, buth_;
	uint32_t padding_;

	UI::Box vbox1_, vbox2_;

	UI::Dropdown<MenuTarget> singleplayer_;
	UI::Dropdown<MenuTarget> multiplayer_;
	UI::Button replay_;
	UI::Button editor_;
	UI::Button addons_;
	UI::Button options_;
	UI::Button about_;
	UI::Button exit_;
	UI::Textarea version_;
	UI::Textarea copyright_;

	std::string filename_for_continue_;

	const Image& splashscreen_;
	const Image& title_image_;

	uint32_t init_time_;

	std::vector<std::string> images_;
	uint32_t last_image_exchange_time_;
	size_t draw_image_, last_image_;
	Rectf image_pos(const Image&);
	Rectf title_pos();
	float calc_opacity(uint32_t time);

	bool visible_;
	void set_button_visibility(bool);

	// Values from internet login window
	std::string nickname_;
	std::string password_;
	bool auto_log_;
	bool register_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};

#endif  // end of include guard: WL_UI_FSMENU_MAIN_H
