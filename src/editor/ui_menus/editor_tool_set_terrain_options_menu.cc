/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_tool_set_terrain_options_menu.h"

#include <memory>

#include <SDL_keycode.h>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/editor_set_terrain_tool.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"
#include "logic/map.h"
#include "logic/map_objects/world/editor_category.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"

namespace {

using namespace Widelands;

UI::Checkbox* create_terrain_checkbox(UI::Panel* parent,
                                      const TerrainDescription& terrain_descr,
                                      std::vector<std::unique_ptr<const Image>>* offscreen_images) {

	constexpr int kSmallPicSize = 20;

	std::vector<std::string> tooltips;

	// Blit the main terrain image
	const Image& terrain_texture = terrain_descr.get_texture(0);
	Texture* texture = new Texture(terrain_texture.width(), terrain_texture.height());
	texture->blit(Rect(0, 0, terrain_texture.width(), terrain_texture.height()),
					  terrain_texture,
					  Rect(0, 0, terrain_texture.width(), terrain_texture.height()),
					  1.,
					  BlendMode::UseAlpha);
	Point pt(1, terrain_texture.height() - kSmallPicSize - 1);

	// Collect tooltips and blit small icons representing "is" values
	for (const TerrainDescription::Type& terrain_type : terrain_descr.get_types()) {
                tooltips.insert(tooltips.end(),
								terrain_descr.custom_tooltips().begin(),
								terrain_descr.custom_tooltips().end());
		tooltips.push_back(terrain_type.descname);

		texture->blit(Rect(pt.x, pt.y, terrain_type.icon->width(), terrain_type.icon->height()),
			 *terrain_type.icon,
			 Rect(0, 0, terrain_type.icon->width(), terrain_type.icon->height()),
			  1.,
			  BlendMode::UseAlpha);
		pt.x += kSmallPicSize + 1;
	}
	// Make sure we delete this later on.
	offscreen_images->emplace_back(texture);

	/** TRANSLATORS: %1% = terrain name, %2% = list of terrain types  */
	const std::string tooltip = ((boost::format(_("%1%: %2%")))
								  % terrain_descr.descname()
								  % i18n::localize_list(tooltips, i18n::ConcatenateWith::AND)).str();

	std::unique_ptr<const Image>& image = offscreen_images->back();
	UI::Checkbox* cb = new UI::Checkbox(parent, Point(0, 0), image.get(), tooltip);
	cb->set_desired_size(image->width() + 1, image->height() + 1);
	return cb;
}

}  // namespace

EditorToolSetTerrainOptionsMenu::EditorToolSetTerrainOptionsMenu(
   EditorInteractive& parent, EditorSetTerrainTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Terrain Select")) {
	const Widelands::World& world = parent.egbase().world();
	multi_select_menu_.reset(
	   new CategorizedItemSelectionMenu<Widelands::TerrainDescription, EditorSetTerrainTool>(
	      this,
	      world.editor_terrain_categories(),
	      world.terrains(),
	      [this](UI::Panel* cb_parent, const TerrainDescription& terrain_descr) {
		      return create_terrain_checkbox(cb_parent, terrain_descr, &offscreen_images_);
		   },
	      [this] {select_correct_tool();},
	      &tool));
	set_center_panel(multi_select_menu_.get());
}

EditorToolSetTerrainOptionsMenu::~EditorToolSetTerrainOptionsMenu() {
}
