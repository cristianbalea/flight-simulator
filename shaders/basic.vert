#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;


out vec3 fNormal;
out vec3 fPosition;
out vec4 fragPosEye;
out vec4 fragPosLightSpace;
out vec2 fTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceTrMatrix;
uniform mat3 normalMatrix;

void main() 
{
	fragPosEye = view * model * vec4(vPosition, 1.0f);
	fNormal = vNormal;
	fTexCoords = vTexCoords;
	fPosition = vPosition;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);	
}
