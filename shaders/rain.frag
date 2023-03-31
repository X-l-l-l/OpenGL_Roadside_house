#version 410 core

in vec2 fTexCoords;

out vec4 fColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main()
{
    if (texture(diffuseTexture, fTexCoords).a <0.1)
            discard;

    //compute final vertex color
    vec3 color = min(0.5f*texture(diffuseTexture, fTexCoords).rgb + texture(specularTexture, fTexCoords).rgb, 1.0f);

    fColor = vec4(color, 1.0f);
}
