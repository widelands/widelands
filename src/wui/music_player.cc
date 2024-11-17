/*
 * Copyright (C) 2019-2024 by the Widelands Development Team
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

#include "wui/music_player.h"

#include <vector>

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "graphic/style_manager.h"
#include "sound/sound_handler.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/slider.h"
#include "wlapplication_options.h"

namespace {

    constexpr int kSpacing = 4;

/**
 * UI elements to set sound properties for 1 type of sounds.
 */
class MusicTrackControl : public UI::Box {
public:
    /**
     * @brief SoundControl Creates gui controls to show music tracks in a playlist
     * @param parent The parent panel
     * @param filename The music track filename, for toggling track playback on/off
     * @param title The title of the music track, to display
     */
    MusicTrackControl(UI::Box* parent,
                 const std::string& filename,
                 const std::string& title)
       : UI::Box(parent, UI::PanelStyle::kWui, filename, 0, 0, UI::Box::Horizontal),
         enable_(this, panel_style_, "enable", Vector2i::zero(), title),
         filename_(filename) {

        set_inner_spacing(kSpacing);
        add(&enable_, UI::Box::Resizing::kFullSize, UI::Align::kRight);
        if (SoundHandler::is_backend_disabled()) {
            enable_.set_enabled(false);
        } else {
            enable_.set_state(read_state());

            enable_.changedto.connect([this](bool on) { write_state(on); });
        }
        set_thinks(false);
    }

    int checkbox_width() {
        enable_.layout();
        return enable_.get_w();
    }

    void set_checkbox_width(const int w) {
        enable_.set_size(w, enable_.get_h());
    }

private:

    void write_state(bool on) {
        enable_.set_state(on);
        g_sh->set_music_track_enabled(filename_, on);
    }

    bool read_state() {
        return g_sh->is_music_track_enabled(filename_);
    }

    /// Enable / disable for the music track
    UI::Checkbox enable_;
    /// The filename for the music track
    std::string filename_;
};


}

MusicPlayer::MusicPlayer(UI::Panel& parent)
   : UI::Box(&parent, UI::PanelStyle::kWui, "music_player", 0, 0, UI::Box::Vertical),
    vbox_track_playlist_(this, UI::PanelStyle::kWui, "track_playlist", 0, 0, UI::Box::Vertical),
    hbox_playback_control_(this, UI::PanelStyle::kWui, "playback_control", 0, 0, UI::Box::Horizontal),
    button_playstop_(&hbox_playback_control_, "playstop", 0, 0, 100, 34, UI::ButtonStyle::kWuiSecondary, "Play/Stop"),
    button_next_(&hbox_playback_control_, "next", 0, 0, 80, 34, UI::ButtonStyle::kWuiSecondary, "Next"),
    checkbox_shuffle_(&hbox_playback_control_, UI::PanelStyle::kFsMenu, "shuffle", Vector2i::zero(), _("Shuffle")),
    label_current_track_(this, UI::PanelStyle::kWui, "current_track", UI::FontStyle::kWuiLabel, "Current track title", UI::Align::kLeft) {

    set_inner_spacing(kSpacing);
    set_desired_size(370, 150);
    vbox_track_playlist_.set_max_size(370, 100);
    vbox_track_playlist_.set_min_desired_breadth(370);
    vbox_track_playlist_.set_inner_spacing(2);
    vbox_track_playlist_.set_force_scrolling(true);

    std::vector<MusicTrackControl*> musicTrackControls;

    // todo loop through tracks
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_00.ogg", "Title of track 1"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_01.ogg", "Title of track 2"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_02.ogg", "Title of track 3"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_03.ogg", "Title of track 4"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_04.ogg", "Title of track 5"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_05.ogg", "Title of track 6"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_06.ogg", "Title of track 7"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_07.ogg", "Title of track 8"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_08.ogg", "Title of track 9"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_09.ogg", "Title of track 10"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_10.ogg", "Title of track 11"));
    musicTrackControls.emplace_back(new MusicTrackControl(&vbox_track_playlist_, "filename_11.ogg", "Title of track 12"));

    log_info("--- INITING MusicTrackControl's: %li", musicTrackControls.size());

    vbox_track_playlist_.add_space(2);
    for (MusicTrackControl* control : musicTrackControls) {
        vbox_track_playlist_.add(control);
    }
    vbox_track_playlist_.add_inf_space(); // aligns scrollbar to the right

    hbox_playback_control_.add(&button_playstop_);
    hbox_playback_control_.add_space(kSpacing);
    hbox_playback_control_.add(&button_next_);
    hbox_playback_control_.add_space(kSpacing);
    hbox_playback_control_.add(&checkbox_shuffle_);

    add(&vbox_track_playlist_);
    add(&hbox_playback_control_);
    add(&label_current_track_);
    // due to a layout bug, adding an empty label at the bottom to prevent clipping the label for current track title.
    UI::Textarea* label_spacer = new UI::Textarea(this, UI::PanelStyle::kWui, "spacer", UI::FontStyle::kWuiLabel, "", UI::Align::kLeft);
    add(label_spacer);
}

void MusicPlayer::draw(RenderTarget& dst) {
    if (get_w() == 0) {
        // The size hasn't been set yet
        return;
    }
    UI::PanelStyleInfo style = *g_style_manager->tabpanel_style(UI::TabPanelStyle::kWuiDark);
    draw_background(dst, style);

}
