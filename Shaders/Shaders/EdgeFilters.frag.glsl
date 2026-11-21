R"(
    #version 420

       // This is a fragment shader that implements edge detection using the Sobel
    // operator for post-processing effects. It analyzes a rendered texture 
    // from a previous pass to identify edges by detecting significant 
    // luminance changes between neighboring pixels.

    layout(location=0) out vec4 FragColor;

   // Texture from the first pass
   // layout(binding=0) uniform sampler2D Texture;
   // layout(binding=1) uniform sampler2D Texture1;
    layout(binding=2) uniform sampler2D Texture2; // need second pass cause hori+vert blur

    uniform bool EdgeFilter;
    uniform vec4 EdgeColor;
    uniform vec4 BackgroundColor;
    uniform float EdgeThreshold = 0.05f;

    // Clamp offset so neighbor sampling stays inside texture bounds
    ivec2 clampOffset(ivec2 coord, ivec2 texSize, ivec2 offset)
    {
        ivec2 targetCoord   = coord + offset;
        ivec2 clampedTarget = clamp(targetCoord, ivec2(0), texSize - 1);
        return clampedTarget - coord;
    }

    float luminance(vec3 color)
    {
        return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
    }

    float luminanceAt(ivec2 coord, ivec2 texSize, ivec2 offset)
    {
        ivec2 safeOffset = clampOffset(coord, texSize, offset);
        vec3 color = texelFetchOffset(Texture2, coord, 0, safeOffset).rgb;
        return luminance(color);
    }

    void main()
    {
        ivec2 pix = ivec2(gl_FragCoord.xy);

        if (EdgeFilter) {
            ivec2 texSize = textureSize(Texture2, 0);

            float s00 = luminanceAt(pix, texSize, ivec2(-1,  1));
            float s10 = luminanceAt(pix, texSize, ivec2(-1,  0));
            float s20 = luminanceAt(pix, texSize, ivec2(-1, -1));
            float s01 = luminanceAt(pix, texSize, ivec2( 0,  1));
            float s21 = luminanceAt(pix, texSize, ivec2( 0, -1));
            float s02 = luminanceAt(pix, texSize, ivec2( 1,  1));
            float s12 = luminanceAt(pix, texSize, ivec2( 1,  0));
            float s22 = luminanceAt(pix, texSize, ivec2( 1, -1));

            float sx = s00 + 2.0 * s10 + s20 - (s02 + 2.0 * s12 + s22);
            float sy = s00 + 2.0 * s01 + s02 - (s20 + 2.0 * s21 + s22);

            float g = sx * sx + sy * sy;

            FragColor = (g > EdgeThreshold)
                ? EdgeColor
                : texelFetch(Texture2, pix, 0) * (1.0 - BackgroundColor.a)
                  + BackgroundColor * BackgroundColor.a;
        }
        else {
            FragColor = texelFetch(Texture2, pix, 0);
        }
    }
)"