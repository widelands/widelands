/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "graphic/minimap_renderer.h"

#include <memory>

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

namespace {

const RGBColor kWhite(255, 255, 255);

// Blend two colors.
inline RGBColor blend_color(const RGBColor& c1, const RGBColor& c2) {
	return RGBColor((c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2);
}

// Returns the color to be used in the minimap for the given field.
inline RGBColor calc_minimap_color(const Widelands::EditorGameBase& egbase,
                                   const Widelands::FCoords& f,
                                   MiniMapLayer layers,
                                   Widelands::PlayerNumber owner,
                                   bool see_details) {
	RGBColor color;
	if (layers & MiniMapLayer::Terrain) {
		color = egbase.world()
		           .terrain_descr(f.field->terrain_d())
		           .get_minimap_color(f.field->get_brightness());
	}

	if (layers & MiniMapLayer::Owner) {
		if (0 < owner) {
			color = blend_color(color, egbase.player(owner).get_playercolor());
		}
	}

	if (see_details) {
		// if ownership layer is displayed, it creates enough contrast to
		// visualize objects using white color.

		if (upcast(PlayerImmovable const, immovable, f.field->get_immovable())) {
			if ((layers & MiniMapLayer::Road) && dynamic_cast<Road const*>(immovable)) {
				color = blend_color(color, kWhite);
			}

			if (((layers & MiniMapLayer::Flag) && dynamic_cast<Flag const*>(immovable)) ||
			    ((layers & MiniMapLayer::Building) &&
			     dynamic_cast<Widelands::Building const*>(immovable))) {
				color = kWhite;
			}
		}
	}

	return color;
}

// Draws the dotted frame border onto the minimap.
bool is_minimap_frameborder(const Widelands::FCoords& f,
                            const Point& ptopleft,
                            const Point& pbottomright,
                            int32_t mapwidth,
                            int32_t mapheight,
                            int32_t modx,
                            int32_t mody) {
	bool isframepixel = false;

	if (ptopleft.x <= pbottomright.x) {
		if (f.x >= ptopleft.x && f.x <= pbottomright.x &&
		    (f.y == ptopleft.y || f.y == pbottomright.y) && f.x % 2 == modx)
			isframepixel = true;
	} else {
		if (((f.x >= ptopleft.x && f.x <= mapwidth) || (f.x >= 0 && f.x <= pbottomright.x)) &&
		    (f.y == ptopleft.y || f.y == pbottomright.y) && (f.x % 2) == modx)
			isframepixel = true;
	}

	if (ptopleft.y <= pbottomright.y) {
		if (f.y >= ptopleft.y && f.y <= pbottomright.y &&
		    (f.x == ptopleft.x || f.x == pbottomright.x) && f.y % 2 == mody)
			isframepixel = true;
	} else {
		if (((f.y >= ptopleft.y && f.y <= mapheight) || (f.y >= 0 && f.y <= pbottomright.y)) &&
		    (f.x == ptopleft.x || f.x == pbottomright.x) && f.y % 2 == mody)
			isframepixel = true;
	}

	return isframepixel;
}

// Does the actual work of drawing the minimap.
void draw_minimap_int(Texture* texture,
                      const Widelands::EditorGameBase& egbase,
                      const Widelands::Player* player,
                      const Point& viewpoint,
                      MiniMapLayer layers) {
	const Widelands::Map& map = egbase.map();

	const uint16_t surface_h = texture->height();
	const uint16_t surface_w = texture->width();

	// size of the display frame
	int32_t xsize = g_gr->get_xres() / kTriangleWidth / 2;
	int32_t ysize = g_gr->get_yres() / kTriangleHeight / 2;

	const int32_t mapwidth = egbase.get_map().get_width();
	const int32_t mapheight = map.get_height();

	Point ptopleft;  // top left point of the current display frame
	ptopleft.x = viewpoint.x + mapwidth / 2 - xsize;
	if (ptopleft.x < 0) {
		ptopleft.x += mapwidth;
	}
	ptopleft.y = viewpoint.y + mapheight / 2 - ysize;
	if (ptopleft.y < 0) {
		ptopleft.y += mapheight;
	}

	Point pbottomright;  // bottom right point of the current display frame
	pbottomright.x = viewpoint.x + mapwidth / 2 + xsize;
	if (pbottomright.x >= mapwidth) {
		pbottomright.x -= mapwidth;
	}
	pbottomright.y = viewpoint.y + mapheight / 2 + ysize;
	if (pbottomright.y >= mapheight) {
		pbottomright.y -= mapheight;
	}

	uint32_t modx = pbottomright.x % 2;
	uint32_t mody = pbottomright.y % 2;

	for (uint32_t y = 0; y < surface_h; ++y) {
		Widelands::FCoords f(
		   Widelands::Coords(viewpoint.x, viewpoint.y + (layers & MiniMapLayer::Zoom2 ? y / 2 : y)));
		map.normalize_coords(f);
		f.field = &map[f];
		Widelands::MapIndex i = Widelands::Map::get_index(f, mapwidth);
		for (uint32_t x = 0; x < surface_w; ++x) {
			if (x % 2 || !(layers & MiniMapLayer::Zoom2)) {
				move_r(mapwidth, f, i);
			}

			RGBColor pixel_color;
			if ((layers & MiniMapLayer::ViewWindow) &&
			    is_minimap_frameborder(f, ptopleft, pbottomright, mapwidth, mapheight, modx, mody)) {
				pixel_color = RGBColor(255, 0, 0);
			} else {
				uint16_t vision =
				   0;  // See Player::Field::Vision: 1 if seen once, > 1 if seen right now.
				Widelands::PlayerNumber owner = 0;
				if (player == nullptr || player->see_all()) {
					vision = 2;  // Seen right now.
					owner = f.field->get_owned_by();
				} else if (player != nullptr) {
					const auto& field = player->fields()[i];
					vision = field.vision;
					owner = field.owner;
				}

				if (vision > 0) {
					pixel_color = calc_minimap_color(egbase, f, layers, owner, vision > 1);
				}
			}

			texture->set_pixel(x, y, pixel_color);
		}
	}
}

}  // namespace

std::unique_ptr<Texture> draw_minimap(const EditorGameBase& egbase,
                                      const Player* player,
                                      const Point& viewpoint,
                                      MiniMapLayer layers) {
	// TODO(unknown): Currently the minimap is redrawn every frame. That is not really
	//       necesary. The created texture could be cached and only redrawn two
	//       or three times per second
	const Map& map = egbase.map();
	const int16_t map_w = (layers & MiniMapLayer::Zoom2) ? map.get_width() * 2 : map.get_width();
	const int16_t map_h = (layers & MiniMapLayer::Zoom2) ? map.get_height() * 2 : map.get_height();

	std::unique_ptr<Texture> texture(new Texture(map_w, map_h));

	texture->lock(Texture::Lock_Discard);
	draw_minimap_int(texture.get(), egbase, player, viewpoint, layers);
	texture->unlock(Texture::Unlock_Update);

	return texture;
}

void write_minimap_image(const EditorGameBase& egbase,
                         const Player* player,
                         const Point& gviewpoint,
                         MiniMapLayer layers,
                         ::StreamWrite* const streamwrite) {
	assert(streamwrite != nullptr);

	Point viewpoint(gviewpoint);

	// map dimension
	const int16_t map_w = egbase.get_map().get_width();
	const int16_t map_h = egbase.get_map().get_height();
	const int32_t maxx = MapviewPixelFunctions::get_map_end_screen_x(egbase.get_map());
	const int32_t maxy = MapviewPixelFunctions::get_map_end_screen_y(egbase.get_map());
	// adjust the viewpoint top topleft in map coords
	viewpoint.x += g_gr->get_xres() / 2;
	if (viewpoint.x >= maxx) {
		viewpoint.x -= maxx;
	}
	viewpoint.y += g_gr->get_yres() / 2;
	if (viewpoint.y >= maxy) {
		viewpoint.y -= maxy;
	}
	viewpoint.x /= kTriangleWidth;
	viewpoint.y /= kTriangleHeight;
	viewpoint.x -= map_w / 2;
	viewpoint.y -= map_h / 2;

	// Render minimap
	std::unique_ptr<Texture> texture(draw_minimap(egbase, player, viewpoint, layers));
	save_to_png(texture.get(), streamwrite, ColorType::RGBA);
}
