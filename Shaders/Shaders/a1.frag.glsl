R"(
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)

uniform int uTab; // uniform variable for Tabs
uniform float iTime; // Shader playback time (in seconds)

uniform vec2 uCursor;

uniform float radius;
uniform float spacing;
uniform float y_height;

layout(location=0) out vec4 FragColor;

vec4 checkerboardTexture(vec2 uv, int size)
{
    uv = uv * size + iTime;
    //5.0f makes it more darker
    float color = mod(floor(uv.x) + floor(uv.y), 2.0f) / 15.0f; // x+y even:black  odd:white
    return vec4(color, color, color, 1.0f);
}

vec4 colorTab(vec2 uv)
{
    //get per pixel belonging to each quadrant
    float v = uv.x * uv.y; 

    //mask for postive and negative regions
    float mask = smoothstep(0.0, 0.05, v);

    //time interp, range [0,1]
    float t = 0.5+0.5*sin(iTime);
    
    vec3 black = vec3(0.0);
    vec3 startColor = vec3(0.1, 0.2, 0.3);
    vec3 endColor = vec3(0.3, 0.2, 0.1);

    vec3 interp_col = mix(black, mix(startColor, endColor, t), mask);

    return vec4(interp_col, 1.0f);
}

float circle(vec2 current_pixel, vec2 circle_center, float radius)
{
    float distance = length(current_pixel - circle_center);
         //return (distance <= radius) ? 1.0 : 0.0;
    //smoothstep performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1.
    return smoothstep(radius, radius - 2.0, distance);

}

vec3 overlayTabsDots(vec2 p, vec3 baseColor)
{

    vec2 center = vec2(iResolution.x * 0.5, y_height);

    vec2 c0 = center + vec2(-spacing, 0.0);
    vec2 c1 = center + vec2( 0.0,     0.0);
    vec2 c2 = center + vec2( spacing, 0.0);

    vec3 dotOff = vec3(0.45);
    vec3 dotOn  = vec3(0.90);

    vec3 col0 = (uTab == 0) ? dotOn : dotOff;
    vec3 col1 = (uTab == 1) ? dotOn : dotOff;
    vec3 col2 = (uTab == 2) ? dotOn : dotOff;

    float a0 = circle(p, c0, radius);
    float a1 = circle(p, c1, radius);
    float a2 = circle(p, c2, radius);

    vec3 c = baseColor;
    c = mix(c, col0, a0);
    c = mix(c, col1, a1);
    c = mix(c, col2, a2);
    return c;
}

vec4 litCursor(){
     //Calculate distance from pixel to Cursor
      float distance = length(gl_FragCoord.xy - uCursor);

      //mix take 3 params,
      //vec4(distance, 0.0, 0.0, 1.0)

      float radius = 120.0;
      float t = smoothstep(0.0, radius, distance);

      return vec4(mix(vec3(1.0),vec3(0.0), t),1.0);
}

void main()
{
    vec3 color;

     if(uTab == 0)// Animation tab
     {
     color = checkerboardTexture(gl_FragCoord.xy / iResolution.xy, 20).rgb;
     }
     else if(uTab == 1)// Color tab
     {
         vec2 uv = (gl_FragCoord.xy * 2.0f - iResolution.xy)/ iResolution.xy;
         color = colorTab(uv).rgb;
     }
     else if(uTab == 2)// Mouse tab
     {
       color = litCursor().rgb;
     }

      // Overlay the 3 tab dots LAST
    color = overlayTabsDots(gl_FragCoord.xy, color);

    FragColor = vec4(color, 1.0);
}
)"