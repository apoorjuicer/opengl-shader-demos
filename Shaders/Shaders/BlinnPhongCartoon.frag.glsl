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

    in vec3 Position;
    in vec3 Normal;
    in vec2 TexCoord;

    uniform Light light[1];
    uniform Material material;
    uniform mat4 V;         // View transform matrix

    layout(location=0) out vec4 FragColor;

    const float PI = 3.14159265358979323846;

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

    void main() 
    {
        vec3 N = normalize(Normal);

       vec3 color = blinnPhongCartoon(Position, N, light[0], material, V);
        // lastly) Gamma correction 
        color = pow(color, vec3(1.0f/2.2f));
        FragColor = vec4(color, 1.0f);
    }
)"