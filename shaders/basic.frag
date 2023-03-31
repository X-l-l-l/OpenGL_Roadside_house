#version 410 core

in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPositionLight;
in vec4 fPosition;
in vec4 fPositionEye;


out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform mat3 lightDirMatrix;
uniform vec3 pointLight1;
uniform vec3 pointLight2;
uniform vec3 pointLight3;
uniform vec3 viewPos;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 64.0f;

uniform int night;
uniform int fogOn;

void computeLightComponents()
{
    vec3 cameraPositionEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

    //transform normal
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //compute light direction
    vec3 lightDirN = normalize(lightDirMatrix * lightDir);

    //compute view direction
    vec3 viewDirN = normalize(cameraPositionEye - fPositionEye.xyz);

    //compute half vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{
    // perform perspective divide
    vec3 normalizedCoords = fPositionLight.xyz / fPositionLight.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.00005f;
    //float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;
    if (currentDepth - bias > closestDepth)
        return 1.0f;
    else
        return 0.0f;

}

float constant = 1.0f;
float pointLinear = 10.0f;
float pointQuadratic = 3.0f;
float pointSpecularStrength = 10.0f;

void computePointLight(vec3 posPoint)
{
    vec3 norm = normalize(normalMatrix * fNormal);
    vec3 lightDir = normalize(posPoint - fPosition.xyz);

    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);

    // specular shading
    vec3 viewDir = normalize(viewPos - fPosition.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // attenuation
    float distance    = length(posPoint - fPosition.xyz);
    float attenuation = 100.0 / (constant + pointLinear * distance + pointQuadratic * (distance * distance));

    // combine results
    ambient  += lightColor * texture(diffuseTexture, fTexCoords).rgb * attenuation;
    diffuse  += lightColor * diff * texture(diffuseTexture, fTexCoords).rgb * attenuation;
    specular += lightColor * spec * pointSpecularStrength * attenuation;
}

float computeFog()
{
 float fogDensity = 0.03f;
 float fragmentDistance = length(fPositionEye)*2;

 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
//float fogFactor = (fog_maxdist - fragmentDistance)/(fog_maxdist - fog_mindist);

 return clamp(fogFactor, 0.0f,1.0f);
}

void main() 
{
    if(night == 0)
        computeLightComponents();
    else
        {computePointLight(pointLight1);
        computePointLight(pointLight2);
        computePointLight(pointLight3);}

    float shadow = computeShadow();

    //compute final vertex color

    ambient *= vec3(texture(diffuseTexture, fTexCoords));
    diffuse *= vec3(texture(diffuseTexture, fTexCoords));
    specular *= vec3(texture(specularTexture, fTexCoords));

    vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    vec3 colorFinal = min(color, 1.0f);
    vec4 colorFinalFog = vec4(color,1.0f);
    fColor = vec4(colorFinal, 1.0f);
    if (fogOn==1){
        		float fogFactor = computeFog();
        		vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

        		fColor = fogColor*(1-fogFactor)+colorFinalFog*fogFactor;
        	}else{
        		fColor = vec4(colorFinal,1.0f);
        	}
}
