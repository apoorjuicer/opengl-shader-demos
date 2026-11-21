R"(
#version 420

layout(location = 0) out vec4 FragColor;
layout(binding = 3) uniform sampler2D Texture3;
layout(binding = 4) uniform sampler2D Texture4;

uniform float AlphaFactor;

void main() {
    ivec2 coord = ivec2(gl_FragCoord.xy);

    //texelFetch - perform a lookup of a single texel within a texture
    //texelFetch performs a lookup of a single texel from texture coordinate P in the texture bound to sampler.

    //pass3
    vec4 EdgeColor = texelFetch(Texture3, coord, 0);
    //pass4 
    vec4 CartoonColor = texelFetch(Texture4, coord, 0);

    float blend = (1 - AlphaFactor);

    if( EdgeColor.r > 0.5 ){
       FragColor = vec4(1.0) * blend + CartoonColor * AlphaFactor;
    }else{
         FragColor = EdgeColor;
    }
}
)"