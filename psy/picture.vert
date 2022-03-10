#version 330 core
layout (location = 0) in vec3 pos; 	// the position variable has attribute position 0
layout (location = 1) in vec4 color;	// the vertex color
layout (location = 2) in vec2 tex_pos;	// the texture position variable has attribute position 2
  
out vec4 vertex_Color;          // specify a color output to the fragment shader
out vec2 texture_Coordinate;    // specify a texture coordinate to the fragment shader

void main()
{
    gl_Position = vec4(pos, 1.0); // see how we directly give a vec3 to vec4's constructor
    vertex_Color = color; // set the output variable to a dark-red color
    texture_Coordinate = tex_pos;
}
