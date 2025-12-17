/*
 * Copyright (C) 2019-2025 by the Widelands Development Team
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
 */

#ifndef WL_UI_GAME_MUSIC_PLAYER_H
#define WL_UI_GAME_MUSIC_PLAYER_H

#include "ui/basic/box.h"
#include "ui/basic/button.h"
#include "ui/basic/checkbox.h"
#include "ui/basic/panel.h"
#include "ui/basic/textarea.h"

class MusicTrackControl;

struct MusicPlayer : public UI::Box {
	explicit MusicPlayer(UI::Panel& parent);

	void think() override;

	void rebuild_music_track_controls();

private:
	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void set_shuffle(bool on);
	void update();

	std::vector<MusicTrackControl*> music_track_controls_;

	UI::Box vbox_track_playlist_;
	UI::Box hbox_playback_control_;
	UI::Button button_next_;
	UI::Checkbox checkbox_shuffle_;
	UI::Box hbox_current_track_;
	UI::Textarea label_current_track_;
};

#endif  // WL_UI_GAME_MUSIC_PLAYER_H
