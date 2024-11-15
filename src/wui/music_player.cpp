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
#include "sound/sound_handler.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/slider.h"
#include "wlapplication_options.h"

namespace {

    constexpr int kPadding = 4;
    constexpr int kSliderWidth = 200;
    constexpr int kSliderHeight = 16;
    constexpr int kCursorWidth = 28;
    constexpr int kSpacing = 4;

/**
 * UI elements to set sound properties for 1 type of sounds.
 */
class MusicTrackControl : public UI::Box {
public:
    /**
     * @brief SoundControl Creates gui controls to show music tracks in a playlist
     * @param parent The parent panel
     * @param title The localized test label to display
     * @param type The type of sound to set the properties for
     */
    MusicTrackControl(UI::Box* parent,
                 const std::string& filename,
                 const std::string& title)
       : UI::Box(parent, UI::PanelStyle::kWui, filename, 0, 0, UI::Box::Horizontal),
         enable_(this, panel_style_, "enable", Vector2i::zero(), title),
         filename_(filename) {

        set_inner_spacing(kPadding);
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
   : UI::Box(&parent, UI::PanelStyle::kWui, "music_player", 0, 0, UI::Box::Vertical) {

    set_inner_spacing(kSpacing);

    std::vector<MusicTrackControl*> controls;

    // todo loop through tracks
    controls.emplace_back(new MusicTrackControl(this, "filename_00.ogg", "Title of track 1"));
    controls.emplace_back(new MusicTrackControl(this, "filename_01.ogg", "Title of track 2"));

    log_info("--- INITING MusicTrackControl's: %li", controls.size());

    int max_w = 0;
    for (MusicTrackControl* control : controls) {
        add(control);
        int w = control->checkbox_width();
        if (w > max_w) {
            max_w = w;
        }
    }
    for (MusicTrackControl* control : controls) {
        control->set_checkbox_width(max_w);
    }
}
