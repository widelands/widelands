/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_SPLASH_OVERLAY_H
#define WL_UI_FSMENU_SPLASH_OVERLAY_H

#include <memory>

class RenderTarget;

namespace UI {
class RenderedText;
}  // namespace UI

namespace FsMenu {

class SplashOverlay {
public:
	explicit SplashOverlay() = default;

	// Draws the splash image and the credits text over it.
	// Loads the credits text on the first call.
	// Calculates the scrolling position of the credits on its own.
	// Returns false if credits_ cannot be drawn (loading failed).
	bool draw_main(RenderTarget& r);

	// Draws the splash image with opacity. Credits are not drawn.
	void draw_fade(RenderTarget& r, float opacity = 1.0f);

private:
	bool load_credits();
	int calculate_text_position(int window_h);

	uint32_t init_time_{0};  // 0 means not initialised
	std::shared_ptr<const UI::RenderedText> credits_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_SPLASH_OVERLAY_H
