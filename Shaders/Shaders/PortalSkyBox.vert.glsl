R"(
    #version 330 core

    // Vertex shader for cube mapping that passes non-normalized mapping vector for
    // current vertex and transforms the vertex position to the clipping space.

    //SkyBox for portal

layout(location = 0) in vec3 VertexPosition;

uniform mat4 M; // Model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

uniform mat4 ProjectorMatrix; // Projection matrix for the projector
out vec4 ProjTexCoord;  // Texture coordinates for the projected texture

out vec3 Vec;

void main() {
    Vec = VertexPosition;

    gl_Position = P * V * M * vec4(VertexPosition, 1.0);

    ProjTexCoord = ProjectorMatrix * M * vec4(VertexPosition, 1.0);
}
)"

