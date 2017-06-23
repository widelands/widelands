/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_PROGRESSWINDOW_H
#define WL_UI_BASIC_PROGRESSWINDOW_H

#include <cstring>
#include <string>
#include <vector>

#include "base/rect.h"
#include "ui_basic/fullscreen_window.h"

class Image;
class RenderTarget;

namespace UI {

/// Manages a progress window on the screen.
struct IProgressVisualization {
	/// perform any visualizations as needed
	/// if repaint is true, ensure previously painted areas are visible
	virtual void update(bool repaint) = 0;

	/// Progress Window is closing, unregister and cleanup
	virtual void stop() = 0;

	virtual ~IProgressVisualization() {
	}
};

/// Manages a progress window on the screen.
struct ProgressWindow : public UI::FullscreenWindow {
	ProgressWindow(const std::string& background = std::string());
	~ProgressWindow();

	/// Register additional visualization (tips/hints, animation, etc)
	void add_visualization(IProgressVisualization* instance);
	void remove_visualization(IProgressVisualization* instance);

	/// Set a picture to render in the background
	void set_background(const std::string& file_name);

	/// Display a progress step description.
	void step(const std::string& description);

private:
	using VisualizationArray = std::vector<IProgressVisualization*>;

	Vector2i label_center_;
	Recti label_rectangle_;
	VisualizationArray visualizations_;
	std::string background_;

	void draw(RenderTarget&) override;
	void update(bool repaint);
};
}

#endif  // end of include guard: WL_UI_BASIC_PROGRESSWINDOW_H
