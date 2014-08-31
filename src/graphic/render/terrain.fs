#version 120


/* layout(location = 0) out vec4 fragment_color; */

varying vec4 output_color;

void main() {
	gl_FragColor = output_color;
}
