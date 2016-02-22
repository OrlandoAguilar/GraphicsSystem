#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                                                 

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{       
  
  vec4 pos=vec4(Position,1);
  pos.y+=0.5;
  gl_Position  = P*V* M* pos;
                              
}
