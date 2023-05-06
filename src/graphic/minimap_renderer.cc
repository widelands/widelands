/*
 * Copyright (C) 2010-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/minimap_renderer.h"

#include <algorithm>
#include <memory>

#include <SDL_timer.h>

#include "economy/flag.h"
#include "economy/road.h"
#include "economy/roadbase.h"
#include "graphic/playercolor.h"
#include "logic/field.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/vision.h"
#include "wui/mapviewpixelfunctions.h"

namespace {

const RGBColor kWhite(255, 255, 255);
const RGBColor kRed(255, 0, 0);
const RGBColor kRoad(220, 220, 220);
const RGBColor kDark(100, 100, 100);
const RGBColor kBright(160, 160, 160);

// Blend two colors.
inline RGBColor blend_color(const RGBColor& c1, const RGBColor& c2) {
	return RGBColor((c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2);
}

inline RGBColor invert_color(const RGBColor& c) {
	// assuming overflow and underflow

	uint8_t r;
	uint8_t g;
	uint8_t b;

	if (c.r > c.g && c.r > c.b && c.g < 200 && c.g + 50 < c.r) {
		r = 0;
		g = c.g;
		b = c.b + (255 - c.b) / 2;
	} else {
		r = 255;
		g = c.g / 2;
		b = c.b / 2;
	}
	return RGBColor(r, g, b);
}

inline RGBColor brighten_color(const RGBColor& c, uint8_t percent) {
	// we need higher range variables to avoid overflow in multiplying
	uint16_t r = c.r;
	uint16_t g = c.g;
	uint16_t b = c.b;

	r = r + ((255 - r) * percent) / 100;
	g = g + ((255 - g) * percent) / 100;
	b = b + ((255 - b) * percent) / 100;

	return RGBColor(r, g, b);
}

inline RGBColor darken_color(const RGBColor& c, uint8_t percent) {
	// we need higher range variables to avoid overflow in multiplying
	uint16_t r = c.r;
	uint16_t g = c.g;
	uint16_t b = c.b;

	r = r - (r * percent) / 100;
	g = g - (g * percent) / 100;
	b = b - (b * percent) / 100;

	return RGBColor(r, g, b);
}

int round_up_to_nearest_even(int number) {
	return number % 2 == 0 ? number : number + 1;
}

// Returns the color to be used in the minimap for the given field.
inline RGBColor calc_minimap_color(const Widelands::EditorGameBase& egbase,
                                   const Widelands::FCoords& f,
                                   const MiniMapLayer layers,
                                   const Widelands::PlayerNumber owner,
                                   const Widelands::Player* const player,
                                   const bool see_details) {
	RGBColor color;
	const Widelands::Map& map = egbase.map();
	if ((layers & MiniMapLayer::Terrain) != 0) {
		color = egbase.descriptions()
		           .get_terrain_descr(f.field->terrain_d())
		           ->get_minimap_color(f.field->get_brightness());
		if ((layers & MiniMapLayer::Owner) != 0) {
			color = darken_color(color, 40);
		}
	}

	if ((layers & MiniMapLayer::Owner) != 0) {
		if (0 < owner) {
			color = blend_color(color, brighten_color(egbase.player(owner).get_playercolor(), 30));
		}
	}

	const RGBColor contrast_color = invert_color(color);

	if (see_details) {
		// if ownership layer is displayed, it creates enough contrast to
		// visualize objects using white color.

		if (((layers & (MiniMapLayer::Road | MiniMapLayer::Flag | MiniMapLayer::Building |
		                MiniMapLayer::Artifacts)) != 0) &&
		    (f.field->get_immovable() != nullptr)) {
			bool high_traffic = false;
			const Widelands::MapObjectType type = f.field->get_immovable()->descr().type();
			if ((layers & MiniMapLayer::Flag) != 0 && type == Widelands::MapObjectType::FLAG) {
				upcast(Widelands::Flag, flag, f.field->get_immovable());
				color = blend_color(kWhite, egbase.player(owner).get_playercolor());
				if (flag->current_wares() > 5 &&
				    (player == nullptr ||
				     flag->get_owner()->player_number() == player->player_number())) {
					high_traffic = true;
				}
			} else if ((layers & MiniMapLayer::Building) != 0 &&
			           type >= Widelands::MapObjectType::BUILDING) {
				color = blend_color(kWhite, egbase.player(owner).get_playercolor());
			} else if (((layers & MiniMapLayer::Road) != 0) &&
			           type == Widelands::MapObjectType::WATERWAY) {
				color = kRoad;
			} else if (((layers & MiniMapLayer::Road) != 0) &&
			           type == Widelands::MapObjectType::ROAD) {
				color = kRoad;
				upcast(Widelands::Road, road, f.field->get_immovable());
				if (road->is_busy() && (player == nullptr || road->get_owner()->player_number() ==
				                                                player->player_number())) {
					high_traffic = true;
				}
			} else if ((layers & MiniMapLayer::Artifacts) != 0) {
				Widelands::FCoords coord[7];
				coord[0] = f;
				map.get_ln(f, &coord[1]);
				map.get_tln(f, &coord[2]);
				map.get_trn(f, &coord[3]);
				map.get_rn(f, &coord[4]);
				map.get_brn(f, &coord[5]);
				map.get_bln(f, &coord[6]);
				for (const Widelands::FCoords& fc : coord) {
					if (fc.field->get_immovable() != nullptr &&
					    fc.field->get_immovable()->descr().has_attribute(
					       Widelands::MapObjectDescr::get_attribute_id("artifact"))) {
						color = kRed;
						break;
					}
				}
			}

			if (high_traffic && (layers & MiniMapLayer::Traffic) != 0) {
				color = contrast_color;
			}
		}

		if (((layers & (MiniMapLayer::Ship | MiniMapLayer::Attack)) != 0) &&
		    (f.field->get_first_bob() != nullptr)) {
			for (Widelands::Bob* bob = f.field->get_first_bob(); bob != nullptr;
			     bob = bob->get_next_bob()) {
				if ((layers & MiniMapLayer::Ship) != 0 &&
				    bob->descr().type() == Widelands::MapObjectType::SHIP) {
					color = kWhite;
					break;
				}
				if ((layers & MiniMapLayer::Attack) != 0 &&
				    bob->descr().type() == Widelands::MapObjectType::SOLDIER &&
				    dynamic_cast<Widelands::Soldier*>(bob)->is_on_battlefield()) {
					uint32_t now = SDL_GetTicks();
					now /= 500;
					if (now % 2 == 0) {
						color = kRed;
					} else {
						color = invert_color(kRed);
					}
					break;
				}
			}
		}

		if ((layers & MiniMapLayer::StartingPositions) != 0) {
			Widelands::Coords starting_pos;
			for (uint32_t p = 1; p <= map.get_nrplayers(); p++) {
				starting_pos = map.get_starting_pos(p);
				if (!static_cast<bool>(starting_pos)) {
					continue;
				}
				uint32_t dist = map.calc_distance(f, starting_pos);
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
void do_draw_minimap(Texture& texture,
                     const Widelands::EditorGameBase& egbase,
                     const Widelands::Player* const player,
                     const Vector2i& top_left,
                     const MiniMapLayer layers,
                     const bool draw_full = true,
                     uint16_t* const rows_drawn = nullptr) {
	const Widelands::Map& map = egbase.map();
	const uint8_t scale = scale_map(map, (layers & MiniMapLayer::Zoom2) != 0);
	const uint16_t map_h = map.get_height();
	const uint16_t map_w = map.get_width();

	uint16_t start_row;
	uint16_t end_row;
	if (draw_full) {
		start_row = 0;
		end_row = map_h;
	} else {
		// Number of fields to update per frame. Feel free to tweak this.
		const uint32_t fields_per_frame = 128 * 256 / scale;

		assert(rows_drawn != nullptr);
		start_row = *rows_drawn < map_h ? *rows_drawn : 0;
		end_row = std::min(static_cast<uint16_t>(start_row + fields_per_frame / map_w), map_h);
		*rows_drawn = end_row;
	}

	for (uint16_t y = start_row; y < end_row; ++y) {
		for (uint16_t x = 0; x < map_w; ++x) {
			Widelands::Coords coords(Widelands::Coords(top_left.x + x, top_left.y + y));
			map.normalize_coords(coords);
			Widelands::FCoords f = map.get_fcoords(coords);
			Widelands::MapIndex i = Widelands::Map::get_index(f, map_w);
			move_r(map_w, f, i);

			Widelands::VisibleState vision;
			Widelands::PlayerNumber owner;
			if (player == nullptr || player->see_all()) {
				// This player has omnivision - show the field like it is in reality.
				vision = Widelands::VisibleState::kVisible;  // Seen right now.
				owner = f.field->get_owned_by();
			} else {
				// This player might be affected by fog of war - instead of the
				// reality, we show her what she last saw on this field. If she has
				// vision of this field, this will be the same as reality -
				// otherwise this shows reality as it was the last time she had
				// vision on the field.
				// If she never had vision, field.vision will be kUnexplored.
				const auto& field = player->fields()[i];
				vision = field.vision;
				owner = field.owner;
			}

			if (vision != Widelands::VisibleState::kUnexplored) {
				const RGBAColor color = calc_minimap_color(
				   egbase, f, layers, owner, player, vision == Widelands::VisibleState::kVisible);
				for (uint8_t x_offset = 0; x_offset < scale; ++x_offset) {
					for (uint8_t y_offset = 0; y_offset < scale; ++y_offset) {
						texture.set_pixel(x * scale + x_offset, y * scale + y_offset, color);
					}
				}
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
                                      const Widelands::Player* const player,
                                      const Rectf& view_area,
                                      const MiniMapType& minimap_type,
                                      const MiniMapLayer layers) {
	std::unique_ptr<Texture> texture_static = create_minimap_empty(egbase, layers);
	draw_minimap_static(*texture_static, egbase, player, layers);
	return draw_minimap_final(*texture_static, egbase, view_area, minimap_type, layers);
}

std::unique_ptr<Texture> create_minimap_empty(const Widelands::EditorGameBase& egbase,
                                              const MiniMapLayer layers) {
	const Widelands::Map& map = egbase.map();
	const int8_t scale = scale_map(map, (layers & MiniMapLayer::Zoom2) != 0);
	const int16_t minimap_w = map.get_width() * scale;
	const int16_t minimap_h = map.get_height() * scale;

	std::unique_ptr<Texture> texture(new Texture(minimap_w, minimap_h));
	texture->fill_rect(Rectf(0.f, 0.f, minimap_w, minimap_h), RGBAColor(0, 0, 0, 255));
	return texture;
}

void draw_minimap_static(Texture& texture,
                         const Widelands::EditorGameBase& egbase,
                         const Widelands::Player* player,
                         const MiniMapLayer layers,
                         const bool draw_full,
                         uint16_t* const rows_drawn) {
	NoteThreadSafeFunction::instantiate(
	   [&texture, &egbase, player, layers, draw_full, rows_drawn]() {
		   const Widelands::Map& map = egbase.map();
		   const Widelands::Coords node =
		      MapviewPixelFunctions::calc_node_and_triangle(map, 0, 0).node;

		   texture.lock();
		   do_draw_minimap(
		      texture, egbase, player, Vector2i(node.x, node.y), layers, draw_full, rows_drawn);
		   texture.unlock(Texture::Unlock_Update);
	   },
	   true);
}

std::unique_ptr<Texture> draw_minimap_final(const Texture& input_texture,
                                            const Widelands::EditorGameBase& egbase,
                                            const Rectf& view_area,
                                            const MiniMapType& minimap_type,
                                            const MiniMapLayer layers) {
	const Widelands::Map& map = egbase.map();
	const bool zoom = (layers & MiniMapLayer::Zoom2) != 0;
	const int8_t scale = scale_map(map, zoom);
	const int16_t minimap_w = map.get_width() * scale;
	const int16_t minimap_h = map.get_height() * scale;

	std::unique_ptr<Texture> texture(new Texture(minimap_w, minimap_h));

	// Center the view on the middle of the 'view_area'.
	switch (minimap_type) {
	case MiniMapType::kStaticViewWindow: {
		const uint16_t move_x =
		   static_cast<uint16_t>(view_area.center().x * scale / kTriangleWidth + minimap_w / 2.f) %
		   minimap_w;
		const uint16_t move_y =
		   static_cast<uint16_t>(view_area.center().y * scale / kTriangleHeight + minimap_h / 2.f) %
		   minimap_h;

		// Assemble the output texture from 4 parts of the input texture.
		// Bottom-right -> top-left
		texture->blit(Rectf(0.f, 0.f, minimap_w - move_x, minimap_h - move_y), input_texture,
		              Rectf(move_x, move_y, minimap_w - move_x, minimap_h - move_y), 1.,
		              BlendMode::Copy);
		// Bottom-left -> top-right
		texture->blit(Rectf(minimap_w - move_x, 0.f, move_x, minimap_h - move_y), input_texture,
		              Rectf(0.f, move_y, move_x, minimap_h - move_y), 1., BlendMode::Copy);
		// Top-right -> bottom-left
		texture->blit(Rectf(0.f, minimap_h - move_y, minimap_w - move_x, move_y), input_texture,
		              Rectf(move_x, 0.f, minimap_w - move_x, move_y), 1., BlendMode::Copy);
		// Top-left -> bottom-right
		texture->blit(Rectf(minimap_w - move_x, minimap_h - move_y, move_x, move_y), input_texture,
		              Rectf(0.f, 0.f, move_x, move_y), 1., BlendMode::Copy);
		break;
	}

	case MiniMapType::kStaticMap:
		texture->blit(Rectf(0.f, 0.f, minimap_w, minimap_h), input_texture,
		              Rectf(0.f, 0.f, minimap_w, minimap_h), 1., BlendMode::Copy);
		break;
	}

	if ((layers & MiniMapLayer::ViewWindow) != 0) {
		texture->lock();
		draw_view_window(map, view_area, minimap_type, zoom, texture.get());
		texture->unlock(Texture::Unlock_Update);
	}

	return texture;
}

int scale_map(const Widelands::Map& map, bool zoom) {
	// The MiniMap can have a maximum size of 600px (width or height).
	// Borders and button height takes 2x20px + 20px. -> 60px
	// If a map width or height is greater than 270px we don't scale.
	// Otherwise we fit as much as possible into a 300px or 400px
	// MiniMap window when zoom is disabled.
	const auto max = std::max(map.get_width(), map.get_height());
	if (max <= 270) {
		if (zoom) {
			return 540 / max;
		}
		if (max > 135) {
			return 370 / max;
		}
		return 270 / max;
	}
	return 1;
}
