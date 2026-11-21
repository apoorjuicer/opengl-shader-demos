R"(
#version 430 core
layout(std430, binding = 0) buffer RayDistances {
float distances[];
};

layout(std430, binding = 1) buffer RayTextureCoordinates {
float coordinates[];
};

layout(std430, binding = 2) buffer RayTextureIds {
int ids[];
};
uniform vec2 iResolution;
out vec4 FragColor;

vec3 wallcolorwith(int id) {
vec3 wc;
if(id == 1) wc = vec3(1.0, 1.0, 0.2);
else if(id == 2) wc = vec3(0.85, 0.0, 0.0);
else if(id == 3) wc = vec3(0.0, 0.85, 0.0);
else if(id == 4) wc = vec3(0.0, 0.0, 0.85);
else if(id == 5) wc = vec3(0.5, 0.5, 0.5);
else wc = vec3(0.0, 0.0, 0.0);

return wc;
}

void main() {
float dist = distances[int(gl_FragCoord.x)];    // distance to wall
int id = ids[int(gl_FragCoord.x)];              // texture id

float distSafe = max(dist, 0.001);
float wallHeight = iResolution.y / distSafe;   //full height no divison by 0

float wallBottom = (iResolution.y - wallHeight) * 0.5;
float wallTop = wallBottom + wallHeight;

vec3 color = vec3(0.4, 0.7, 1.0); // blue sky

        //get uv coordinates
float u = coordinates[int(gl_FragCoord.x)];
float v = (gl_FragCoord.y - wallBottom) / (wallTop - wallBottom);

if(gl_FragCoord.y < wallBottom) // Floor
{
float t = (wallBottom - gl_FragCoord.y) / wallBottom;
t = clamp(t, 0.0, 1.0);
t *= t;
            //near, far, t
color = mix(vec3(0.55, 0.42, 0.28), vec3(0.25, 0.18, 0.10), t);
} else if(gl_FragCoord.y < wallTop) // Wall
{
            //distance base brightness, inverse fall off
            // 1/(something grow with distance)
            //closer brighter, further dimmer
float brightness = clamp(1.0 / (1.0 + dist * 0.2), 0.0, 1.0);

                // Add some vertical shading
float strength = 0.35;
float d = abs(v - 0.5) * 2.0; // 0..1
float v_shading = 1.0 - d * d * strength;
vec3 wallcolor = wallcolorwith(id);

color = wallcolor * brightness * v_shading;
                //8 vertical columns
                //mod it 0 1 0 1 intervals for each index
                //dark-0 light-1
float stripes = mod(floor(u * 8.0), 2.0);
color *= mix(0.6, 9.0, stripes);

}

FragColor = vec4(color, 1.0);
}

)"