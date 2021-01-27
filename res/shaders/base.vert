#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;

out vec2 vertex_texture_coords;

void main() {
	vertex_texture_coords = tex;
	gl_Position = vec4(pos, 0.0, 1.0);
}

