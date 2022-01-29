/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "graphic/gl/workarea_program.h"

#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "wui/mapviewpixelconstants.h"

namespace std {

// This hash function lets us use TCoords as keys in std::unordered_map.
template <> struct hash<Widelands::TCoords<>> {
	std::size_t operator()(const Widelands::TCoords<>& key) const {
		return (static_cast<std::size_t>(key.node.hash()) << 1) ^ static_cast<std::size_t>(key.t);
	}
};

}  // namespace std

WorkareaProgram::WorkareaProgram() : cache_(nullptr) {
	gl_program_.build("workarea");

	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_overlay_ = glGetAttribLocation(gl_program_.object(), "attr_overlay");

	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
}

void WorkareaProgram::gl_draw(int gl_texture, float z_value) {
	glUseProgram(gl_program_.object());

	{
		auto& gl_state = Gl::State::instance();
		gl_state.enable_vertex_attrib_array({attr_position_, attr_overlay_});
		gl_array_buffer_.bind();
		gl_array_buffer_.update(vertices_);
		Gl::vertex_attrib_pointer(
		   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
		Gl::vertex_attrib_pointer(
		   attr_overlay_, 4, sizeof(PerVertexData), offsetof(PerVertexData, overlay_r));
		gl_state.bind(GL_TEXTURE0, gl_texture);
		glUniform1f(u_z_value_, z_value);
		glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
	}
	{
		auto& gl_state = Gl::State::instance();
		gl_state.enable_vertex_attrib_array({attr_position_, attr_overlay_});
		gl_array_buffer_.bind();
		gl_array_buffer_.update(outer_vertices_);
		Gl::vertex_attrib_pointer(
		   attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
		Gl::vertex_attrib_pointer(
		   attr_overlay_, 4, sizeof(PerVertexData), offsetof(PerVertexData, overlay_r));
		gl_state.bind(GL_TEXTURE0, gl_texture);
		glUniform1f(u_z_value_, z_value);
		glDrawArrays(GL_TRIANGLES, 0, outer_vertices_.size());
	}
}

constexpr uint8_t kWorkareaTransparency = 127;
static RGBAColor workarea_colors[]  // Comment to prevent clang-format from breaking codecheck
   {
      RGBAColor(63, 31, 127, kWorkareaTransparency),  // All three circles
      RGBAColor(127, 63, 0, kWorkareaTransparency),   // Medium and outer circle
      RGBAColor(0, 127, 0, kWorkareaTransparency),    // Outer circle
      RGBAColor(63, 0, 127, kWorkareaTransparency),   // Inner and medium circle
      RGBAColor(127, 0, 0, kWorkareaTransparency),    // Medium circle
      RGBAColor(0, 0, 127, kWorkareaTransparency),    // Inner circle
      RGBAColor(0, 0, 0, 0),
   };
static inline RGBAColor apply_color(RGBAColor c1, RGBAColor c2) {
	if (c1.a == 0 && c2.a == 0) {
		return RGBAColor((c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2, 0);
	}
	uint8_t r = (c1.r * c1.a + c2.r * c2.a) / (c1.a + c2.a);
	uint8_t g = (c1.g * c1.a + c2.g * c2.a) / (c1.a + c2.a);
	uint8_t b = (c1.b * c1.a + c2.b * c2.a) / (c1.a + c2.a);
	uint8_t a = (c1.a + c2.a) / 2;
	return RGBAColor(r, g, b, a);
}
static inline RGBAColor apply_color_special(RGBAColor base, RGBAColor special) {
	uint8_t r = (base.r * base.a + special.r * 255) / (base.a + 255);
	uint8_t g = (base.g * base.a + special.g * 255) / (base.a + 255);
	uint8_t b = (base.b * base.a + special.b * 255) / (base.a + 255);
	return RGBAColor(r, g, b, special.a);
}

void WorkareaProgram::add_vertex(const FieldsToDraw::Field& field,
                                 RGBAColor overlay,
                                 std::vector<PerVertexData>* v,
                                 Vector2f offset,
                                 Vector2f viewport) {
	v->emplace_back();
	PerVertexData& back = v->back();

	if (offset.x > 0 || offset.x < 0 || offset.y > 0 || offset.y < 0) {
		back.gl_x = field.surface_pixel.x + offset.x;
		back.gl_y = field.surface_pixel.y + offset.y;
		pixel_to_gl_renderbuffer(viewport.x, viewport.y, &back.gl_x, &back.gl_y);
	} else {
		back.gl_x = field.gl_position.x;
		back.gl_y = field.gl_position.y;
	}
	back.overlay_r = overlay.r / 255.f;
	back.overlay_g = overlay.g / 255.f;
	back.overlay_b = overlay.b / 255.f;
	back.overlay_a = overlay.a / 255.f;
}

constexpr float kBorderStrength = 2.8f;

// Helper functions for calculating the border thickness
const static float kOffsetFactor = static_cast<float>(
   std::sqrt(kBorderStrength * kBorderStrength /
             (kTriangleWidth * kTriangleWidth + kTriangleHeight * kTriangleHeight)));
static Vector2f offset(size_t radius, size_t pos) {
	if (pos % radius == 0) {
		switch (pos / radius) {
		case 0:  // North/Northwest
			return Vector2f(-kTriangleWidth * (2 * kOffsetFactor - kBorderStrength / kTriangleHeight),
			                -kBorderStrength);
		case 1:  // North/Northeast
			return Vector2f(kTriangleWidth * (2 * kOffsetFactor - kBorderStrength / kTriangleHeight),
			                -kBorderStrength);
		case 2:  // Northeast/Southeast
			return Vector2f(kBorderStrength, 0);
		case 3:  // Southeast/South
			return Vector2f(kTriangleWidth * (2 * kOffsetFactor - kBorderStrength / kTriangleHeight),
			                kBorderStrength);
		case 4:  // South/Southwest
			return Vector2f(-kTriangleWidth * (2 * kOffsetFactor - kBorderStrength / kTriangleHeight),
			                kBorderStrength);
		case 5:  // Southwest/Northwest
			return Vector2f(-kBorderStrength, 0);
		default:
			NEVER_HERE();
		}
	} else {
		switch (pos / radius) {
		case 0:  // North
			return Vector2f(0, -kBorderStrength);
		case 1:  // Northeast
			return Vector2f(kOffsetFactor * kTriangleWidth, -kOffsetFactor * kTriangleHeight);
		case 2:  // Southeast
			return Vector2f(kOffsetFactor * kTriangleWidth, kOffsetFactor * kTriangleHeight);
		case 3:  // South
			return Vector2f(0, kBorderStrength);
		case 4:  // Southwest
			return Vector2f(-kOffsetFactor * kTriangleWidth, kOffsetFactor * kTriangleHeight);
		case 5:  // Northwest
			return Vector2f(-kOffsetFactor * kTriangleWidth, -kOffsetFactor * kTriangleHeight);
		default:
			NEVER_HERE();
		}
	}
}

void WorkareaProgram::draw(uint32_t texture_id,
                           Workareas workarea,
                           const FieldsToDraw& fields_to_draw,
                           float z_value,
                           Vector2f rendertarget_dimension) {
	const FieldsToDraw::Field& topleft = fields_to_draw.at(0);
	if (cache_ && cache_->fcoords == topleft.fcoords &&
	    !(cache_->surface_pixel.x > topleft.surface_pixel.x ||
	      cache_->surface_pixel.x < topleft.surface_pixel.x ||
	      cache_->surface_pixel.y > topleft.surface_pixel.y ||
	      cache_->surface_pixel.y < topleft.surface_pixel.y) &&
	    cache_->workareas == workarea) {
		return gl_draw(texture_id, z_value);
	}
	cache_.reset(new WorkareasCache(workarea, topleft.fcoords, topleft.surface_pixel));

	vertices_.clear();
	outer_vertices_.clear();
	{
		size_t estimate_inner = 0;
		size_t estimate_outer = 0;
		for (const WorkareasEntry& wa_map : workarea) {
			estimate_inner += 3 * wa_map.first.size();  // One triangle per entry
			for (const auto& vector : wa_map.second) {
				estimate_outer += 6 * vector.size();  // Two triangles per border segment
			}
		}
		vertices_.reserve(estimate_inner);
		outer_vertices_.reserve(estimate_outer);
	}

	std::unordered_map<Widelands::TCoords<>, RGBAColor> triangle_colors{};
	for (const WorkareasEntry& wa_map : workarea) {
		for (const WorkareaPreviewData& data : wa_map.first) {
			RGBAColor color_to_apply = workarea_colors[data.index];
			if (data.use_special_coloring) {
				color_to_apply = apply_color_special(color_to_apply, RGBAColor(data.special_coloring));
			}
			triangle_colors[data.coords] = apply_color(triangle_colors[data.coords], color_to_apply);
		}
	}

	auto emplace_triangle = [this, workarea, fields_to_draw, triangle_colors](
	                           const FieldsToDraw::Field& field,
	                           Widelands::TriangleIndex triangle_index) {
		RGBAColor color(0, 0, 0, 0);
		if (triangle_colors.count(Widelands::TCoords<>(field.fcoords, triangle_index))) {
			color = triangle_colors.at(Widelands::TCoords<>(field.fcoords, triangle_index));
		}
		if (color.a > 0) {
			add_vertex(field, color, &vertices_);
			add_vertex(fields_to_draw.at(field.brn_index), color, &vertices_);
			add_vertex(
			   fields_to_draw.at(triangle_index == Widelands::TriangleIndex::D ? field.bln_index :
                                                                              field.rn_index),
			   color, &vertices_);
		}
	};

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);
		if (field.brn_index != FieldsToDraw::kInvalidIndex) {
			if (field.bln_index != FieldsToDraw::kInvalidIndex) {
				emplace_triangle(field, Widelands::TriangleIndex::D);
			}
			if (field.rn_index != FieldsToDraw::kInvalidIndex) {
				emplace_triangle(field, Widelands::TriangleIndex::R);
			}
		}
	}

	{
		// Draw the border. Since a basic line is too narrow to be properly visible,
		// we draw two triangles to give the line some thickness.
		for (const WorkareasEntry& wa_map : workarea) {
			int32_t index = 5;
			for (const auto& border : wa_map.second) {
				assert(index == 5 || index == 4 || index == 2);
				RGBAColor& color = workarea_colors[index];
				const size_t nr_border_coords = border.size();
				const size_t radius = nr_border_coords / 6;
				assert(radius * 6 == nr_border_coords);
				size_t border_pos = 0;
				for (auto it = border.begin(); it != border.end(); ++it, ++border_pos) {
					int f1 = fields_to_draw.calculate_index(it->x, it->y);
					if (f1 == FieldsToDraw::kInvalidIndex) {
						continue;
					}
					int f2;
					if (it + 1 == border.end()) {
						f2 = fields_to_draw.calculate_index(border.begin()->x, border.begin()->y);
					} else {
						f2 = fields_to_draw.calculate_index((it + 1)->x, (it + 1)->y);
					}
					if (f2 != FieldsToDraw::kInvalidIndex) {
						const FieldsToDraw::Field& field1 = fields_to_draw.at(f1);
						const FieldsToDraw::Field& field2 = fields_to_draw.at(f2);
						Vector2f off1 = offset(radius, border_pos);
						Vector2f off2 = offset(radius, (border_pos + 1) % nr_border_coords);
						add_vertex(field1, color, &outer_vertices_);
						add_vertex(field1, color, &outer_vertices_, off1, rendertarget_dimension);
						add_vertex(field2, color, &outer_vertices_, off2, rendertarget_dimension);
						add_vertex(field1, color, &outer_vertices_);
						add_vertex(field2, color, &outer_vertices_);
						add_vertex(field2, color, &outer_vertices_, off2, rendertarget_dimension);
					}
				}
				switch (index) {
				case 5:
					index = 4;
					break;
				case 4:
					index = 2;
					break;
				default:
					index = -1;
					break;
				}
			}
		}
	}

	gl_draw(texture_id, z_value);
}
