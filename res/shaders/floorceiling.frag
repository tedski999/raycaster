#version 330 core

in vec2 vertex_texture_coords;
out vec4 frag_color;

uniform sampler2D texture_sample;

void main() {
	frag_color = texture(texture_sample, vertex_texture_coords);
	//frag_color = vec4(1.0,0.0,0.0,1.0);
}

