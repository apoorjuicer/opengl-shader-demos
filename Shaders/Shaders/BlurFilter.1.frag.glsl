R"(
    #version 420

    // This is a fragment shader that implements the first pass of a two-pass 
    // Gaussian blur filter. It applies a vertical blur by sampling pixels 
    // along the Y-axis using precomputed Gaussian weights.

    layout(location=0) out vec4 FragColor;
     uniform bool BlurFilter;
     uniform int Binding = 0; 
    layout(binding=0) uniform sampler2D Texture0;
    layout(binding=1) uniform sampler2D Texture1;

    // Precomputed normalized Gaussian weights. 
    // We need only half of all weights because they are symmetric about zero.
    float Weight[5] = float[5]( 0.158435f, 0.148836f, 0.123389f, 
                                0.0902733f, 0.0582848f );

    // Applies the weighted vertical sum of the Gaussian blur filter 
    // to store the result in yet another texture. 

    ivec2 clampCoord(ivec2 coord, ivec2 texSize)
    {
        return clamp(coord, ivec2(0), texSize - 1);
    }

    vec4 clampedTexel(ivec2 coord, ivec2 texSize)
    {
        return texelFetch(Texture0, clampCoord(coord, texSize), 0);
    }

    vec4 clampedTexelOffset(ivec2 coord, ivec2 texSize, ivec2 offset)
    {
        return clampedTexel(coord + offset, texSize);
    }

    // Applies the weighted vertical sum of the Gaussian blur filter 
    // to store the result in yet another texture. 
    void main() 
    {
        ivec2 pix = ivec2(gl_FragCoord.xy);
        vec4 sum = texelFetch(Texture0, pix, 0);

        if(BlurFilter){
        //textureSize returns the dimensions of level lod (if present) of the texture bound to sampler
        ivec2 texSize = textureSize(Texture0, 0);

            sum *= Weight[0];
            sum += clampedTexelOffset(pix, texSize, ivec2(0, -4)) * Weight[4];
            sum += clampedTexelOffset(pix, texSize, ivec2(0, -3)) * Weight[3];
            sum += clampedTexelOffset(pix, texSize, ivec2(0, -2)) * Weight[2];
            sum += clampedTexelOffset(pix, texSize, ivec2(0, -1)) * Weight[1];
            sum += clampedTexelOffset(pix, texSize, ivec2(0,  1)) * Weight[1];
            sum += clampedTexelOffset(pix, texSize, ivec2(0,  2)) * Weight[2];
            sum += clampedTexelOffset(pix, texSize, ivec2(0,  3)) * Weight[3];
            sum += clampedTexelOffset(pix, texSize, ivec2(0,  4)) * Weight[4];

        }

 
        FragColor = sum;
    }
)"