// Useful: http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf

#version 120

attribute vec2 position;
attribute vec2 texture_position;
attribute float height;

varying vec2 o_texture_position;

#define HEIGHT_FACTOR 5

/* layout(location = 1) in float intensity; */

/* uniform float scale; */
/* uniform mat3 model_matrix; */

void main() {
	o_texture_position = texture_position;
	vec4 p = vec4(position, 0., 1.);
	p.y -= height * HEIGHT_FACTOR;
	gl_Position = gl_ProjectionMatrix * p;
}
