/*
 * Copyright (C) 2022-2023 by the Widelands Development Team
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

#ifndef WL_WUI_PINNED_NOTE_H
#define WL_WUI_PINNED_NOTE_H

#include <memory>

#include "logic/game.h"
#include "logic/map_objects/pinned_note.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/textinput.h"
#include "ui_basic/unique_window.h"

///  Shows the current teams lineup and allows the player to perform diplomatic actions.
class PinnedNoteEditor : public UI::UniqueWindow {
public:
	PinnedNoteEditor(InteractivePlayer& parent,
	                 UI::UniqueWindow::Registry& r,
	                 Widelands::FCoords pos,
	                 const std::string& text,
	                 const RGBColor& rgb,
	                 bool is_new);

	bool handle_key(bool down, SDL_Keysym code) override;

private:
	InteractivePlayer& iplayer_;
	Widelands::FCoords pos_;
	RGBColor initial_color_, current_color_;
	bool delete_on_cancel_;

	void ok();
	void die() override;
	void send_delete();
	void update_color_preview();

	UI::Box box_, buttons_box_;
	UI::Button ok_, delete_, cancel_, color_;
	UI::EditBox* text_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NotePinnedNoteMoved>> subscriber_;
};

#endif  // end of include guard: WL_WUI_PINNED_NOTE_H
