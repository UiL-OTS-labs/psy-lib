#version 440 core

layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform mat4 projection;
uniform mat4 model;
  
void main()
{
    // see how we directly give a vec3 to vec4's constructor
    vec4 vertex = vec4(aPos, 1.0); 
    gl_Position = projection * model * vertex;
}
