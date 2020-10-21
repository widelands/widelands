/*
 * Copyright (C) 2010-2020 by the Widelands Development Team
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
#include "economy/roadbase.h"
#include "graphic/playercolor.h"
#include "logic/field.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/terrain_description.h"
#include "wui/mapviewpixelfunctions.h"

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
		color = egbase.descriptions()
		           .get_terrain_descr(f.field->terrain_d())
		           ->get_minimap_color(f.field->get_brightness());
	}

	if (layers & MiniMapLayer::Owner) {
		if (0 < owner) {
			color = blend_color(color, egbase.player(owner).get_playercolor());
		}
	}

	if (see_details) {
		// if ownership layer is displayed, it creates enough contrast to
		// visualize objects using white color.

		if (upcast(Widelands::PlayerImmovable const, immovable, f.field->get_immovable())) {
			if ((layers & MiniMapLayer::Road) && dynamic_cast<Widelands::RoadBase const*>(immovable)) {
				color = blend_color(color, kWhite);
			}

			if (((layers & MiniMapLayer::Flag) && dynamic_cast<Widelands::Flag const*>(immovable)) ||
			    ((layers & MiniMapLayer::Building) &&
			     dynamic_cast<Widelands::Building const*>(immovable))) {
				color = kWhite;
			}
		}

		if (layers & MiniMapLayer::StartingPositions) {
			const Widelands::Map& map = egbase.map();
			Widelands::Coords starting_pos;
			uint32_t dist;
			for (uint32_t p = 1; p <= map.get_nrplayers(); p++) {
				starting_pos = map.get_starting_pos(p);
				dist = map.calc_distance(f, starting_pos);
				if (dist < 9) {
					color = dist == 0 ? kWhite : blend_color(color, kPlayerColors[p - 1]);
					break;
				}
			}
		}
	}

	return color;
}

void draw_view_window(const Widelands::Map& map,
                      const Rectf& view_area,
                      const MiniMapType minimap_type,
                      const bool zoom,
                      Texture* texture) {
	const float multiplier = scale_map(map, zoom);
	const int half_width =
	   round_up_to_nearest_even(std::ceil(view_area.w / kTriangleWidth * multiplier / 2.f));
	const int half_height =
	   round_up_to_nearest_even(std::ceil(view_area.h / kTriangleHeight * multiplier / 2.f));

	Vector2i center_pixel = Vector2i::zero();
	switch (minimap_type) {
	case MiniMapType::kStaticViewWindow:
		center_pixel = Vector2i(texture->width() / 2, texture->height() / 2);
		break;

	case MiniMapType::kStaticMap: {
		Vector2f origin = view_area.center();
		MapviewPixelFunctions::normalize_pix(map, &origin);
		center_pixel =
		   Vector2i(origin.x / kTriangleWidth, origin.y / kTriangleHeight) * scale_map(map, zoom);
		break;
	}
	}

	const int width = map.get_width() * scale_map(map, zoom);
	const int height = map.get_height() * scale_map(map, zoom);
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

// Does the actual work of drawing the minimap.
void do_draw_minimap(Texture* texture,
                     const Widelands::EditorGameBase& egbase,
                     const Widelands::Player* player,
                     const Vector2i& top_left,
                     MiniMapLayer layers) {
	const Widelands::Map& map = egbase.map();
	const uint16_t surface_h = texture->height();
	const uint16_t surface_w = texture->width();
	const int32_t mapwidth = map.get_width();

	for (uint32_t y = 0; y < surface_h; ++y) {
		for (uint32_t x = 0; x < surface_w; ++x) {
			Widelands::Coords coords(
			   Widelands::Coords(top_left.x + x / scale_map(map, layers & MiniMapLayer::Zoom2),
			                     top_left.y + y / scale_map(map, layers & MiniMapLayer::Zoom2)));
			map.normalize_coords(coords);
			Widelands::FCoords f = map.get_fcoords(coords);
			Widelands::MapIndex i = Widelands::Map::get_index(f, mapwidth);
			move_r(mapwidth, f, i);

			Widelands::SeeUnseeNode vision;
			Widelands::PlayerNumber owner;
			if (player == nullptr || player->see_all()) {
				// This player has omnivision - show the field like it is in reality.
				vision = Widelands::SeeUnseeNode::kVisible;  // Seen right now.
				owner = f.field->get_owned_by();
			} else {
				// This player might be affected by fog of war - instead of the
				// reality, we show her what she last saw on this field. If she has
				// vision of this field, this will be the same as reality -
				// otherwise this shows reality as it was the last time she had
				// vision on the field.
				// If she never had vision, field.vision will be 0.
				const auto& field = player->fields()[i];
				vision = field.seeing;
				owner = field.owner;
			}

			if (vision != Widelands::SeeUnseeNode::kUnexplored) {
				texture->set_pixel(x, y,
				                   calc_minimap_color(egbase, f, layers, owner,
				                                      vision == Widelands::SeeUnseeNode::kVisible));
			}
		}
	}
}

}  // namespace

Vector2f minimap_pixel_to_mappixel(const Widelands::Map& map,
                                   const Vector2i& minimap_pixel,
                                   const Rectf& view_area,
                                   MiniMapType minimap_type,
                                   const bool zoom) {
	Vector2f top_left = Vector2f::zero();
	switch (minimap_type) {
	case MiniMapType::kStaticViewWindow:
		top_left =
		   view_area.center() -
		   Vector2f(map.get_width() * kTriangleWidth, map.get_height() * kTriangleHeight) / 2.f;
		break;

	case MiniMapType::kStaticMap:
		top_left = Vector2f::zero();
		break;
	}

	const float multiplier = scale_map(map, zoom);
	Vector2f map_pixel = top_left + Vector2f(minimap_pixel.x / multiplier * kTriangleWidth,
	                                         minimap_pixel.y / multiplier * kTriangleHeight);
	MapviewPixelFunctions::normalize_pix(map, &map_pixel);
	return map_pixel;
}

std::unique_ptr<Texture> draw_minimap(const Widelands::EditorGameBase& egbase,
                                      const Widelands::Player* player,
                                      const Rectf& view_area,
                                      const MiniMapType& minimap_type,
                                      MiniMapLayer layers) {
	// TODO(sirver): Currently the minimap is redrawn every frame. That is not really
	//       necessary. The created texture could be cached and only redrawn two
	//       or three times per second
	const Widelands::Map& map = egbase.map();
	const int16_t map_w = map.get_width() * scale_map(map, layers & MiniMapLayer::Zoom2);
	const int16_t map_h = map.get_height() * scale_map(map, layers & MiniMapLayer::Zoom2);

	std::unique_ptr<Texture> texture(new Texture(map_w, map_h));

	texture->fill_rect(
	   Rectf(0.f, 0.f, texture->width(), texture->height()), RGBAColor(0, 0, 0, 255));

	// Center the view on the middle of the 'view_area'.
	const bool zoom = layers & MiniMapLayer::Zoom2;
	Vector2f top_left =
	   minimap_pixel_to_mappixel(map, Vector2i::zero(), view_area, minimap_type, zoom);
	const Widelands::Coords node =
	   MapviewPixelFunctions::calc_node_and_triangle(map, top_left.x, top_left.y).node;

	texture->lock();
	do_draw_minimap(texture.get(), egbase, player, Vector2i(node.x, node.y), layers);

	if (layers & MiniMapLayer::ViewWindow) {
		draw_view_window(map, view_area, minimap_type, zoom, texture.get());
	}
	texture->unlock(Texture::Unlock_Update);

	return texture;
}

int scale_map(const Widelands::Map& map, bool zoom) {
	// The MiniMap can have a maximum size of 600px. If a map is wider than 300px we don't scale.
	// Otherwise we fit as much as possible into a 300px/400px MiniMap window when zoom is disabled.
	const uint16_t map_w = map.get_width();
	if (!(map_w > 300)) {
		if (zoom) {
			return (600 - (600 % map_w)) / map_w;
		} else if (map_w > 150) {
			return (400 - (400 % map_w)) / map_w;
		} else {
			return (300 - (300 % map_w)) / map_w;
		}
	}
	return 1;
}
