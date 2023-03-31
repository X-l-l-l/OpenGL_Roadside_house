#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int night;

void main()
{
    color = texture(skybox, textureCoordinates);
    if (night == 1)
        color *= vec4(0.1f,0.1f,0.1f,1.0f);
}
