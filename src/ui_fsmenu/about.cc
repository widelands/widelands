/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include "base/i18n.h"

FullscreenMenuAbout::FullscreenMenuAbout(FullscreenMenuMain& fsmm)
   : UI::Window(&fsmm,
                "about",
                fsmm.get_w() / 4,
                fsmm.get_h() / 4,
                fsmm.get_w() / 2,
                fsmm.get_h() / 2,
                _("About Widelands")),
     parent_(fsmm),
     close_(this, "close", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Close")),
     tabs_(this, UI::PanelStyle::kFsMenu, UI::TabPanelStyle::kFsMenu) {
	tabs_.add_tab("txts/README.lua");
	tabs_.add_tab("txts/LICENSE.lua");
	tabs_.add_tab("txts/AUTHORS.lua");
	tabs_.add_tab("txts/TRANSLATORS.lua");

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged&) { layout(); });

	close_.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	});

	layout();
}

bool FullscreenMenuAbout::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

constexpr int16_t kPadding = 4;
void FullscreenMenuAbout::layout() {
	if (!is_minimal()) {
		set_size(parent_.get_w() / 2, parent_.get_h() / 2);

		close_.set_size(get_inner_w() / 2, get_h() / 20);
		close_.set_pos(Vector2i(get_inner_w() / 4, get_inner_h() - kPadding - close_.get_h()));

		tabs_.set_size(get_inner_w(), get_inner_h() - close_.get_h() - 2 * kPadding);
	}

	UI::Window::layout();
}
