R"(
    #version 420

    // This is a fragment shader that implements several basic image processing
    // filters for post-processing effects. It takes a rendered texture from 
    // a previous pass and applies one of four different filters based on 
    // an option

    layout(location=0) out vec4 FragColor;

    // Texture from the first pass
    layout(binding=0) uniform sampler2D Texture;

    // Parameters
    uniform int Option;

    float luminance(vec3 color) 
    {
        return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
    }

    float rand(vec2 co)
    {
        return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    }

    void main()
    {
        ivec2 pix = ivec2(gl_FragCoord.xy);
        vec4 color = texelFetch(Texture, pix, 0);
    
        if (Option==1) // No Filter
        {
            FragColor = color;
        }
        else if (Option==2) // Inverse filter
        {
            FragColor = vec4(1.0f) - color;
        }
        else if (Option==3) // Color filter
        {
            FragColor = vec4(color.r, 0.0f, 0.0f, 1.0f);
        }
        else if (Option==4) // Brightness filter
        {
            float b = luminance(color.rgb);
            FragColor = vec4(b, b, b, 1.0f);
        }
        else if (Option==5) // Noise adding filter
        {
            float r = rand(gl_FragCoord.xy);
            FragColor = (r < 0.1f) ? vec4(1.0f): color;
        }else{
            FragColor = vec4(1,0,1,1); // hot pink
        }


    }
)"