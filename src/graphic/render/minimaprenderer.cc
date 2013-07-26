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

#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/graphic.h"
#include "graphic/render/terrain_sdl.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "graphic/texture.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"
#include "upcast.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/minimap.h"
#include "wui/overlay_manager.h"

using namespace Widelands;

/**
 * Renders a minimap into the current window. The field at viewpoint will be
 * in the top-left corner of the window. Flags specifies what information to
 * display (see Minimap_XXX enums).
 *
 * Calculate the field at the top-left corner of the clipping rect
 * The entire clipping rect will be used for drawing.
 */
void MiniMapRenderer::renderminimap
	(const Widelands::Editor_Game_Base &       egbase,
	 Player                      const * const player,
	 Point                               const viewpoint,
	 uint32_t                            const flags)
{
	draw_minimap
		(egbase, player, m_rect, viewpoint -
			((flags & MiniMap::Zoom2) ? Point(m_offset.x / 2, m_offset.y / 2) : m_offset),
				viewpoint, flags);
}

/*
 * Blend to colors; only needed for calc_minimap_color below
 */

inline static uint32_t blend_color
	(const SDL_PixelFormat &       format,
	 uint32_t                const clr1,
	 Uint8 const r2, Uint8 const g2, Uint8 const b2)
{
	Uint8 r1, g1, b1;
	SDL_GetRGB(clr1, &const_cast<SDL_PixelFormat &>(format), &r1, &g1, &b1);
	return
		SDL_MapRGB
			(&const_cast<SDL_PixelFormat &>(format),
			 (r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2);
}

/*
===============
Return the color to be used in the minimap for the given field.
===============
*/

inline static uint32_t calc_minimap_color
	(const SDL_PixelFormat             &       format,
	 const Widelands::Editor_Game_Base &       egbase,
	 Widelands::FCoords                  const f,
	 uint32_t                            const flags,
	 Widelands::Player_Number            const owner,
	 bool                                const see_details)
{
	uint32_t pixelcolor = 0;

	if (flags & MiniMap::Terrn) {
		pixelcolor =
			g_gr->
			get_maptexture_data
				(egbase.map().world()
				 .terrain_descr(f.field->terrain_d()).get_texture())
			->get_minimap_color(f.field->get_brightness());
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

	if (see_details)
		// if ownership layer is displayed, it creates enoungh contrast
		// to visualize objects using white color.
		// Otherwise, a more contrasting color may be needed:
		// * winterland -> orange

		if (upcast(PlayerImmovable const, immovable, f.field->get_immovable())) {
			if ((flags & MiniMap::Roads) and dynamic_cast<Road const *>(immovable))
			{
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

	return pixelcolor;
}


/*
===============
Used to draw a dotted frame border on the mini map.
===============
 */
template<typename T>
static bool draw_minimap_frameborder
	(Widelands::FCoords  const f,
	 Point               const ptopleft,
	 Point               const pbottomright,
	 int32_t             const mapwidth,
	 int32_t             const mapheight,
	 int32_t             const modx,
	 int32_t             const mody)
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

/*
 *
 *
 *
 */
template<typename T>
static void draw_minimap_int
	(Uint8                             * const pixels,
	 uint16_t                            const pitch,
	 const SDL_PixelFormat             &       format,
	 int32_t                            const mapwidth,
	 const Widelands::Editor_Game_Base &       egbase,
	 Widelands::Player           const * const player,
	 Rect                                const rc,
	 Point                               const viewpoint,
	 Point                               const framepoint,
	 uint32_t                            const flags)
{
	const Widelands::Map & map = egbase.map();

	int32_t mapheight = map.get_height();

	// size of the display frame
	int32_t xsize = g_gr->get_xres() / TRIANGLE_WIDTH / 2;
	int32_t ysize = g_gr->get_yres() / TRIANGLE_HEIGHT / 2;

	Point ptopleft; // top left point of the current display frame
	ptopleft.x = framepoint.x + mapwidth / 2 - xsize;
	if (ptopleft.x < 0) ptopleft.x += mapwidth;
	ptopleft.y = framepoint.y + mapheight / 2 - ysize;
	if (ptopleft.y < 0) ptopleft.y += mapheight;

	Point pbottomright; // bottom right point of the current display frame
	pbottomright.x = framepoint.x + mapwidth / 2 + xsize;
	if (pbottomright.x >= mapwidth) pbottomright.x -= mapwidth;
	pbottomright.y = framepoint.y + mapheight / 2 + ysize;
	if (pbottomright.y >= mapheight) pbottomright.y -= mapheight;

	uint32_t modx = pbottomright.x % 2;
	uint32_t mody = pbottomright.y % 2;

	if (not player or player->see_all()) for (uint32_t y = 0; y < rc.h; ++y) {
		Uint8 * pix = pixels + (rc.y + y) * pitch + rc.x * sizeof(T);
		Widelands::FCoords f
			(Widelands::Coords
			 	(viewpoint.x, viewpoint.y + (flags & MiniMap::Zoom2 ? y / 2 : y)));
		map.normalize_coords(f);
		f.field = &map[f];
		Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
		for (uint32_t x = 0; x < rc.w; ++x, pix += sizeof(T)) {
			if (x % 2 || !(flags & MiniMap::Zoom2))
				move_r(mapwidth, f, i);

			if
				(draw_minimap_frameborder<T>
				 (f, ptopleft, pbottomright, mapwidth, mapheight, modx, mody))
			{
				*reinterpret_cast<T *>(pix) = static_cast<T>
					(SDL_MapRGB(&const_cast<SDL_PixelFormat &>(format), 255, 0, 0));
			} else {
				*reinterpret_cast<T *>(pix) = static_cast<T>
					(calc_minimap_color
				 		(format, egbase, f, flags, f.field->get_owned_by(), true));
			}
		}
	} else {
		Widelands::Player::Field const * const player_fields = player->fields();
		for (uint32_t y = 0; y < rc.h; ++y) {
			Uint8 * pix = pixels + (rc.y + y) * pitch + rc.x * sizeof(T);
			Widelands::FCoords f
				(Widelands::Coords
			 		(viewpoint.x, viewpoint.y +
			 		 (flags & MiniMap::Zoom2 ? y / 2 : y)));
			map.normalize_coords(f);
			f.field = &map[f];
			Widelands::Map_Index i = Widelands::Map::get_index(f, mapwidth);
			for (uint32_t x = 0; x < rc.w; ++x, pix += sizeof(T)) {
				if (x % 2 || !(flags & MiniMap::Zoom2))
					move_r(mapwidth, f, i);

				if
					(draw_minimap_frameborder<T>
					 (f, ptopleft, pbottomright, mapwidth, mapheight, modx, mody))
				{
					*reinterpret_cast<T *>(pix) = static_cast<T>
						(SDL_MapRGB
							(&const_cast<SDL_PixelFormat &>(format), 255, 0, 0));
				} else {
					const Widelands::Player::Field & player_field = player_fields[i];
					Widelands::Vision const vision = player_field.vision;

					*reinterpret_cast<T *>(pix) =
						static_cast<T>
						(vision ?
						 calc_minimap_color
						 	(format,
						 	 egbase,
						 	 f,
						 	 flags,
						 	 player_field.owner,
						 	 1 < vision)
						 :
						 0);
				}
			}
		}
	}
}


/*
===============
Draw a minimap into the given rectangle of the bitmap.
viewpt is the field at the top left of the rectangle.
===============
*/
void MiniMapRenderer::draw_minimap
	(const Widelands::Editor_Game_Base &       egbase,
	 Widelands::Player           const * const player,
	 Rect                                const rc,
	 Point                               const viewpt,
	 Point                               const framept,
	 uint32_t                            const flags)
{
	// First create a temporary SDL Surface to draw the minimap. This Surface is
	// is created in almost display pixel format without alpha channel.
	// Bits per pixels must be 16 or 32 for the minimap code to work
	// TODO: Currently the minimap is redrawn every frame. That is not really
	//       necesary. The created surface could be cached and only redrawn two
	//       or three times per second
	const SDL_PixelFormat & fmt =
		g_gr->get_render_target()->get_surface()->format();
	SDL_Surface * surface =
		SDL_CreateRGBSurface
			(SDL_SWSURFACE,
			 rc.w,
			 rc.h,
			 fmt.BytesPerPixel == 2 ? 16 : 32,
			 fmt.Rmask,
			 fmt.Gmask,
			 fmt.Bmask,
			 0);

	Rect rc2;
	rc2.x = rc2.y = 0;
	rc2.w = rc.w;
	rc2.h = rc.h;

	SDL_FillRect(surface, 0, SDL_MapRGBA(surface->format, 0, 0, 0, 255));
	SDL_LockSurface(surface);

	Uint8 * const pixels = static_cast<uint8_t *>(surface->pixels);
	Widelands::X_Coordinate const w = egbase.map().get_width();
	switch (surface->format->BytesPerPixel) {
	case sizeof(Uint16):
		draw_minimap_int<Uint16>
			(pixels, surface->pitch, *surface->format,
			 w, egbase, player, rc2, viewpt, framept, flags);
		break;
	case sizeof(Uint32):
		draw_minimap_int<Uint32>
			(pixels, surface->pitch, *surface->format,
			 w, egbase, player, rc2, viewpt, framept, flags);
		break;
	default:
		assert(false);
		break;
	}

	SDL_UnlockSurface(surface);

	std::unique_ptr<Surface> minimap_surface(Surface::create(surface));
	m_surface->blit(Point(rc.x, rc.y), minimap_surface.get(), rc2);
}
