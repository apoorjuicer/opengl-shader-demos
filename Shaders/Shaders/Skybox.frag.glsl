R"(
#version 420

    // Fragment shader for cube mapping with gamma correction.

    in vec3 Vec;

    layout(location=0) out vec4 FragColor;

    vec4 checkerboardTexture(vec2 uv, int size)
    {
        uv = uv * size;
        //5f makes it more darker
        float color = mod(floor(uv.x) + floor(uv.y), 2.f); // x+y even:black  odd:white
        return vec4(color, color, color, 1.f);
    }

    bool insideFace(float a, float b)
    {
        return (a >= -1.0 && a <= 1.0 &&
                b >= -1.0 && b <= 1.0);
    }

    vec2 mapToUV(float a, float b, bool flipA)
    {
        float u = flipA ? (1.0 - a) * 0.5 : (1.0 + a) * 0.5;
        float v = (1.0 + b) * 0.5;
        return vec2(u, v);
    }


    // vec2 vec2uv(vec3 v)
    // top btm face ignored
    // assume v normalized(view vector)
    // 0,0 = btm-left, 1,1 top right
    // *====X(1,1)
    // =
    // =
    // X(0,0)
    // min is -1,-1,-1 max is 1,1,1
    // only need care x and z, since y is ignored(topbottom)
    // p(t) = t * v, t is dstiance scale,
    vec2 vec2uv(vec3 v)
    {
        if (length(v) < 1e-6)
            return vec2(0.0);

        // ignore top/bottom
        if (abs(v.y) > abs(v.x) && abs(v.y) > abs(v.z)){
            return vec2(0.0);
        }
            

        // Check Z faces first if Z dominates, else X faces
        if (abs(v.z) >= abs(v.x))
        {
            // Z face hit first
            float t = (v.z > 0.0) ? (1.0 / v.z) : (-1.0 / v.z);
            float x = t * v.x;
            float y = t * v.y;

            if (insideFace(x, y))
                return mapToUV(x, y, v.z > 0.0); // flip for z=+1
        }
        else
        {
            // X face hit first
            float t = (v.x > 0.0) ? (1.0 / v.x) : (-1.0 / v.x);
            float z = t * v.z;
            float y = t * v.y;

            if (insideFace(z, y))
                return mapToUV(z, y, v.x < 0.0); // flip for x=-1
        }

        return vec2(0.0);
    }

    void main() 
    {
        // Access the cube map texture
        vec3 color = vec3(checkerboardTexture(vec2uv(normalize(Vec)), 10));

         // Gamma correction
        color = pow(color, vec3(1/2.2));

        FragColor = vec4(color, 1);
    }
)"