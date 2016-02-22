#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textureCoordinates;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 shadowMatrix;
uniform mat4 normalMatrix;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;

out vec2 o_textureCoordinates;
out vec3 normalVec, lightVec, eyeVec;
out vec4 shadowCoord;

void main() {
	vec4 worldVertex = M*vec4(vertex,1.0);

	normalVec = normalize(vec3(normalMatrix*vec4(normal,0.0)));    
    lightVec = lightPosition - worldVertex.xyz;

	eyeVec=cameraPosition-worldVertex.xyz;
	
	shadowCoord=shadowMatrix*worldVertex;
	
	o_textureCoordinates=textureCoordinates;
	
	gl_Position=P*V*worldVertex;

} 



