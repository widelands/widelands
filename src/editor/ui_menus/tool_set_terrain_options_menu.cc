/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "editor/ui_menus/tool_set_terrain_options_menu.h"

#include <memory>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/set_terrain_tool.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/terrain_description.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"

namespace {

UI::Checkbox* create_terrain_checkbox(UI::Panel* parent,
                                      LuaInterface* lua,
                                      const Widelands::TerrainDescription& terrain_descr,
                                      std::vector<std::unique_ptr<const Image>>* offscreen_images) {

	constexpr int kSmallPicSize = 20;

	std::vector<std::string> tooltips;

	// Blit the main terrain image
	const Image& terrain_texture = terrain_descr.get_texture(0);
	Texture* texture = new Texture(terrain_texture.width(), terrain_texture.height());
	texture->blit(
	   Rectf(0.f, 0.f, terrain_texture.width(), terrain_texture.height()), terrain_texture,
	   Rectf(0.f, 0.f, terrain_texture.width(), terrain_texture.height()), 1., BlendMode::UseAlpha);
	Vector2i pt(1, terrain_texture.height() - kSmallPicSize - 1);

	// Collect tooltips and blit small icons representing "is" values
	for (const Widelands::TerrainDescription::Type& terrain_type : terrain_descr.get_types()) {
		tooltips.push_back(terrain_type.descname);

		texture->blit(Rectf(pt.x, pt.y, terrain_type.icon->width(), terrain_type.icon->height()),
		              *terrain_type.icon,
		              Rectf(0.f, 0.f, terrain_type.icon->width(), terrain_type.icon->height()), 1.,
		              BlendMode::UseAlpha);
		pt.x += kSmallPicSize + 1;
	}

	// Get information about preferred trees
	std::unique_ptr<LuaTable> table(lua->run_script("scripting/editor/terrain_tooltip.lua"));
	std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
	cr->push_arg(terrain_descr.name());
	cr->resume();
	const std::string& treeinfo = cr->pop_table()->get_string("text");

	// Make sure we delete this later on.
	offscreen_images->emplace_back(texture);

	/** TRANSLATORS: %1% = terrain name, %2% = list of terrain types  */
	const std::string tooltip = (bformat(_("%1%: %2%"), terrain_descr.descname(),
	                                     i18n::localize_list(tooltips, i18n::ConcatenateWith::AND)))

	                               .append(treeinfo);

	std::unique_ptr<const Image>& image = offscreen_images->back();
	UI::Checkbox* cb =
	   new UI::Checkbox(parent, UI::PanelStyle::kWui, Vector2i::zero(), image.get(), tooltip);
	cb->set_desired_size(image->width() + 1, image->height() + 1);
	return cb;
}

}  // namespace

EditorToolSetTerrainOptionsMenu::EditorToolSetTerrainOptionsMenu(
   EditorInteractive& parent, EditorSetTerrainTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Terrain"), tool) {
	const Widelands::Descriptions& descriptions = parent.egbase().descriptions();
	LuaInterface* lua = &parent.egbase().lua();
	multi_select_menu_.reset(
	   new CategorizedItemSelectionMenu<Widelands::TerrainDescription, EditorSetTerrainTool>(
	      this, parent.editor_categories(Widelands::MapObjectType::TERRAIN), descriptions.terrains(),
	      [this, lua](UI::Panel* cb_parent, const Widelands::TerrainDescription& terrain_descr) {
		      return create_terrain_checkbox(cb_parent, lua, terrain_descr, &offscreen_images_);
	      },
	      [this] { select_correct_tool(); }, &tool));
	set_center_panel(multi_select_menu_.get());

	initialization_complete();
}
