#version 330 core

// Ouput data
out vec4 color;

in vec2 o_uv0;

uniform sampler2D colorTexture;

void main(){

        color = texture(colorTexture, o_uv0);
        //color = vec4(o_uv0, 0, 1);
        //color = vec4(0.2, 0.2, 0.4, 1);

}
