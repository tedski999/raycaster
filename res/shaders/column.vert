#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex;

out vec4 vertex_color;

void main() {
	vertex_color = color;
	gl_Position = vec4(pos, 0.0, 1.0);
}

