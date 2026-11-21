R"( 
#version 330 core

    // This fragment shader computes per-pixel lighting using the Phong 
    // illumination model. It takes a single light source and material 
    // properties, transforms lighting into view space, and outputs ambient, 
    // diffuse, and specular contributions for realistic shaded rendering.

    //uniform int Option = 1;

struct Light {
    vec3 position;      // Position of the light source in the world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

struct Material {
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor
};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space

uniform Light light[1]; //declare array of size1
uniform Material material;
uniform mat4 V;         // View transform matrix

layout(location = 0) out vec4 FragColor;

    //lambert lighting
vec3 lambertLighting(vec3 p, vec3 n, vec3 lightPos, vec3 lightIntensity, vec3 kD) {
        // direction from surface point to light
    vec3 lightDir = normalize(lightPos - p);

        // cosine term (Lambert)
    float ndotl = max(dot(normalize(n), lightDir), 0.0);

        // incoming light
    vec3 incomingLight = ndotl * lightIntensity;

    //vec3 diffuse = vec3(1.0);

        // final shaded color
    return kD * incomingLight;
}
//Phong reflection model = (dot(R, V))^n
//R = reflection of light d around normal
// V = view direction
// n is shininess

//Blinn-Phong specular =(max(N.H,0))^n
// vec3 H = normalize(L + V);
//float spec = pow(max(dot(N, H), 0.0), shininess);

vec3 blinnPhong(vec3 pView, vec3 nView, Light Lw, Material m, mat4 Vmat) {
    vec3 N = normalize(nView);

    // Convert light position from world -> view
    vec3 lightPosView = vec3(Vmat * vec4(Lw.position, 1.0));

    // Directions in view space
    vec3 L = normalize(lightPosView - pView);
    vec3 Vdir = normalize(-pView);         // camera is at (0,0,0) in view space
    vec3 H = normalize(L + Vdir);

    // Ambient
    vec3 ambient = m.Ka * Lw.La;

        // Diffuse 
    vec3 diffuse = lambertLighting(pView,            // fragment position (view space)
    N,                // normal (view space)
    lightPosView,     // light position (view space)
    Lw.Ld,            // diffuse light intensity
    m.Kd              // material diffuse reflectivity
    );

    // Specular (Blinn–Phong)
    float ndoth = max(dot(N, H), 0.0);
    //N.L>0 light on same side as normal
    //if yes light spec * material spec * pow(H.L)
    vec3 specular = (dot(N, L) > 0.0) ? m.Ks * Lw.Ls * pow(ndoth, m.shininess) : vec3(0.0);

    //phong reflection
    //vec3 result = ambient + diffuse + specular
    return ambient + diffuse + specular;
}

vec3 Phong(vec3 pView, vec3 nView, Light Lw, Material m, mat4 Vmat) {
    vec3 N = normalize(nView);

    vec3 lightPosView = vec3(Vmat * vec4(Lw.position, 1.0));
    vec3 L = normalize(lightPosView - pView);
    vec3 Vdir = normalize(-pView);
    vec3 R = normalize(reflect(-L, N));

    vec3 ambient = m.Ka * Lw.La;

    float ndotl = max(dot(N, L), 0.0);
    vec3 diffuse = m.Kd * Lw.Ld * ndotl;

    float rdotv = max(dot(R, Vdir), 0.0);
    vec3 specular = (ndotl > 0.0) ? m.Ks * Lw.Ls * pow(rdotv, m.shininess) : vec3(0.0);

    return ambient + diffuse + specular;
}

void main() {

    FragColor = vec4(blinnPhong(Position, Normal, light[0], material, V), 1.0);
}
)"