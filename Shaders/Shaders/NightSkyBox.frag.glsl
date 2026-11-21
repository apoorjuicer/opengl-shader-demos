R"(
    #version 420

    // Fragment shader for cube mapping with gamma correction.

    in vec3 Vec;

    layout(binding=0) uniform samplerCube CubeMapTex;

    layout(location=0) out vec4 FragColor;

    void main() 
    {
        // Access the cube map texture
        vec3 color = vec3(texture(CubeMapTex, normalize(Vec)));

        // Gamma correction
        color = pow(color, vec3(1.0f/2.2f));

        FragColor = vec4(color, 1.0f);
    }

)"