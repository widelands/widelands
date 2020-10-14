/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "wui/toolbar.h"

#include "wui/info_panel.h"

MainToolbar::MainToolbar(InfoPanel& parent)
   : UI::Panel(&parent, 0, 0, parent.get_inner_w(), parent.get_inner_h()),
     box(this, 0, 0, UI::Box::Horizontal),
     on_top(false),
     draw_background(false),
     repeat_(0) {
     parent.set_toolbar(*this);
}

void MainToolbar::change_imageset(const ToolbarImageset& images) {
	imageset_ = images;
	finalize();
}

void MainToolbar::finalize() {
	// Set box size and get minimum height
	int box_width, height;
	box.get_desired_size(&box_width, &height);
	box.set_size(box_width, height);

	// Calculate repetition and width
	repeat_ = 1;
	int width = imageset_.left->width() + imageset_.center->width() + imageset_.right->width();
	while (width < box.get_w()) {
		++repeat_;
		width += imageset_.left->width() + imageset_.right->width();
	}
	width += imageset_.left_corner->width() + imageset_.right_corner->width();

	// Find the highest image
	height = std::max(height, imageset_.left_corner->height());
	height = std::max(height, imageset_.left->height());
	height = std::max(height, imageset_.center->height());
	height = std::max(height, imageset_.right->height());
	height = std::max(height, imageset_.right_corner->height());

	// Set size and position
	set_size(width, height);
	get_parent()->layout();

	// Notify dropdowns
	box.position_changed();
}

void MainToolbar::draw(RenderTarget& dst) {
	if (!draw_background) {
		return;
	}

	int x = 0;
	// Left corner
	dst.blit(Vector2i(x, on_top ? 0 : get_h() - imageset_.left_corner->height()), imageset_.left_corner);
	x += imageset_.left_corner->width();
	// Repeat left
	for (int i = 0; i < repeat_; ++i) {
		dst.blit(Vector2i(x, on_top ? 0 : get_h() - imageset_.left->height()), imageset_.left);
		x += imageset_.left->width();
	}
	// Center
	dst.blit(Vector2i(x, on_top ? 0 : get_h() - imageset_.center->height()), imageset_.center);
	x += imageset_.center->width();
	// Repeat right
	for (int i = 0; i < repeat_; ++i) {
		dst.blit(Vector2i(x, on_top ? 0 : get_h() - imageset_.right->height()), imageset_.right);
		x += imageset_.right->width();
	}
	// Right corner
	dst.blit(Vector2i(x, on_top ? 0 : get_h() - imageset_.right_corner->height()), imageset_.right_corner);
}
