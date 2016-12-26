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
#include "graphic/game_renderer_gl4.h"
#include "graphic/gl/terrain_program_gl4.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"
#include "logic/field.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

namespace {

const RGBColor kWhite(255, 255, 255);
const RGBColor kRed(255, 0, 0);

// Blend two colors.
inline RGBColor blend_color(const RGBColor& c1, const RGBColor& c2) {
	return RGBColor((c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2);
}

int round_up_to_nearest_even(int number) {
	return number % 2 == 0 ? number : number + 1;
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

void draw_view_window(const Map& map,
                      const Rectf& view_area,
                      const MiniMapType minimap_type,
                      const bool zoom,
                      Texture* texture) {
	const float divider = zoom ? 1.f : 2.f;
	const int half_width =
	   round_up_to_nearest_even(std::ceil(view_area.w / kTriangleWidth / divider));
	const int half_height =
	   round_up_to_nearest_even(std::ceil(view_area.h / kTriangleHeight / divider));

	Vector2i center_pixel;
	switch (minimap_type) {
	case MiniMapType::kStaticViewWindow:
		center_pixel = Vector2i(texture->width() / 2, texture->height() / 2);
		break;

	case MiniMapType::kStaticMap: {
		Vector2f origin = view_area.center();
		MapviewPixelFunctions::normalize_pix(map, &origin);
		center_pixel =
		   Vector2i(origin.x / kTriangleWidth, origin.y / kTriangleHeight) * (zoom ? 2 : 1);
		break;
	}
	}

	const int width = zoom ? map.get_width() * 2 : map.get_width();
	const int height = zoom ? map.get_height() * 2 : map.get_height();
	const auto make_red = [width, height, &texture](int x, int y) {
		if (x < 0) {
			x += width;
		}
		if (x >= width) {
			x -= width;
		}
		if (y < 0) {
			y += height;
		}
		if (y >= height) {
			y -= height;
		}
		texture->set_pixel(x, y, kRed);
	};

	bool draw = true;
	for (int y = -half_height; y <= half_height; ++y) {
		if (draw) {
			make_red(-half_width + center_pixel.x, y + center_pixel.y);
			make_red(half_width + center_pixel.x, y + center_pixel.y);
		}
		draw = !draw;
	}

	draw = true;
	for (int x = -half_width; x <= half_width; ++x) {
		if (draw) {
			make_red(x + center_pixel.x, -half_height + center_pixel.y);
			make_red(x + center_pixel.x, half_height + center_pixel.y);
		}
		draw = !draw;
	}
}

#if 0
// Calculate the field coordinates of the dotted frame indicating where the
// main map view is currently centered, without wrap-around.
void calc_minimap_frame(const Widelands::Map& map,
                        const Point& viewpoint,
                        Point& topleft,
                        Point& bottomright) {
	int32_t xsize = g_gr->get_xres() / kTriangleWidth / 2;
	int32_t ysize = g_gr->get_yres() / kTriangleHeight / 2;

	const int32_t mapwidth = map.get_width();
	const int32_t mapheight = map.get_height();

	topleft.x = viewpoint.x + mapwidth / 2 - xsize;
	topleft.y = viewpoint.y + mapheight / 2 - ysize;

	bottomright.x = viewpoint.x + mapwidth / 2 + xsize;
	bottomright.y = viewpoint.y + mapheight / 2 + ysize;
}
#endif

// Does the actual work of drawing the minimap.
void do_draw_minimap(Texture* texture,
                     const Widelands::EditorGameBase& egbase,
                     const Widelands::Player* player,
                     const Vector2i& top_left,
                     MiniMapLayer layers) {
	const Widelands::Map& map = egbase.map();
	const uint16_t surface_h = texture->height();
	const uint16_t surface_w = texture->width();
	const int32_t mapwidth = egbase.get_map().get_width();

	for (uint32_t y = 0; y < surface_h; ++y) {
		Widelands::FCoords f(
		   Widelands::Coords(top_left.x, top_left.y + (layers & MiniMapLayer::Zoom2 ? y / 2 : y)));
		map.normalize_coords(f);
		f.field = &map[f];
		Widelands::MapIndex i = Widelands::Map::get_index(f, mapwidth);
		for (uint32_t x = 0; x < surface_w; ++x) {
			if (x % 2 || !(layers & MiniMapLayer::Zoom2)) {
				move_r(mapwidth, f, i);
			}

			uint16_t vision = 0;  // See Player::Field::Vision: 1 if seen once, > 1 if seen right now.
			Widelands::PlayerNumber owner = 0;
			if (player == nullptr || player->see_all()) {
				// This player has omnivision - show the field like it is in reality.
				vision = 2;  // Seen right now.
				owner = f.field->get_owned_by();
			} else if (player != nullptr) {
				// This player might be affected by fog of war - instead of the
				// reality, we show her what she last saw on this field. If she has
				// vision of this field, this will be the same as reality -
				// otherwise this shows reality as it was the last time she had
				// vision on the field.
				// If she never had vision, field.vision will be 0.
				const auto& field = player->fields()[i];
				vision = field.vision;
				owner = field.owner;
			}

			RGBColor color;
			if (vision > 0)
				color = calc_minimap_color(egbase, f, layers, owner, vision > 1);
			texture->set_pixel(x, y, color);
		}
	}
}

/**
 * Mini-map renderer implementation that generates a mini-map texture in
 * software and blits it.
 */
class MiniMapRendererSoftware : public MiniMapRenderer {
public:
	MiniMapRendererSoftware(const Widelands::EditorGameBase& egbase,
	                        const Widelands::Player* player)
	  : MiniMapRenderer(egbase, player) {
	}

	void draw(RenderTarget& dst,
	          const Rectf& viewarea,
	          MiniMapType type,
	          MiniMapLayer layers) override {
		texture_ = draw_minimap(egbase(), player(), viewarea, type, layers);
		dst.blit(Vector2f(), texture_.get());
	}

private:
	std::unique_ptr<Texture> texture_;
};

/**
 * Mini-map renderer that delegates to the GL4 programs for mini-map drawing
 * in a pixel shader.
 */
class MiniMapRendererGl4 : public MiniMapRenderer {
public:
	MiniMapRendererGl4(const Widelands::EditorGameBase& egbase,
	                   const Widelands::Player* player)
	  : MiniMapRenderer(egbase, player) {
		  args_.terrain = TerrainInformationGl4::get(egbase, player);
	}

	void draw(RenderTarget& dst,
	          const Rectf& viewpoint,
	          MiniMapType type,
	          MiniMapLayer layers) override {
		Surface* surface = dst.get_surface();
		if (!surface)
			return;

		// TODO(nha): handle MiniMapType

		args_.terrain->update_minimap();

#if 0
		// TODO(nha): frame
		Point frame_topleft, frame_bottomright;
		calc_minimap_frame(egbase().map(), viewpoint, frame_topleft, frame_bottomright);

		args_.minfx = frame_topleft.x;
		args_.minfy = frame_topleft.y;
		args_.maxfx = frame_bottomright.x;
		args_.maxfy = frame_bottomright.y;
#else
		args_.minfx = args_.maxfx = args_.minfy = args_.maxfy = 0;
#endif

		args_.minimap_tl_fx = viewpoint.x;
		args_.minimap_tl_fy = viewpoint.y;
		args_.minimap_layers = layers;

		const Recti& bounding_rect = dst.get_rect();

		args_.surface_offset = (bounding_rect.origin() + dst.get_offset()).cast<float>();
		args_.surface_width = surface->width();
		args_.surface_height = surface->height();

		// Enqueue the drawing.
		RenderQueue::Item i;
		i.program_id = RenderQueue::Program::kMiniMapGl4;
		i.blend_mode = BlendMode::Copy;
		i.terrain_arguments.destination_rect =
			Rectf(bounding_rect.x, args_.surface_height - bounding_rect.y - bounding_rect.h,
			      bounding_rect.w, bounding_rect.h);
		i.terrain_arguments.renderbuffer_width = args_.surface_width;
		i.terrain_arguments.renderbuffer_height = args_.surface_height;
		i.terrain_gl4_arguments = &args_;
		RenderQueue::instance().enqueue(i);
	}

private:
	TerrainGl4Arguments args_;
};

}  // namespace

Vector2f minimap_pixel_to_mappixel(const Widelands::Map& map,
                                   const Vector2i& minimap_pixel,
                                   const Rectf& view_area,
                                   MiniMapType minimap_type,
                                   const bool zoom) {
	Vector2f top_left;
	switch (minimap_type) {
	case MiniMapType::kStaticViewWindow:
		top_left =
		   view_area.center() -
		   Vector2f(map.get_width() * kTriangleWidth, map.get_height() * kTriangleHeight) / 2.f;
		break;

	case MiniMapType::kStaticMap:
		top_left = Vector2f(0., 0.);
		break;
	}

	const float multiplier = zoom ? 2.f : 1.f;
	Vector2f map_pixel = top_left + Vector2f(minimap_pixel.x / multiplier * kTriangleWidth,
	                                         minimap_pixel.y / multiplier * kTriangleHeight);
	MapviewPixelFunctions::normalize_pix(map, &map_pixel);
	return map_pixel;
}

std::unique_ptr<Texture> draw_minimap(const EditorGameBase& egbase,
                                      const Player* player,
                                      const Rectf& view_area,
                                      const MiniMapType& minimap_type,
                                      MiniMapLayer layers) {
	// TODO(sirver): Currently the minimap is redrawn every frame. That is not really
	//       necessary. The created texture could be cached and only redrawn two
	//       or three times per second
	const Map& map = egbase.map();
	const int16_t map_w = (layers & MiniMapLayer::Zoom2) ? map.get_width() * 2 : map.get_width();
	const int16_t map_h = (layers & MiniMapLayer::Zoom2) ? map.get_height() * 2 : map.get_height();

	// Center the view on the middle of the 'view_area'.
	const bool zoom = layers & MiniMapLayer::Zoom2;
	Vector2f top_left =
	   minimap_pixel_to_mappixel(map, Vector2i(0, 0), view_area, minimap_type, zoom);
	const Coords node =
	   MapviewPixelFunctions::calc_node_and_triangle(map, top_left.x, top_left.y).node;

	std::unique_ptr<Texture> texture(new Texture(map_w, map_h));
	texture->lock(Texture::Lock_Discard);
	do_draw_minimap(texture.get(), egbase, player, Vector2i(node.x, node.y), layers);

	if (layers & MiniMapLayer::ViewWindow) {
		draw_view_window(map, view_area, minimap_type, zoom, texture.get());
	}
	texture->unlock(Texture::Unlock_Update);

	return texture;
}

MiniMapRenderer::MiniMapRenderer(const Widelands::EditorGameBase& egbase,
                                 const Widelands::Player* player)
  : egbase_(egbase), player_(player) {
}

std::unique_ptr<MiniMapRenderer>
MiniMapRenderer::create(const Widelands::EditorGameBase& egbase,
                        const Widelands::Player* player) {
	// TODO(nha): automatic selection
	if (TerrainProgramGl4::supported())
		return std::unique_ptr<MiniMapRenderer>(new MiniMapRendererGl4(egbase, player));
	return std::unique_ptr<MiniMapRenderer>(new MiniMapRendererSoftware(egbase, player));
}
