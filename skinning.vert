#version 330 core                                                                  
  
layout (location = 0) in vec3 vertex;                                                                 
layout (location = 1) in vec3 Normal;   
layout (location = 2) in vec2 TexCoord;                                             
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec2 TexCoord0;
out vec3 Normal0;                                                                   
out vec3 lightVec;
out vec3 eyeVec;
out vec4 shadowCoord;

const int MAX_BONES = 100;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform mat4 gBones[MAX_BONES];
uniform mat4 shadowMatrix;

void main()
{       
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

    vec4 PosL    = BoneTransform * vec4(vertex, 1.0);
	vec4 worldVertex    = M* PosL;
	
    gl_Position  = P*V* worldVertex;
    TexCoord0    = TexCoord;
    vec4 NormalL = BoneTransform * vec4(Normal, 0.0);
    Normal0      = (M* NormalL).xyz;
    
	shadowCoord=shadowMatrix*worldVertex;

    lightVec = lightPosition - worldVertex.xyz;
	eyeVec = cameraPosition-worldVertex.xyz;
	
}
