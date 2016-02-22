#version 330

out vec4 FragColor;
uniform float red;
void main()
{                                    
     FragColor = vec4(red,0.5,0.5,0.8);
}
