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
	TerrainDescription::Type::kGreen,                                 //  "green"
	TerrainDescription::Type::kDry,                                   //  "dry"
	TerrainDescription::Type::kDry | TerrainDescription::Type::kMountain,    //  "mountain"
	TerrainDescription::Type::kDry | TerrainDescription::Type::kImpassable,  //  "impassable"
	TerrainDescription::Type::kDead | TerrainDescription::Type::kDry |
		TerrainDescription::Type::kImpassable,  //  "dead"
	TerrainDescription::Type::kImpassable | TerrainDescription::Type::kDry | TerrainDescription::Type::kWater,
	-1,  // end marker
};

UI::Checkbox* create_terrain_checkbox(UI::Panel* parent,
                                      const TerrainDescription& terrain_descr,
                                      std::vector<std::unique_ptr<const Image>>* offscreen_images) {
	const Image* green = g_gr->images().get("pics/terrain_green.png");
	const Image* water = g_gr->images().get("pics/terrain_water.png");
	const Image* mountain = g_gr->images().get("pics/terrain_mountain.png");
	const Image* dead = g_gr->images().get("pics/terrain_dead.png");
	const Image* impassable = g_gr->images().get("pics/terrain_impassable.png");
	const Image* dry = g_gr->images().get("pics/terrain_dry.png");

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

		if (ter_is == TerrainDescription::Type::kGreen) {
			blit(Rect(pt.x, pt.y, green->width(), green->height()),
			     *green,
			     Rect(0, 0, green->width(), green->height()),
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
				tooltips.push_back(_("aquatic"));
			}
			else if (ter_is & TerrainDescription::Type::kMountain) {
				blit(Rect(pt.x, pt.y, mountain->width(), mountain->height()),
				     *mountain,
				     Rect(0, 0, mountain->width(), mountain->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				pt.x += kSmallPicWidth + 1;
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				tooltips.push_back(_("mountainous"));
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
				tooltips.push_back(_("dead"));
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
			if (ter_is & TerrainDescription::Type::kDry) {
				blit(Rect(pt.x, pt.y, dry->width(), dry->height()),
				     *dry,
				     Rect(0, 0, dry->width(), dry->height()),
				     1.,
				     BlendMode::UseAlpha,
				     texture);
				/** TRANSLATORS: This is a terrain type tooltip in the editor */
				 tooltips.push_back(_("treeless"));
			}
		}

		// Make sure we delete this later on.
		offscreen_images->emplace_back(texture);
		break;
	}

	tooltips.insert(tooltips.end(),
						 terrain_descr.custom_tooltips().begin(),
						 terrain_descr.custom_tooltips().end());

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
