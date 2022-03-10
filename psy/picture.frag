#version 330 core

out vec4 FragColor;
  
in vec4 vertex_Color; // the input variable from the vertex shader (same name and same type)
in vec2 texture_Coordinate; // the input variable from the vertex shader (same name and same type)
uniform sampler2D pic_texture;

void main()
{
    FragColor = texture(pic_texture, texture_Coordinate);
}
