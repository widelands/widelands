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
#include "logic/world/editor_category.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "ui_basic/tabpanel.h"

namespace {

using namespace Widelands;

static const int32_t check[] = {
	TerrainDescription::Type::kArable,                                        // Arable implies walkable
	TerrainDescription::Type::kWalkable,
	TerrainDescription::Type::kMineable,                                      // Mountain implies walkable"
	TerrainDescription::Type::kImpassable,
	TerrainDescription::Type::kDead | TerrainDescription::Type::kImpassable,  // Dead is impassable
	TerrainDescription::Type::kImpassable | TerrainDescription::Type::kWater, // Water is impassable
	-1,  // end marker
};

UI::Checkbox* create_terrain_checkbox(UI::Panel* parent,
                                      const TerrainDescription& terrain_descr,
                                      std::vector<std::unique_ptr<const Image>>* offscreen_images) {
	const Image* arable = g_gr->images().get("pics/terrain_arable.png");
	const Image* water = g_gr->images().get("pics/terrain_water.png");
	const Image* mineable = g_gr->images().get("pics/terrain_mineable.png");
	const Image* dead = g_gr->images().get("pics/terrain_dead.png");
	const Image* impassable = g_gr->images().get("pics/terrain_impassable.png");
	const Image* walkable = g_gr->images().get("pics/terrain_walkable.png");

	constexpr int kSmallPicHeight = 20;
	constexpr int kSmallPicWidth = 20;

	std::vector<std::string> tooltips;

	for (size_t checkfor = 0; check[checkfor] >= 0; ++checkfor) {
		const TerrainDescription::Type ter_is = terrain_descr.get_is();
		if (ter_is != check[checkfor])
			continue;

		const Texture& terrain_texture = terrain_descr.get_texture(0);
		Texture* texture = new Texture(terrain_texture.width(), terrain_texture.height());
		blit(Rect(0, 0, terrain_texture.width(), terrain_texture.height()),
		              terrain_texture,
		              Rect(0, 0, terrain_texture.width(), terrain_texture.height()),
		              1.,
		              BlendMode::UseAlpha, texture);
		Point pt(1, terrain_texture.height() - kSmallPicHeight - 1);

		if (ter_is == TerrainDescription::Type::kArable) {
			blit(Rect(pt.x, pt.y, arable->width(), arable->height()),
				 *arable,
				 Rect(0, 0, arable->width(), arable->height()),
			     1.,
			     BlendMode::UseAlpha,
			     texture);
			pt.x += kSmallPicWidth + 1;
			/** TRANSLATORS: This is a terrain type tooltip in the editor */
			tooltips.push_back(_("arable"));
		} else {
			if (ter_is & TerrainDescription::Type::kWater) {
				blit(Rect(pt.x, pt.y, water->width(), water->height()),
				     *water,
				     Rect(0, 0, water->width(), water->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				pt.x += kSmallPicWidth + 1;
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				tooltips.push_back(_("navigable"));
			}
			else if (ter_is & TerrainDescription::Type::kMineable) {
				blit(Rect(pt.x, pt.y, mineable->width(), mineable->height()),
					 *mineable,
					 Rect(0, 0, mineable->width(), mineable->height()),
					 1.,
					 BlendMode::UseAlpha,
					 texture);
				pt.x += kSmallPicWidth + 1;
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				tooltips.push_back(_("mineable"));
			}
			if (ter_is & TerrainDescription::Type::kDead) {
				blit(Rect(pt.x, pt.y, dead->width(), dead->height()),
				     *dead,
				     Rect(0, 0, dead->width(), dead->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				pt.x += kSmallPicWidth + 1;
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				tooltips.push_back(_("irreclaimable"));
			}
			if (ter_is & TerrainDescription::Type::kImpassable) {
				blit(Rect(pt.x, pt.y, impassable->width(), impassable->height()),
				     *impassable,
				     Rect(0, 0, impassable->width(), impassable->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				pt.x += kSmallPicWidth + 1;
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				tooltips.push_back(_("impassable"));
			}
			if (ter_is & TerrainDescription::Type::kWalkable) {
				blit(Rect(pt.x, pt.y, walkable->width(), walkable->height()),
					 *walkable,
					 Rect(0, 0, walkable->width(), walkable->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				 tooltips.push_back(_("walkable"));
			}
		}

		// Make sure we delete this later on.
		offscreen_images->emplace_back(texture);
		break;
	}
	/** TRANSLATORS: %1% = terrain name, %2% = list of terrain types  */
	const std::string tooltip = ((boost::format("%1%: %2%"))
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
