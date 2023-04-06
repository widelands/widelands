/*
 * Copyright (C) 2007-2023 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_PROGRESSWINDOW_H
#define WL_UI_BASIC_PROGRESSWINDOW_H

#include <memory>

#include <SDL_events.h>

#include "base/rect.h"
#include "graphic/note_graphic_resolution_changed.h"
#include "graphic/styles/progress_bar_style.h"
#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"

namespace UI {

/// Manages a progress window on the screen.
struct IProgressVisualization {
	/// perform any visualizations as needed
	virtual void update(RenderTarget&, const Recti& bounds) = 0;

	/// Progress Window is closing, unregister and cleanup
	virtual void stop() = 0;

	virtual ~IProgressVisualization() = default;
};

/// Manages a progress window on the screen.
struct ProgressWindow : public UI::Panel {
	explicit ProgressWindow(UI::Panel*,
	                        const std::string& theme,
	                        const std::string& background,
	                        bool crop = true);
	~ProgressWindow() override;

	/// Register additional visualization (tips/hints, animation, etc)
	void add_visualization(IProgressVisualization* instance);
	void remove_visualization(IProgressVisualization* instance);

	/// Set a picture to render in the background
	void set_background(const std::string& file_name);

	/// Display a progress step description.
	void step(const std::string& description);

private:
	using VisualizationArray = std::vector<IProgressVisualization*>;

	bool try_set_background(const std::string& template_directory);

	Vector2i label_center_;
	Recti label_rectangle_;
	VisualizationArray visualizations_;
	std::string theme_;
	std::string background_;
	bool crop_;
	std::shared_ptr<const UI::RenderedText> progress_message_;

	const UI::ProgressbarStyleInfo& progress_style() const;

	static std::vector<SDL_Event> event_buffer_;
	static bool ui_key(bool down, SDL_Keysym code);

	void draw(RenderTarget&) override;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_PROGRESSWINDOW_H
