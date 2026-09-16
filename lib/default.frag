#version 330 core

// Inputs the color from the Vertex Shader
in vec4 fragColour;
in vec2 texCoord;

// Outputs colors in RGBA
out vec4 FragColour;

// ???
uniform sampler2D textureSampler;

void main() {
    vec4 texColour = texture(textureSampler, texCoord);
    FragColour = texColour * fragColour;
}

