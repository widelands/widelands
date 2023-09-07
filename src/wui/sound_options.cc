/*
 * Copyright (C) 2019-2023 by the Widelands Development Team
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

#include "wui/sound_options.h"

#include <vector>

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "graphic/text_layout.h"
#include "sound/sound_handler.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/slider.h"

namespace {

constexpr int kPadding = 4;
constexpr int kSliderWidth = 200;
constexpr int kSliderHeight = 16;
constexpr int kCursorWidth = 28;
constexpr int kSpacing = 32;

UI::PanelStyle slider_to_panel_style(UI::SliderStyle style) {
	return style == UI::SliderStyle::kFsMenu ? UI::PanelStyle::kFsMenu : UI::PanelStyle::kWui;
}

/**
 * UI elements to set sound properties for 1 type of sounds.
 */
class SoundControl : public UI::Box {
public:
	/**
	 * @brief SoundControl Creates a new sound control box
	 * @param parent The parent panel
	 * @param title The localized test label to display
	 * @param type The type of sound to set the properties for
	 * @param style The User interface style for the slider
	 */
	SoundControl(UI::Box* parent,
	             UI::SliderStyle style,
	             const std::string& name,
	             const std::string& title,
	             SoundType type,
	             FxId representative_fx = kNoSoundEffect)
	   : UI::Box(parent, slider_to_panel_style(style), name, 0, 0, UI::Box::Horizontal),
	     volume_(this,
	             "volume",
	             0,
	             0,
	             kSliderWidth,
	             kSliderHeight,
	             0,
	             g_sh->get_max_volume(),
	             g_sh->get_volume(type),
	             style,
	             /** TRANSLATORS: Tooltip for volume slider in sound options */
	             _("Changes the volume. Click to hear a sample."),
	             kCursorWidth),
	     enable_(this, panel_style_, "enable", Vector2i::zero(), title),
	     type_(type),
	     fx_(representative_fx) {
		set_inner_spacing(kPadding);
		if (UI::g_fh->fontset()->is_rtl()) {
			add(&volume_, UI::Box::Resizing::kAlign, UI::Align::kRight);
			add(&enable_, UI::Box::Resizing::kAlign, UI::Align::kRight);
		} else {
			add(&enable_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
			add(&volume_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
		}

		if (SoundHandler::is_backend_disabled()) {
			enable_.set_enabled(false);
			volume_.set_visible(false);
		} else {
			enable_.set_state(g_sh->is_sound_enabled(type));
			volume_.set_visible(g_sh->is_sound_enabled(type));

			enable_.changedto.connect([this](bool on) { enable_changed(on); });
			volume_.changedto.connect([this](int32_t value) { volume_changed(value); });
			volume_.clicked.connect([this] { play_sound_sample(); });
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
	/// Plays a system sound sample selected from the given sound type
	void play_sound_sample() {
		if (fx_ != kNoSoundEffect) {
			g_sh->play_fx(type_, fx_);
		}
	}

	/// Sets new enable/disable value in the sound handler for the sound type and enables/disables
	/// the volume slider accordingly
	void enable_changed(bool on) {
		enable_.set_state(on);
		volume_.set_visible(on);
		g_sh->set_enable_sound(type_, on);
	}

	/// Sets the volume in the sound handler to the new 'value'
	void volume_changed(int32_t value) {
		g_sh->set_volume(type_, value);
	}

	/// Control the volume for the sound type
	UI::HorizontalSlider volume_;
	/// Enable / disable sound type
	UI::Checkbox enable_;
	/// The sound type to control
	const SoundType type_;
	/// Representative sound effect to play
	const FxId fx_;
};

}  // namespace

SoundOptions::SoundOptions(UI::Panel& parent, UI::SliderStyle style)
   : UI::Box(&parent, slider_to_panel_style(style), "sound_options", 0, 0, UI::Box::Vertical),
     custom_songset_(
        this,
        panel_style_,
        "custom_songset",
        {0, 0},
        _("Play your own music in-game"),
        richtext_escape(
           _("You can play custom in-game music by placing your own music files in "
             "‘<Widelands Home Directory>/music/custom_XX.*’ (where ‘XX’ are sequential "
             "two-digit numbers starting with 00). Supported file formats are ‘.mp3’ and ‘.ogg’.")),
        0) {

	set_inner_spacing(kSpacing);

	std::vector<SoundControl*> controls;
	controls.emplace_back(new SoundControl(
	   this, style, "music", pgettext("sound_options", "Music"), SoundType::kMusic));

	controls.emplace_back(new SoundControl(
	   this, style, "chat", pgettext("sound_options", "Chat Messages"), SoundType::kChat,
	   SoundHandler::register_fx(SoundType::kChat, "sound/lobby_chat")));

	controls.emplace_back(new SoundControl(
	   this, style, "messages", pgettext("sound_options", "Game Messages"), SoundType::kMessage,
	   SoundHandler::register_fx(SoundType::kMessage, "sound/message")));

	controls.emplace_back(new SoundControl(
	   this, style, "ui", pgettext("sound_options", "User Interface"), SoundType::kUI));

	controls.emplace_back(new SoundControl(
	   this, style, "ambient", pgettext("sound_options", "Ambient Sounds"), SoundType::kAmbient,
	   SoundHandler::register_fx(SoundType::kAmbient, "sound/create_construction_site")));

	int max_w = 0;
	for (SoundControl* control : controls) {
		add(control);
		if (const int w = control->checkbox_width(); w > max_w) {
			max_w = w;
		}
	}
	for (SoundControl* control : controls) {
		control->set_checkbox_width(max_w);
	}

	add(&custom_songset_);
	custom_songset_.set_state(g_sh->use_custom_songset());
	custom_songset_.changedto.connect([](bool state) { g_sh->use_custom_songset(state); });

	// TODO(GunChleoc): There's a bug (probably somewhere in Box, triggered in combination with
	// Window::set_center_panel) that will hide the bottom SoundControl in GameOptionsSoundMenu if
	// the MultilineTextarea is not added to the Box. So, we create and add it even if its text is
	// empty.
	UI::MultilineTextarea* sound_warning =
	   new UI::MultilineTextarea(this, "warning", 0, 0, 100, 0, slider_to_panel_style(style), "",
	                             UI::Align::kLeft, UI::MultilineTextarea::ScrollMode::kNoScrolling);
	add(sound_warning, UI::Box::Resizing::kExpandBoth);

	if (SoundHandler::is_backend_disabled()) {
		sound_warning->set_text(_("Sound is disabled either due to a problem with the sound driver, "
		                          "or because it was switched off at the command line."));
	}
}
