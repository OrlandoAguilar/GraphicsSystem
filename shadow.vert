#version 330 core

layout(location = 0) in vec3 vertex;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

const int MAX_BONES = 100;

uniform mat4 PrVi;
uniform mat4 M;
uniform int numberBones;

uniform mat4 gBones[MAX_BONES];

out vec4 position;

void main()
{ 
	vec4 vert=vec4(vertex,1.0);
	if (numberBones>0){
		mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
		BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
		BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
		BoneTransform     += gBones[BoneIDs[3]] * Weights[3];
		vert=BoneTransform*vert;
	}
	
	vec4 pos=PrVi*M*vert;
	position=pos;
	gl_Position=pos;
}
