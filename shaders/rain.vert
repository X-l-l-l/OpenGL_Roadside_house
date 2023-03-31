#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec2 fTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float fall;

void main()
{
    vec3 poz = vec3(0.0f,fall,0.0f);

	gl_Position = projection * view * model * vec4(vPosition-poz, 1.0f);
	fTexCoords = vTexCoords;
}
