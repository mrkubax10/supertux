#version 450

layout(location = 0) in vec2 texcoord;
layout(location = 1) in vec2 position;
layout(location = 2) in vec4 diffuse;

layout(location = 0) out vec2 texcoord_var;
layout(location = 1) out vec4 diffuse_var;

uniform mat3 modelviewprojection;

void main(void)
{
  texcoord_var = texcoord;
  diffuse_var = diffuse;
  gl_Position = vec4(vec3(position, 1) * modelviewprojection, 1.0);
}

/* EOF */
