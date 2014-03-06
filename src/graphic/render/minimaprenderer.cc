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

#include "graphic/render/minimaprenderer.h"

#include <memory>

#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/in_memory_image.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"
#include "upcast.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"

using namespace Widelands;

namespace  {

// Blend two colors.
inline uint32_t blend_color
	(const SDL_PixelFormat& format, uint32_t clr1, Uint8 r2, Uint8 g2, Uint8 b2)
{
	Uint8 r1, g1, b1;
	SDL_GetRGB(clr1, &const_cast<SDL_PixelFormat &>(format), &r1, &g1, &b1);
	return
		SDL_MapRGB
			(&const_cast<SDL_PixelFormat &>(format), (r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2);
}

// Returns the color to be used in the minimap for the given field.
inline uint32_t calc_minimap_color
	(const SDL_PixelFormat& format, const Widelands::Editor_Game_Base& egbase,
	 const Widelands::FCoords& f, uint32_t flags, Widelands::Player_Number owner,
	 bool see_details)
{
	uint32_t pixelcolor = 0;

	if (flags & MiniMap::Terrn) {
		// We or with format.Amask here because the terrain map texture data is
		// only RGB and not RGBA in SDL render mode. Or'ing with Amask guarantees
		// that alpha is set to 255 for this color and is fast.
		pixelcolor =
			g_gr->
			get_maptexture_data
				(egbase.map().world()
				 .terrain_descr(f.field->terrain_d()).get_texture())
			->get_minimap_color(f.field->get_brightness()) | format.Amask;
	}

	if (flags & MiniMap::Owner) {
		if (0 < owner) { //  If owned, get the player's color...
			const RGBColor & player_color = egbase.player(owner).get_playercolor();

			//  ...and add the player's color to the old color.
			pixelcolor = blend_color
				(format,
				 pixelcolor,
				 player_color.r,  player_color.g, player_color.b);
		}
	}

	if (see_details) {
		// if ownership layer is displayed, it creates enough contrast to
		// visualize objects using white color.
		// Otherwise, a more contrasting color may be needed:
		// * winterland -> orange

		if (upcast(PlayerImmovable const, immovable, f.field->get_immovable())) {
			if ((flags & MiniMap::Roads) and dynamic_cast<Road const *>(immovable)) {
				if (!(flags & MiniMap::Owner) && !strcmp(egbase.map().get_world_name(), "winterland"))
						pixelcolor = blend_color(format, pixelcolor, 255, 127, 0);
				else //ownership layer displayed or greenland
						pixelcolor = blend_color(format, pixelcolor, 255, 255, 255);
			}

			if
				(((flags & MiniMap::Flags) and dynamic_cast<Flag const *>(immovable))
				 or
				 ((flags & MiniMap::Bldns)
				  and
				  dynamic_cast<Widelands::Building const *>(immovable)))
			{
				if (!(flags & MiniMap::Owner) && !strcmp(egbase.map().get_world_name(), "winterland"))
					pixelcolor =
						SDL_MapRGB
							(&const_cast<SDL_PixelFormat &>(format), 255, 127, 0);
				else //ownership layer displayed or greenland
					pixelcolor =
						SDL_MapRGB
							(&const_cast<SDL_PixelFormat &>(format), 255, 255, 255);
			}
		}
	}

	return pixelcolor;
}

// Draws the dotted frame border onto the minimap.
bool is_minimap_frameborder
	(const Widelands::FCoords& f, const Point& ptopleft, const Point& pbottomright,
	 int32_t mapwidth, int32_t mapheight, int32_t modx, int32_t mody)
{
	bool isframepixel = false;

	if (ptopleft.x <= pbottomright.x) {
		if
			(f.x >= ptopleft.x && f.x <= pbottomright.x
			 && (f.y == ptopleft.y || f.y == pbottomright.y)
			 && f.x % 2 == modx)
			isframepixel = true;
	} else {
		if
			(((f.x >= ptopleft.x && f.x <= mapwidth)
			  ||
			  (f.x >= 0 && f.x <= pbottomright.x))
			 &&
			 (f.y == ptopleft.y || f.y == pbottomright.y)
			 &&
			 (f.x % 2) == modx)
			isframepixel = true;
	}

	if (ptopleft.y <= pbottomright.y) {
		if
			(f.y >= ptopleft.y && f.y <= pbottomright.y
			 && (f.x == ptopleft.x || f.x == pbottomright.x)
			 && f.y % 2 == mody)
			isframepixel = true;
	} else {
		if
			(((f.y >= ptopleft.y && f.y <= mapheight)
			  ||
			  (f.y >= 0 && f.y <= pbottomright.y))
			 &&
			 (f.x == ptopleft.x || f.x == pbottomright.x)
			 &&
			 f.y % 2 == mody)
			isframepixel = true;
	}

	return isframepixel;
}

// Does the actual work of drawing the minimap.
void draw_minimap_int
	(Surface* surface, const Widelands::Editor_Game_Base& egbase,
	 const Widelands::Player* player, const Point& viewpoint, uint32_t flags)
{
	const Widelands::Map & map = egbase.map();

	Uint8* const pixels = surface->get_pixels();
	const SDL_PixelFormat& format = surface->format();
	const uint16_t pitch = surface->get_pitch();
	const uint16_t surface_h = surface->height();
	const uint16_t surface_w = surface->width();

	// size of the display frame
	int32_t xsize = g_gr->get_xres() / TRIANGLE_WIDTH / 2;
	int32_t ysize = g_gr->get_yres() / TRIANGLE_HEIGHT / 2;

	const int32_t mapwidth = egbase.get_map().get_width();
	const int32_t mapheight = map.get_height();

	Point ptopleft; // top left point of the current display frame
	ptopleft.x = viewpoint.x + mapwidth / 2 - xsize;
	if (ptopleft.x < 0) ptopleft.x += mapwidth;
	ptopleft.y = viewpoint.y + mapheight / 2 - ysize;
	if (ptopleft.y < 0) ptopleft.y += mapheight;

	Point pbottomright; // bottom right point of the current display frame
	pbottomright.x = viewpoint.x + mapwidth / 2 + xsize;
	if (pbottomright.x >= mapwidth) pbottomright.x -= mapwidth;
	pbottomright.y = viewpoint.y + mapheight / 2 + ysize;
	if (pbottomright.y >= mapheight) pbottomright.y -= mapheight;

	uint32_t modx = pbottomright.x % 2;
	uint32_t mody = pbottomright.y % 2;

	if (not player or player->see_all()) {
			for (uint32_t y = 0; y < surface_h; ++y) {
			Uint8 * pix = pixels + y * pitch;
			Widelands::FCoords f
				(Widelands::Coords
					(viewpoint.x, viewpoint.y + (flags & MiniMap::Zoom2 ? y / 2 : y)));
			map.normalize_coords(f);
			f.field = &map[f];
			Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
			for (uint32_t x = 0; x < surface_w; ++x, pix += sizeof(Uint32)) {
				if (x % 2 || !(flags & MiniMap::Zoom2))
					move_r(mapwidth, f, i);

				if
					(is_minimap_frameborder
					 (f, ptopleft, pbottomright, mapwidth, mapheight, modx, mody))
				{
					*reinterpret_cast<Uint32 *>(pix) = static_cast<Uint32>
						(SDL_MapRGB(&const_cast<SDL_PixelFormat &>(format), 255, 0, 0));
				} else {
					*reinterpret_cast<Uint32 *>(pix) = static_cast<Uint32>
						(calc_minimap_color
							(format, egbase, f, flags, f.field->get_owned_by(), true));
				}
			}
		}
	} else {
		Widelands::Player::Field const * const player_fields = player->fields();
		for (uint32_t y = 0; y < surface_h; ++y) {
			Uint8 * pix = pixels + y * pitch;
			Widelands::FCoords f
				(Widelands::Coords
			 		(viewpoint.x, viewpoint.y +
			 		 (flags & MiniMap::Zoom2 ? y / 2 : y)));
			map.normalize_coords(f);
			f.field = &map[f];
			Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
			for (uint32_t x = 0; x < surface_w; ++x, pix += sizeof(Uint32)) {
				if (x % 2 || !(flags & MiniMap::Zoom2))
					move_r(mapwidth, f, i);

				if
					(is_minimap_frameborder
					 (f, ptopleft, pbottomright, mapwidth, mapheight, modx, mody))
				{
					*reinterpret_cast<Uint32 *>(pix) = static_cast<Uint32>
						(SDL_MapRGB
							(&const_cast<SDL_PixelFormat &>(format), 255, 0, 0));
				} else {
					const Widelands::Player::Field & player_field = player_fields[i];
					Widelands::Vision const vision = player_field.vision;

					*reinterpret_cast<Uint32 *>(pix) =
						static_cast<Uint32>
						(vision ?
						 calc_minimap_color
						 	(format,
						 	 egbase,
						 	 f,
						 	 flags,
						 	 player_field.owner,
						 	 1 < vision)
						 :
						 SDL_MapRGB(&const_cast<SDL_PixelFormat &>(format), 0, 0, 0));
				}
			}
		}
	}
}

// A helper function to draw the minimap. This is called from
// renderminimap().
Surface* draw_minimap
	(const Widelands::Editor_Game_Base& egbase, const Widelands::Player* player,
	 const Point& viewpt, uint32_t flags)
{
	// First create a temporary SDL Surface to draw the minimap.
	// TODO: Currently the minimap is redrawn every frame. That is not really
	//       necesary. The created surface could be cached and only redrawn two
	//       or three times per second
	const Map& map = egbase.map();
	const int16_t map_w = (flags & MiniMap::Zoom2) ? map.get_width() * 2 : map.get_width();
	const int16_t map_h = (flags & MiniMap::Zoom2) ? map.get_height() * 2 : map.get_height();

	Surface* surface = Surface::create(map_w, map_h);
	assert(surface->format().BytesPerPixel == sizeof(Uint32));

	surface->fill_rect(Rect(0, 0, surface->width(), surface->height()), RGBAColor(0, 0, 0, 255));
	surface->lock(Surface::Lock_Normal);

	draw_minimap_int(surface, egbase, player, viewpt, flags);

	surface->unlock(Surface::Unlock_Update);

	return surface;
}

}  // namespace

Surface* MiniMapRenderer::get_minimap_image
	(const Editor_Game_Base& egbase, const Player* player, const Point& viewpoint, uint32_t flags)
{
	Surface* minimap_surf = draw_minimap(egbase, player, viewpoint, flags);
	return minimap_surf;
}

void MiniMapRenderer::write_minimap_image
	(const Editor_Game_Base& egbase, const Player* player, const Point& gviewpoint, uint32_t flags,
	 ::StreamWrite* const streamwrite)
{
	assert(streamwrite != nullptr);

	Point viewpoint(gviewpoint);

	// map dimension
	const int16_t map_w = egbase.get_map().get_width();
	const int16_t map_h = egbase.get_map().get_height();
	const int32_t maxx = MapviewPixelFunctions::get_map_end_screen_x(egbase.get_map());
	const int32_t maxy = MapviewPixelFunctions::get_map_end_screen_y(egbase.get_map());
	// adjust the viewpoint top topleft in map coords
	viewpoint.x += g_gr->get_xres() / 2;
	if (viewpoint.x >= maxx)
		viewpoint.x -= maxx;
	viewpoint.y += g_gr->get_yres() / 2;
	if (viewpoint.y >= maxy)
		viewpoint.y -= maxy;
	viewpoint.x /= TRIANGLE_WIDTH;
	viewpoint.y /= TRIANGLE_HEIGHT;
	viewpoint.x -= map_w / 2;
	viewpoint.y -= map_h / 2;

	// Render minimap
	std::unique_ptr<Surface> surface
		(get_minimap_image(egbase, player, viewpoint, flags));
	std::unique_ptr<const Image> image(new_in_memory_image("minimap", surface.release()));
	g_gr->save_png(image.get(), streamwrite);
	image.reset();
}
