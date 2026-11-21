R"(
#version 330 core

    // This vertex shader prepares per-vertex reflection and refraction 
    // directions in world space for use in the fragment shader (e.g., 
    // sampling reflection/refraction environment maps), and transforms 
    // the vertex to clip space for rasterization.

struct Camera {
    vec3 position;    // In world space
};

struct Material {
    vec3 color;
    float reflectionFactor; // The light reflection factor
    float eta;              // The ratio of indices of refraction
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

uniform mat4 M; // Model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

uniform float iTime; //

uniform Camera camera;
uniform Material material;

uniform int dynamic_option = 1;

out vec3 ReflectDir;
out vec3 RefractDir;



void main() {
    vec4 positionInWorld;
    
    if (dynamic_option == 1)
    {
        positionInWorld = M * vec4(VertexPosition, 1.0f);
    }
    else if(dynamic_option == 2)
    {
        float waveX = sin(iTime * 2 + VertexPosition.x * 8);
        float waveY = cos(iTime * 4 + VertexPosition.y * 10);
        float waveZ = sin(iTime * 6 + VertexPosition.z * 12);

        float waveS = (waveX * 0.03) + (waveY * 0.04) + (waveZ * 0.05);
        positionInWorld = M * vec4((VertexPosition + VertexNormal * waveS), 1.0f);
    }

    mat3 N = mat3(transpose(inverse(M)));  // Normal transform matrix
    vec3 normalInWorld = N * VertexNormal;

    vec3 viewInWorld = normalize(vec3(positionInWorld) - camera.position);

    ReflectDir = reflect(viewInWorld, normalInWorld);
    RefractDir = refract(viewInWorld, normalInWorld, material.eta);

    gl_Position = P * V * positionInWorld;
}
)"