#version 330 core
layout (location = 0) in vec3 pos; 	// the position variable has attribute position 0
layout (location = 1) in vec4 color;	// the vertex color
layout (location = 2) in vec2 tex_pos;	// the texture position variable has attribute position 2

uniform mat4 projection;
uniform mat4 model;

out vec4 vertex_Color;          // specify a color output to the fragment shader
out vec2 texture_Coordinate;    // specify a texture coordinate to the fragment shader

void main()
{
    vec4 vertex = vec4(pos, 1.0); // see how we directly give a vec3 to vec4's constructor
    gl_Position = projection * model * vertex;
    vertex_Color = color; // set the output variable to a dark-red color
    texture_Coordinate = tex_pos;
}
