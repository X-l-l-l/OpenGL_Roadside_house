#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fPositionLight;
out vec4 fPosition;
out vec4 fPositionEye;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 windDir;
uniform float wind;
uniform mat4 lightSpaceTrMatrix;

void main()
{
    vec3 windGen = vec3(windDir.x*wind,0.0f,windDir.y*wind);

    if (vPosition.y>0 && vPosition.y<4.5f)
	    gl_Position = projection * view * model * vec4(vPosition+windGen, 1.0f);
	else if (vPosition.y>=5 && vPosition.y<10)
	    gl_Position = projection * view * model * vec4(vPosition+windGen*1.5, 1.0f);
	else if (vPosition.y>=10 && vPosition.y<20)
    	    gl_Position = projection * view * model * vec4(vPosition+windGen*2, 1.0f);
    else if (vPosition.y>=20)
        	    gl_Position = projection * view * model * vec4(vPosition+windGen*2.5, 1.0f);
    else
    	gl_Position = projection * view * model * vec4(vPosition, 1.0f);


	fPositionEye = view * model * vec4(vPosition, 1.0f);
    fNormal = vNormal;
    fTexCoords = vTexCoords;
    fPositionLight = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
    fPosition = model * vec4(vPosition, 1.0f);
}
