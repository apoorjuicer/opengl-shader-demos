R"(
	    #version 420

    // Fragment shader for cube mapping with texture projection and gamma correction.

in vec3 Vec;
in vec4 ProjTexCoord;

layout(binding = 7) uniform sampler2D Texture;
layout(binding = 6) uniform samplerCube CubeMapTex;

layout(location = 0) out vec4 FragColor;

    float luminance(vec3 color)
    {
        return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
    }

void main() {
        // Access the cube map texture
        vec4 texColor = texture(CubeMapTex, normalize(Vec));
        //vec4 portalColor = texColor;
if (ProjTexCoord.w > 0.0 &&
    ProjTexCoord.x > 0.0 && ProjTexCoord.x < ProjTexCoord.w &&
    ProjTexCoord.y > 0.0 && ProjTexCoord.y < ProjTexCoord.w)
{
        vec4 portalColor = textureProj(Texture, ProjTexCoord);
        //without lumi big fat black box
        float lumi = clamp(luminance(portalColor.rgb), 0.0, 1.0);
        texColor = mix(texColor, portalColor, lumi);
}

//gamma coorrect
FragColor = pow(texColor, vec4(1.0 / 2.2));

}
)"

