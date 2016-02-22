#version 330

uniform float ds_LineWidth = 5.0;


layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

void main()
{
    vec3 ndc0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 ndc1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;

    vec2 lineScreenForward = normalize(ndc1.xy - ndc0.xy);
    vec2 lineScreenRight = vec2(-lineScreenForward.y, lineScreenForward.x);
    vec2 lineScreenOffset = (vec2(ds_LineWidth)/vec2(800,600)) * lineScreenRight;
	
    gl_Position = vec4(gl_in[0].gl_Position.xy + lineScreenOffset, gl_in[0].gl_Position.z, gl_in[0].gl_Position.w);
    EmitVertex();

    gl_Position = vec4(gl_in[0].gl_Position.xy - lineScreenOffset, gl_in[0].gl_Position.z, gl_in[0].gl_Position.w);
    EmitVertex();

    gl_Position = vec4(gl_in[1].gl_Position.xy + lineScreenOffset, gl_in[1].gl_Position.z, gl_in[1].gl_Position.w);
    EmitVertex();

    gl_Position = vec4(gl_in[1].gl_Position.xy - lineScreenOffset, gl_in[1].gl_Position.z, gl_in[1].gl_Position.w);
    EmitVertex();

    EndPrimitive();
}