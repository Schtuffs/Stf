#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec4 aColour;
// Texture Coordinates
layout (location = 2) in vec2 aTex;


// Outputs the color for the Fragment Shader
out vec4 fragColour;
// Outputs the texture coordinates to the fragment shader
out vec2 texCoord;

// Controls the scale of the vertices
uniform float scale;

void main() {
    // Outputs the positions/coordinates of all vertices
    gl_Position = vec4(aPos, 1.0);

    // Assigns the colors from the Vertex Data to "color"
    fragColour = aColour;

    // Assigns the texture coordinates from the Vertex Data to "texCoord"
    texCoord = aTex;
}
