#shader vertex
#version 410 core

layout(location=0)in vec4 position;
layout(location=2)in vec2 texCoord;

out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



out vec2 v_TexCoord;

void main(){
    FragPos = vec3(model * position);
    gl_Position = projection * view * vec4(FragPos, 1.0);
    v_TexCoord =vec2(texCoord.x, 1-texCoord.y);
}

    #shader fragment
    #version 410 core

layout(location=0)out vec4 color;

in vec2 v_TexCoord;
uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main(){
    vec4 texColor = texture(u_Texture, v_TexCoord);
    color =texColor;
}