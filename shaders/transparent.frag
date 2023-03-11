#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosEye;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

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

uniform float reflectFactor;

void computeDirLight()
{
    //compute ambient light
    ambient = ambientStrength * lightColor;

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor; 

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(-fPosEye.xyz);

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

float computeFog() { 
    float fragmentDistance = length(fragPosEye); 
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2)); 
    return clamp(fogFactor, 0.0f, 1.0f); 
}

void main() {
    computeDirLight();

    //compute final vertex color
    //vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);

    //fColor = vec4(color, 1.0f);
	
	vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
	if(colorFromTexture.a < 0.1)
		discard;
	
    float fogFactor = computeFog(); 
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f); 

    //fColor = mix(fogColor, colorFromTexture, fogFactor);




    vec3 viewDirectionN = normalize(-fPosition);
    vec3 normalN = normalize(fNormal);
    vec3 reflection = reflect(viewDirectionN, normalN);
    vec3 colorFromSkybox = vec3(texture(skybox, reflection));

    vec4 aux = mix(vec4(colorFromSkybox, 1.0f),colorFromTexture, reflectFactor);
    fColor = vec4(colorFromSkybox, 0.5f);
    
    //fColor = mix(fogColor, aux, fogFactor);
}
