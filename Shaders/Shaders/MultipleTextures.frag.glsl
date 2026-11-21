R"(
    #version 420

    // This is a multi-texture blending fragment shader.

    in vec2 TexCoord;

    layout(binding=0) uniform sampler2D Texture0;
    layout(binding=1) uniform sampler2D Texture1;

    uniform float AlphaFactor = 1.0f;

    layout(location=0) out vec4 FragColor;

    void main()
    {
        vec4 color0 = texture(Texture0, TexCoord);
        vec4 color1 = texture(Texture1, TexCoord);

        FragColor = vec4(mix(color0.rgb, color1.rgb, color1.a*AlphaFactor), 1.0f);
    }
)"