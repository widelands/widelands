/*
 * Copyright (C) 2002-2004, 2006-2009, 2013 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_new_map.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/texture.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/progresswindow.h"

inline EditorInteractive& MainMenuNewMap::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

MainMenuNewMap::MainMenuNewMap(EditorInteractive & parent)
	:
	UI::Window(&parent, "new_map_menu", 0, 0, 360, 150, _("New Map")),
	margin_(4),
	box_width_(get_inner_w() -  2 * margin_),
	box_(this, margin_, margin_, UI::Box::Vertical, 0, 0, margin_),
	width_(&box_, 0, 0, box_width_, box_width_ / 3,
			 0, 0, 0,
			 _("Width:"), "", g_gr->images().get("pics/but1.png"), UI::SpinBox::Type::kValueList),
	height_(&box_, 0, 0, box_width_, box_width_ / 3,
			  0, 0, 0,
			  _("Height:"), "", g_gr->images().get("pics/but1.png"), UI::SpinBox::Type::kValueList),
	list_(&box_, 0, 0, box_width_, 330),
	// Buttons
	button_box_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, margin_),
	ok_button_(&button_box_, "create_map", 0, 0, box_width_ / 2 - margin_, 0,
		 g_gr->images().get("pics/but5.png"),
		 _("Create Map")),
	cancel_button_(&button_box_, "generate_map", 0, 0, box_width_ / 2 - margin_, 0,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"))
{
	width_.set_value_list(Widelands::kMapDimensions);
	height_.set_value_list(Widelands::kMapDimensions);

	{
		size_t width_index, height_index;
		Widelands::Extent const map_extent = parent.egbase().map().extent();
		for (width_index = 0;
			  width_index < Widelands::kMapDimensions.size() &&
			  Widelands::kMapDimensions[width_index] < map_extent.w;
			  ++width_index) {}
		width_.set_value(width_index);

		for (height_index = 0;
			  height_index < Widelands::kMapDimensions.size() &&
			  Widelands::kMapDimensions[height_index] < map_extent.h;
			  ++height_index) {}
		height_.set_value(height_index);
	}

	box_.add(&width_, UI::Box::AlignLeft);
	box_.add(&height_, UI::Box::AlignLeft);
	box_.add_space(margin_);
	UI::Textarea* terrain_label = new UI::Textarea(&box_, _("Terrain:"));
	box_.add(terrain_label, UI::Box::AlignLeft);
	box_.add(&list_, UI::Box::AlignLeft);
	box_.add_space(2 * margin_);

	cancel_button_.sigclicked.connect(boost::bind(&MainMenuNewMap::clicked_cancel, this));
	ok_button_.sigclicked.connect(boost::bind(&MainMenuNewMap::clicked_create_map, this));
	button_box_.add(&cancel_button_, UI::Box::AlignLeft);
	button_box_.add(&ok_button_, UI::Box::AlignLeft);
	box_.add(&button_box_, UI::Box::AlignLeft);

	box_.set_size(box_width_,
					  width_.get_h() + height_.get_h() + terrain_label->get_h() + list_.get_h()
					  + button_box_.get_h() + 9 * margin_);
	set_size(get_w(), box_.get_h() + 2 * margin_ + get_h() - get_inner_h());
	fill_list();
	center_to_parent();
}


void MainMenuNewMap::clicked_create_map() {
	EditorInteractive& parent = eia();
	Widelands::EditorGameBase & egbase = parent.egbase();
	Widelands::Map              & map    = egbase.map();
	UI::ProgressWindow loader;

	egbase.cleanup_for_load();

	map.create_empty_map(
				egbase.world(),
				width_.get_value() > 0 ? width_.get_value() : Widelands::kMapDimensions[0],
				height_.get_value() > 0 ? height_.get_value() : Widelands::kMapDimensions[0],
				list_.get_selected(),
				_("No Name"),
				g_options.pull_section("global").get_string("realname", pgettext("map_name", "Unknown")));

	egbase.postload     ();
	egbase.load_graphics(loader);

	map.recalc_whole_map(egbase.world());
	parent.map_changed();
	die();
}

void MainMenuNewMap::clicked_cancel() {
	die();
}

/*
 * fill the terrain list
 */
void MainMenuNewMap::fill_list() {
	list_.clear();
	const DescriptionMaintainer<Widelands::TerrainDescription>& terrains = eia().egbase().world().terrains();

	for (Widelands::DescriptionIndex index = 0; index < terrains.size(); ++index) {
		const Widelands::TerrainDescription& terrain = terrains.get(index);
		upcast(Image const, image, &terrain.get_texture(0));
		list_.add(terrain.descname(), index, image);
	}
	list_.select(0);
}
