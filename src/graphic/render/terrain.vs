// Useful: http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf

#version 120

attribute vec2 position;
attribute vec3 color;
attribute float height;

#define HEIGHT_FACTOR 5

/* layout(location = 1) in float intensity; */

/* uniform float scale; */
/* uniform mat3 model_matrix; */
varying vec4 output_color;

void main() {
	output_color = vec4(color, 1.);
	vec4 p = vec4(position, 0., 1.);
	p.y -= height * HEIGHT_FACTOR;
	gl_Position = gl_ProjectionMatrix * p;
}
