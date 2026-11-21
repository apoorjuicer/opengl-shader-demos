R"(
    #version 330 core

    // This fragment shader computes per‑pixel lighting using the Phong 
    // illumination model. It takes a single light source and material 
    // properties, transforms lighting into view space, and outputs ambient, 
    // diffuse, and specular contributions for realistic shaded rendering.

    struct Light 
    {
        vec3 position;      // Position of the light source in the world space
        vec3 La;            // Ambient light intensity
        vec3 Ld;            // Diffuse light intensity
        vec3 Ls;            // Specular light intensity
    };

    struct Material 
    {
        vec3 Ka;            // Ambient reflectivity
        vec3 Kd;            // Diffuse reflectivity
        vec3 Ks;            // Specular reflectivity
        float shininess;    // Specular shininess factor
    };

    in vec3 Position;       // In view space
    in vec3 Normal;         // In view space

    uniform Light light[1];
    uniform Material material;
    uniform mat4 V;         // View transform matrix

    layout(location=0) out vec4 FragColor;

    vec3 phong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
    {
        // --- Convert light position from world -> view space
        vec3 lightPosVS = vec3(view * vec4(light.position, 1.0));

        // --- Direction vectors in view space
        vec3 N = normalize(normal);                 // surface normal
        vec3 L = normalize(lightPosVS - position);  // surface -> light
        vec3 Vdir = normalize(-position);           // surface -> camera (camera is at origin in view space)

        // --- Ambient
        vec3 ambient = light.La * material.Ka;

        // --- Diffuse (Lambert)
        float NdotL = max(dot(N, L), 0.0);
        vec3 diffuse = light.Ld * material.Kd * NdotL;

        // --- Specular (Phong: reflect then dot with view dir)
        vec3 specular = vec3(0.0);
        if (NdotL > 0.0)
        {
            vec3 R = reflect(-L, N); // reflect expects "incident" vector pointing *towards* the surface
            float RdotV = max(dot(R, Vdir), 0.0);
            specular = light.Ls * material.Ks * pow(RdotV, material.shininess);
        }
        return ambient + diffuse + specular;
    }

    void main() 
    {
        FragColor = vec4(phong(Position, normalize(Normal), light[0], material, V), 1.0f);
    }
)"