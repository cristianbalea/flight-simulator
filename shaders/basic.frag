#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform float carPosition;
uniform int headlights;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec4 fPosEye;

uniform int isFog;
uniform float fogDensity;
uniform samplerCube skybox;

in vec3 color;


void computeDirLight()
{
    vec3 cameraPosEye = vec3(0.0f);

    ambient = ambientStrength * lightColor;

    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    
    vec3 normalEye = normalize(normalMatrix * fNormal);

    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor; 

    vec3 viewDir = normalize(-fPosEye.xyz);
    vec3 viewDirN = normalize(viewDir);

    vec3 reflectDir = reflect(-lightDirN, normalEye);

    vec3 halfVector = normalize(lightDirN + viewDirN);

    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

void computePointLight(vec3 lightPosition, vec3 lightCol, float constant, float linear, float quadratic) {
    float dist = length(lightPosition - fPosition);
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

    diffuse += att * diffuse * lightCol;
    ambient += att * ambient * lightCol;
    specular += att * specular * lightCol;
}

void computeSpotLight(vec3 lightPosition, vec3 spotlightDir, float constant, 
                                float linear, float quadratic, float cutOff, float outerCutOff) {
    vec3 lightDirection = normalize(lightPosition - fPosition);

    float distance = length(lightPosition - fPosition);
    float att = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    
    float theta = dot(lightDirection, spotlightDir);
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0f, 1.0f);

    diffuse += att * intensity * diffuse * vec3(1.0f, 1.0f, 0.0f);
    ambient += att * intensity * specular * vec3(1.0f, 1.0f, 0.0f);
    specular += att * intensity * specular * vec3(1.0f, 1.0f, 0.0f);
}

float computeFog() { 
    float fragmentDistance = length(fragPosEye); 
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2)); 
    return clamp(fogFactor, 0.0f, 1.0f); 
}

void main() {
    computeDirLight();

    vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
	if(colorFromTexture.a < 0.1)
		discard;

    ambient *= colorFromTexture.rgb;
    diffuse *= colorFromTexture.rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    computePointLight(vec3(-474.99f, 18.0f, 508.68f), vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.0045f, 0.0075f);
    computePointLight(vec3(-473.82f, 18.0f, 638.94f), vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.0045f, 0.0075f);
    computePointLight(vec3(-592.64f, 18.0f, 508.68f), vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.0045f, 0.0075f);
    computePointLight(vec3(-592.64f, 18.0f, 639.56f), vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.0045f, 0.0075f);

    if(headlights == 1)
        computeSpotLight(vec3(4*carPosition - 12.0f, 10.0f, -146.0f), vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.0045f, 0.0075f, 0.97f, 0.93f);
    
    vec3 color = min((ambient + diffuse) + specular, 1.0f);
	
    float fogFactor = computeFog(); 
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f); 

    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}