R"(
    #version 330 core

    struct Light 
    {
        vec3 position;      // Position of the light source in the world space
    
        vec3 L;             // Light intensity (for PBR)

        vec3 La;            // Ambient light intensity (for Phong)
        vec3 Ld;            // Diffuse light intensity (for Phong)
        vec3 Ls;            // Specular light intensity (for Phong)
    };

    struct Material 
    {
        // For PBR
        float rough;        // Roughness
        float metal;        // Metallic (1.0f) or dielectric (0.0f)
        vec3 color;         // Diffuse color for metal

        // For Phong
        vec3 Ka;            // Ambient reflectivity
        vec3 Kd;            // Diffuse reflectivity
        vec3 Ks;            // Specular reflectivity
        float shininess;    // Specular shininess factor

        float effect;
    };

    struct Fog 
    {
      float minDist;
      float maxDist;
      vec4 color;
    };

    in vec3 Position;
    in vec3 Normal;
    in vec2 TexCoord;

    uniform Light light[1];
    uniform Material material;
    uniform Fog fog; // good ol fog
    uniform mat4 V;         // View transform matrix

    layout(location=0) out vec4 FragColor;

    const float PI = 3.14159265358979323846;

    const int CHECKERBOARD_SIZE = 50;
const float CHECKERBOARD_MIXLEVEL = 0.5; // Mixing level with shade 

    //
    // The Microgeometry Normal Distribution Function, based on GGX/Trowbrodge-Reitz, 
    // that describes the relative concentration of microfacet normals 
    // in the direction H. It has an effect on the size and shape 
    // of the specular highlight.
    //
    // Parameter is cosine of the angle between the normal and H which is the halfway vector of 
    // both the light direction and the view direction
    //
    float ggxDistribution(float nDotH) 
    {
        float alpha2 = material.rough * material.rough * material.rough * material.rough;
        float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
        return alpha2 / (PI * d * d);
    }

    //
    // The Smith Masking-Shadowing Function describes the probability that microfacets with 
    // a given normal are visible from both the light direction and the view direction.
    //
    // Parameter is cosine of the angle between the normal vector and the view direction.
    //
    float geomSmith(float nDotL) 
    {
        float k = (material.rough + 1.0f) * (material.rough + 1.0f) / 8.0f;
        float denom = nDotL * (1.0f - k) + k;
        return 1.0f / denom;
    }

    //
    // Schlick approximation for Fresnel reflection that defines the probability of light
    // reflected from an optically flat surface.
    //
    // Parameter is cosine of the angle between the light direction vector and 
    // the halfway vector of both the light direction and the view direction
    //
    vec3 schlickFresnel(float lDotH) 
    {
        vec3 f0 = vec3(0.04f); // Dielectrics
        if (material.metal == 1.0f)
            f0 = material.color;
        return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
    }

    //
    // Bidirectional Reflectance Distribution Function.
    // This is the common way to model reflectance based on microfacet theory. 
    // This theory was developed to describe reflection from general, non-optically flat surfaces. 
    // It models the surface as consisting of small facets that are optically flat (mirrors) and 
    // are oriented in various directions. Only those that are oriented correctly to reflect toward 
    // the viewer can contribute.
    //
    // Parameters are the position of a fragment and the surface normal in the view space.
    //
    vec3 microfacetModel(vec3 position, vec3 n) 
    {  
        vec3 diffuseBrdf = material.color;

        vec3 lightI = light[0].L;
        vec3 lightPositionInView = (V * vec4(light[0].position, 1.0f)).xyz;

        vec3 l = lightPositionInView - position;
        float dist = length(l);
        l = normalize(l);
        lightI *= 100 / (dist * dist); // Intensity is normalized, so scale up by 100 first

        vec3 v = normalize(-position);
        vec3 h = normalize(v + l);
        float nDotH = dot(n, h);
        float lDotH = dot(l, h);
        float nDotL = max(dot(n, l), 0.0f);
        float nDotV = dot(n, v);
        vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * schlickFresnel(lDotH) * 
                          geomSmith(nDotL) * geomSmith(nDotV);

        return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
    }

    vec3 checkerboardTexture(vec2 uv, int size)
    {
        uv = uv * size;
        //5f makes it more darker
        float color = mod(floor(uv.x) + floor(uv.y), 2.f)/15.f; // x+y even:black  odd:white
        return vec3(color, color, color);
    }

    // Cartoon shading function
    float cartoon(float value)
    {
        //2-3 cartoony , 5-8 smoother
        int levels = 2; //number of level to cartoon
        float scaleFactor = 1.0f / levels;
        return floor(value * levels) * scaleFactor;
    }
  
    vec3 blinnPhongCartoon(vec3 pView, vec3 nView, Light Lw, Material m, mat4 Vmat)
    {
        vec3 N = normalize(nView);

        // world -> view
        vec3 lightPosView = vec3(Vmat * vec4(Lw.position, 1.0));

        // directions in view space
        vec3 L = normalize(lightPosView - pView);
        vec3 Vdir = normalize(-pView);
        vec3 H = normalize(L + Vdir);
        // quantized -> reduce a smooth value into a small number of fixed step
        // change lightning smoothly from ddark to bright
        // restricting the ndotl to fixed value
        // Ambient (usually dont need quantized)
        vec3 ambient = m.Ka * Lw.La;

        // Diffuse (Lambert) quantized
        float ndotl = max(dot(N, L), 0.0);
        float diffStep = cartoon(ndotl);
        vec3 diffuse = m.Kd * Lw.Ld * diffStep;

        // Specular (Blinn-Phong) quantized
        vec3 specular = vec3(0.0);
        if (ndotl > 0.0)
        {
            float ndoth = max(dot(N, H), 0.0);
            float spec = pow(ndoth, m.shininess);
            float specStep = cartoon(spec);
            specular = m.Ks * Lw.Ls * specStep;
        }

        return ambient + diffuse + specular;
    }


    vec2 tileCenter(vec2 uv, float tiles)
    {
        vec2 f = fract(uv * tiles);      // [0,1) within each tile cell
        return f * 2.0 - 1.0;            // [-1,1], center at (0,0)
    }


    bool discardCircle(vec2 uv, float tiles, float radius)
    {
        vec2 p = tileCenter(uv, tiles);   // p in [-1,1], center at 0
        return length(p) < radius;        // radius
    }
    bool discardStar(vec2 uv, float tiles, float size)
    {
        vec2 p = tileCenter(uv, tiles);
        p /= size;

        float a = atan(p.y, p.x);    // angle
        float d = length(p);         // radius

        float spikes = 5.0;          // 5-point star
        float baseR  = 0.55;
        float spikeA = 0.20;
        float r = baseR + spikeA * cos(spikes * a);

        return d < r;
    }




    void main() 
    {
        //1) base lighting pbr shade
        vec3 N = normalize(Normal);
        vec3 pbrShade = microfacetModel(Position, N);
        //2) combine checkerboard with checkboard / texture
        //vec3 tex = checkerboardTexture(TexCoord, 1);
       // vec3 color = mix(shade, tex, 0.1f);
        vec3 color = pbrShade;

        //FragColor = vec4(blinnPhongCartoon(Position, normalize(Normal), light[0], material, V), 1.0f);

            if (material.effect==0.0f) // PBR MATERIAL_PBR or MATERIAL_PBR_PHONG we use phong
            {
                    color = pbrShade;
            }
            //creates holes in pixel, discard 
            else if (material.effect==1.0f) // Discard, do not draw this pixel
            {
               // if (discardCircle(TexCoord, 10.0, 0.5)){
                //   discard;
                //}
                if (discardStar(TexCoord, 10.0, 1.0)) discard;
                color = pbrShade;
            }
            else if (material.effect==2.0f) // Cartoon
            {
               color = blinnPhongCartoon(Position, N, light[0], material, V);
            }
            else if (material.effect==3.0f) // Checkerboard texture
            {
                vec3 tex = checkerboardTexture(TexCoord, 80);
                color = mix(pbrShade, tex, 0.1f);
            }
        //second last to apply fog..
        //second last) Fog
        //float dist = length(Position);
        //float fogFactor = clamp((dist - fog.minDist) / (fog.maxDist - fog.minDist), 0.0, 1.0);
        //color = mix(color, fog.color.rgb, fogFactor);


        // lastly) Gamma correction 
        color = pow(color, vec3(1.0f/2.2f));
        FragColor = vec4(color, 1.0f);
    }
)"