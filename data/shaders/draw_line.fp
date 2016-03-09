#version 120

varying vec4 var_color;

#define PI 3.141592653589793

void main() {
	// Empirically found shading function that got consensus.
	float alpha = pow(cos(var_color.a * PI / 2), 1.5);
	gl_FragColor = vec4(var_color.r, var_color.g, var_color.b, alpha);
}
