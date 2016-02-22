#version 330

layout(location = 0) in vec3 vertex;

uniform mat4 V;
uniform mat4 P;

void main()
{    
    gl_Position = P*V*vec4(vertex,1.0);
}


