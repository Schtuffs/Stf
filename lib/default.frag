#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Inputs the color from the Vertex Shader
in vec4 fragColour;

void main() {
    FragColor = fragColour;
}

