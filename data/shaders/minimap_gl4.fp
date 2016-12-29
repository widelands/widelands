#version 130
#extension GL_ARB_uniform_buffer_object: enable

// Varyings
in vec2 var_field;

// Uniforms
uniform bool u_layer_terrain;
uniform bool u_layer_owner;
uniform uint u_layer_details;

uniform vec2 u_frame_topleft;
uniform vec2 u_frame_bottomright;
uniform vec2 u_frame_width;

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

// Return true if a and b are closer than threshold modulo 1.
bool wrap_close(float a, float b, float threshold) {
	float dist = a - b;
	dist -= floor(dist + 0.5);
	return abs(dist) < threshold;
}

void main() {
	float player_brightness = texture(u_player_brightness, var_field).r;

	// Determine whether we're on the frame
	bool on_frame = false;
	float low, high, pix, width;

	if (wrap_close(var_field.x, u_frame_topleft.x, u_frame_width.x) ||
	    wrap_close(var_field.x, u_frame_bottomright.x, u_frame_width.x)) {
		on_frame = true;
		low = u_frame_topleft.y;
		high = u_frame_bottomright.y;
		pix = var_field.y;
		width = u_frame_width.y;
	} else if (wrap_close(var_field.y, u_frame_topleft.y, u_frame_width.y) ||
	           wrap_close(var_field.y, u_frame_bottomright.y, u_frame_width.y)) {
		on_frame = true;
		low = u_frame_topleft.x;
		high = u_frame_bottomright.x;
		pix = var_field.x;
		width = u_frame_width.x;
	}

	if (on_frame) {
		pix -= floor(pix - low); // Normalize to range [low, low + 1)
		if (pix <= high &&
		    (int((pix - low) / (2 * width)) & 1) == 0) {
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
		}
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
