#version 130
#extension GL_ARB_uniform_buffer_object: enable

// From terrain_common_gl4:
void init_common();
uvec4 get_field_base(ivec2 coord);
float calc_brightness(ivec2 coord, uint node_ubrightness);
void calc_basepix(ivec2 coord, out vec2 basepix, out float heightpix, out uint node_brightness);
void calc_pix(ivec2 coord, out vec2 pix, out uint node_brightness);

// Varyings
in vec2 var_field;

// Uniforms
uniform bool u_layer_terrain;
uniform bool u_layer_owner;
uniform uint u_layer_details;

uniform ivec2 u_frame_topleft;
uniform ivec2 u_frame_bottomright;

// Textures (map data).
uniform usampler2D u_terrain_base;
uniform sampler2D u_player_brightness;
uniform usampler2D u_minimap_extra;
uniform sampler2D u_terrain_color;
uniform sampler2D u_player_color;

float calc_node_brightness(uint node_ubrightness) {
	// Brightness is really an 8-bit signed value, but it's stored in an
	// GL_RGBA8UI texture, so here we use signed (arithmetic) shifts to do
	// the conversion.
	int node_brightness = int(node_ubrightness << 24) >> 24;
	float brightness = 144. / 255. + node_brightness * (1. / 255.);
	brightness = min(1., brightness * (255. / 160.));
	return brightness;
}

void main() {
	float player_brightness = texture(u_player_brightness, var_field).r;

	// Determine whether we're on the frame
	ivec2 map_size = textureSize(u_terrain_base, 0);
	ivec2 fc = ivec2(var_field * map_size);
	int c_dist = -1;
	bool below_high = false;

	if (fc.x == u_frame_topleft.x || fc.x == u_frame_bottomright.x) {
		c_dist = fc.y - u_frame_topleft.y;
		below_high = fc.y <= u_frame_bottomright.y;
	} else if (fc.y == u_frame_topleft.y || fc.y == u_frame_bottomright.y) {
		c_dist = fc.x - u_frame_topleft.x;
		below_high = fc.x <= u_frame_bottomright.x;
	}

	if (below_high && c_dist >= 0 && (c_dist & 1) == 0) {
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	// Determine minimap color
	vec3 color = vec3(0.0, 0.0, 0.0);
	if (player_brightness > 0.0) {
		if (u_layer_terrain) {
			uvec4 node = texture(u_terrain_base, var_field);
			float brightness = calc_node_brightness(node.w);

			color = texelFetch(u_terrain_color, ivec2(node.y, 0), 0).rgb;
			color *= brightness;
		}

		if (u_layer_owner || u_layer_details != 0u) {
			uint extra = texture(u_minimap_extra, var_field).r;

			if (u_layer_owner) {
				uint owner = extra & 0x3fu;
				if (owner > 0u) {
					vec3 player_color = texelFetch(u_player_color, ivec2(owner - 1u, 0), 0).rgb;
					color = mix(color, player_color, 0.5);
				}
			}

			uint detail = extra >> 6u;
			if ((u_layer_details & 1u) != 0u && detail == 1u) {
				// Road
				color = mix(color, vec3(1, 1, 1), 0.5);
			} else if (detail != 0u && (u_layer_details & (1u << (detail - 1u))) != 0u) {
				// Flag or building
				color = vec3(1, 1, 1);
			}
		}
	}

	gl_FragColor.rgb = color;
	gl_FragColor.w = 1.0;
}
