#version 120


/* layout(location = 0) out vec4 fragment_color; */

uniform sampler2D tex;

varying vec2 o_texture_position;

void main() {
	gl_FragColor = texture2D(tex, o_texture_position);
}
