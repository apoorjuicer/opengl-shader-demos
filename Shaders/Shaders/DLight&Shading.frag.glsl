R"(

#version 420

    // This fragment shader implements the second (lightning) pass for deferred 
    // shading that applies lighting calculations using the stored surface data

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D LightDirectionTex;
layout(binding = 1) uniform sampler2D ViewDirectionTex;
layout(binding = 2) uniform sampler2D TexCoordinatesTex;
layout(binding = 5) uniform sampler2D ColorTex; //Diffuse Ogre
layout(binding = 6) uniform sampler2D NormalMapTex; //Normal Ogre

struct Material {
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor
};

struct Light {
    vec3 position;      // Position of the light source in the world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

uniform Material material;
uniform Light light[1];
uniform vec4 BackgroundColor;
uniform int TextureMode = 1;
uniform vec2 iResolution;   // Viewport resolution (in pixels)

    // Blinn-Phong illumination with light and view direction vectors given as parameters.
    // All vector parameters must be normalized.
vec3 blinnPhong(vec3 color, vec3 normal, vec3 lightDir, vec3 viewDir) {
    vec3 ambient = light[0].La * material.Ka * color;

    float lDotN = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = color * light[0].Ld * material.Kd * lDotN;

    vec3 spec = vec3(0.0);
    if(lDotN > 0.0) {
        vec3 halfway = normalize(viewDir + lightDir);
        spec = light[0].Ls * material.Ks *
            pow(max(dot(halfway, normal), 0.0), material.shininess);
    }
    return ambient + diffuse + spec;
}

void main() {

    ivec2 pixel = ivec2(gl_FragCoord.xy);

    //vec3 L = texelFetch(LightDirectionTex, pixel, 0).xyz;
   // vec3 V = texelFetch(ViewDirectionTex, pixel, 0).xyz;
    //vec2 uv = texelFetch(TexCoordinatesTex, pixel, 0).xy;

    if(TextureMode == 1) {
        vec2 objectTextureCoordinates 
        = vec2(texelFetch(TexCoordinatesTex, ivec2(gl_FragCoord), 0));

        if(objectTextureCoordinates.x < 0 || objectTextureCoordinates.x > 1 ||
            objectTextureCoordinates.y < 0 || objectTextureCoordinates.y > 1) {
            FragColor = BackgroundColor;
            return;
        }

        if(objectTextureCoordinates.x == 0.0 && objectTextureCoordinates.y == 0.0) {
            FragColor = BackgroundColor;
            return;
        } 
            vec3 normal = vec3(texture(NormalMapTex, objectTextureCoordinates));
            normal.xy = 2.0 * normal.xy - 1.0;
            vec3 color = vec3(texture(ColorTex, objectTextureCoordinates));

            vec3 LightDirection = vec3(texelFetch(LightDirectionTex, ivec2(gl_FragCoord), 0));
            vec3 ViewDirection = vec3(texelFetch(ViewDirectionTex, ivec2(gl_FragCoord), 0));
        // Shade the color using the Blinn-Phong illumination model
            color = blinnPhong(color, normal, normalize(LightDirection), normalize(ViewDirection));

            FragColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
        

    } else if(TextureMode == 2) {
        FragColor = texelFetch(LightDirectionTex, ivec2(gl_FragCoord), 0) * 0.5 + 0.5;
    } else if(TextureMode == 3) {
        FragColor = texelFetch(ViewDirectionTex, ivec2(gl_FragCoord), 0) * 0.5 + 0.5;
    }
}

)"