// Useful: http://www.cs.unh.edu/~cs770/docs/glsl-1.20-quickref.pdf

#version 120

attribute vec2 position;
attribute vec3 color;

/* layout(location = 1) in float intensity; */

/* uniform float scale; */
/* uniform mat3 model_matrix; */
varying vec4 output_color;

void main() {
	output_color = vec4(color, 1.);
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(position, 0., 1.);
}
