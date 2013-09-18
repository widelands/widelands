/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef UI_PROGRESSWINDOW_H
#define UI_PROGRESSWINDOW_H

#include <cstring>
#include <string>
#include <vector>

#include "point.h"
#include "rect.h"

class Image;
class RenderTarget;

namespace UI {

/// Manages a progress window on the screen.
struct IProgressVisualization {
	///perform any visualizations as needed
	///if repaint is true, ensure previously painted areas are visible
	virtual void update(bool repaint) = 0;

	///Progress Window is closing, unregister and cleanup
	virtual void stop() = 0;

	virtual ~IProgressVisualization() {}
};

/// Manages a progress window on the screen.
struct ProgressWindow {
	ProgressWindow(const std::string & background = std::string());
	~ProgressWindow();

	/// Register additional visualization (tips/hints, animation, etc)
	void add_visualization(IProgressVisualization * instance);
	void remove_visualization(IProgressVisualization * instance);

	/// Set a picture to render in the background
	void set_background(const std::string & file_name);

	/// Display a progress step description.
	void step(const std::string & description);
	void stepf(char const * format, ...);

private:
	typedef std::vector<IProgressVisualization *> VisualizationArray;
	uint32_t  m_xres;
	uint32_t  m_yres;
	Point m_label_center;
	Rect  m_label_rectangle;
	VisualizationArray m_visualizations;
	std::string m_background;
	const Image* m_background_pic;

	void draw_background(RenderTarget & rt, uint32_t xres, uint32_t yres);
	void update(bool repaint);
};

}

#endif
