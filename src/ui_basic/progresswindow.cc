/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#include "ui_basic/progresswindow.h"

#include <cstdlib>
#include <memory>
#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include <SDL_events.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "io/filesystem/layered_filesystem.h"
#include "wlapplication.h"

constexpr int16_t kProgressStatusRectPadding = 2;
constexpr int16_t kProgressStatusBorderX = 2;
constexpr int16_t kProgressStatusBorderY = 2;
constexpr int16_t kProgressStatusPositionY = 90; /* in percents, from top */

namespace UI {

std::vector<SDL_Event> ProgressWindow::event_buffer_ = {};

ProgressWindow::ProgressWindow(UI::Panel* parent,
                               const std::string& theme,
                               const std::string& background,
                               bool crop)
   : UI::Panel(parent,
               PanelStyle::kFsMenu /* unused */,
               0,
               0,
               parent != nullptr ? parent->get_inner_w() : g_gr->get_xres(),
               parent != nullptr ? parent->get_inner_h() : g_gr->get_yres()),
     label_center_(Vector2i::zero()),
     theme_(theme),
     crop_(crop) {
	// As long as this window exists and is visible, no tooltips will be drawn.
	set_hide_all_overlays();

	if (parent == nullptr) {
		graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
		   [this](const GraphicResolutionChanged& message) {
			   set_size(message.new_width, message.new_height);
		   });
	}

	event_buffer_.clear();
	initialization_complete();

	set_background(background);
	step(_("Loading…"));
}

ProgressWindow::~ProgressWindow() {
	for (IProgressVisualization* visualization : visualizations_) {
		visualization->stop();  //  inform visualizations
	}
	// Replay keypresses buffered in ui_key()
	for (SDL_Event event : event_buffer_) {
		SDL_PushEvent(&event);
	}
}

inline const UI::ProgressbarStyleInfo& ProgressWindow::progress_style() const {
	return g_style_manager->progressbar_style(UI::PanelStyle::kFsMenu);
}

void ProgressWindow::draw(RenderTarget& rt) {
	{  // Center and downscale background image
		const Image& bg = *g_image_cache->get(background_);
		const float w = bg.width();
		const float h = bg.height();
		rt.blitrect_scale(fit_image(w, h, get_w(), get_h(), crop_), &bg, Recti(0, 0, w, h), 1.f,
		                  BlendMode::UseAlpha);
	}

	// No float division to avoid Texture subsampling.
	label_center_.x = get_w() / 2;
	label_center_.y = get_h() * kProgressStatusPositionY / 100;

	const uint32_t h = text_height(progress_style().font());

	label_rectangle_.x = get_w() / 6;
	label_rectangle_.w = get_w() * 2 / 3;
	label_rectangle_.y = label_center_.y - h / 2 - kProgressStatusRectPadding;
	label_rectangle_.h = h + 2 * kProgressStatusRectPadding;

	Recti border_rect = label_rectangle_;
	border_rect.x -= kProgressStatusBorderX;
	border_rect.y -= kProgressStatusBorderY;
	border_rect.w += 2 * kProgressStatusBorderX;
	border_rect.h += 2 * kProgressStatusBorderY;

	rt.draw_rect(border_rect, progress_style().font().color());
	// TODO(GunChleoc): this should depend on actual progress. Add a total steps variable and reuse
	// the Progressbar class.
	rt.fill_rect(label_rectangle_, progress_style().medium_color());

	if (progress_message_) {
		progress_message_->draw(
		   rt, Vector2i(label_center_.x, label_center_.y - progress_message_->height() / 2),
		   UI::Align::kCenter);
	}

	for (IProgressVisualization* visualization : visualizations_) {
		visualization->update(rt, Recti(0, 0, get_inner_w(), get_inner_h()));  // game tips etc
	}
}

/// Set a picture to render in the background
void ProgressWindow::set_background(const std::string& file_name) {
	if (file_name.empty() || !g_fs->file_exists(file_name)) {
		std::string dir = template_dir() + "loadscreens/gameloading/";
		if (theme_.empty() && g_fs->is_directory(dir)) {
			// choose random theme
			const std::set<std::string> dirs = g_fs->list_directory(dir);
			auto it = dirs.begin();
			std::advance(it, RNG::static_rand(dirs.size()));
			dir = *it;
		} else if (g_fs->is_directory(dir + theme_)) {
			dir += theme_;
		} else {
			log_warn("Theme '%s' not found, using fallback image", theme_.c_str());
			background_ = "images/logos/wl-ico-128.png";
			return do_redraw_now();
		}

		const std::set<std::string> images = g_fs->list_directory(dir);
		if (images.empty()) {
			log_warn("No backgrounds found for theme '%s', using fallback image", theme_.c_str());
			background_ = "images/logos/wl-ico-128.png";
		} else {
			auto it = images.begin();
			std::advance(it, RNG::static_rand(images.size()));
			background_ = *it;
		}
	} else {
		background_ = file_name;
	}
	do_redraw_now();
}

/// Callback function: Buffer keypress events to be replayed after the loading is over.
bool ProgressWindow::ui_key(bool const down, SDL_Keysym const code) {
	// WLApplication can handle some keys immediately; don't buffer them.
	if (WLApplication::get()->handle_key(down, code.sym, code.mod)) {
		return true;
	}
	SDL_Event event;
	event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.keysym = code;
	event_buffer_.push_back(event);
	return true;
}

void ProgressWindow::step(const std::string& description) {
	// Handle events to respond to window resizing, to buffer keypresses,
	// and to prevent "not responding" on windows & "beach ball" on macOS.
	InputCallback input_callback = {nullptr, nullptr, nullptr, ui_key, nullptr, nullptr};
	WLApplication::get()->handle_input(&input_callback);

	progress_message_ =
	   UI::g_fh->render(as_richtext_paragraph(description, progress_style().font()));

	do_redraw_now();
}

/// Register additional visualization (tips/hints, animation, etc)
void ProgressWindow::add_visualization(IProgressVisualization* const instance) {
	// just add to collection
	visualizations_.push_back(instance);
}

void ProgressWindow::remove_visualization(IProgressVisualization* instance) {
	VisualizationArray& visualizations = visualizations_;

	for (VisualizationArray::iterator vis_iter = visualizations.begin();
	     vis_iter != visualizations.end(); ++vis_iter) {

		if (*vis_iter == instance) {
			visualizations_.erase(vis_iter);
			break;
		}
	}
}
}  // namespace UI
