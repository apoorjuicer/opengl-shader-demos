R"(
   #version 330 core

    uniform vec2 iResolution;   // Viewport resolution (in pixels)
    uniform float iTime;        // Shader playback time (in seconds)

    layout(location=0) out vec4 FragColor;

    struct Light
    {
        vec3 position;
    };

    struct Ray
    {
        vec3 origin;
        vec3 direction;   // Normalized
    };

    struct Sphere
    {
        vec3 center;
        float radius;
    };


    //to rotate a vector around an axis
    vec3 rodrigues_formula(vec3 vector, vec3 axis, float angle){
        vec3 norm_axis = normalize(axis);

         return vector * cos(angle) 
         + norm_axis * dot(norm_axis, vector) * (1 - cos(angle))
         + cross(norm_axis, vector) * sin(angle) ;
         
    }
    //Rodrigues rotates vectors; points must be translated to/from the pivot
    vec3 rotateAboutAxis(vec3 point, vec3 pivot, vec3 axis, float angle)
    {
        //zero-length axis results in udb invalid normalization
        //guard against it
        const float FLT_EPSILON = 1e-6;
        if (length(axis) < FLT_EPSILON){
            return point;
        }
        vec3 vector = point - pivot; //express a point in pivot-local space
        vec3 roddy = rodrigues_formula(vector,axis,angle); 
        return pivot + roddy;     // move pivot back to world space
    }

    // Return the shortest non-negative time of intersection the ray and the sphere.
    // Return 0.0f when there is no intersection
    //ray contains origin and direction
    //sphere contains radius and center
    float calculate_t_intersect(Ray ray, Sphere sphere)
    {
        //ray point : P = O + tD
        //sphere condition (p-s) . (p-s) = r^2
        vec3 O = ray.origin;       // eye / camera
        vec3 D = ray.direction;    // ray direction 
        vec3 S = sphere.center;    // sphere center
        float r = sphere.radius;   // sphere radius
        //
         vec3 V = O - S;

        float A = dot(D, D);               // = 1 if D is normalized
        float B = dot(2*V, D);
        float C = dot(V, V) - r * r;

        //t=qudratic eqn

        float discriminant = B * B - 4.0 * A * C;
        if (discriminant < 0.0){
             return 0.0; //no intersection
        }
       float sqrtdisc = sqrt(discriminant);
        // quadratic eqn results in 2 answers
        //t0 is always smaller than t1, t0<t1
        float t0 = (-B - sqrtdisc) / (2 * A);
        float t1 = (-B + sqrtdisc) / (2 * A);

        if (t0 >= 0.0 && t1 >= 0.0){ //case3
            return min(t0, t1);
        }           
        else if (t0 >= 0.0){ //case2
            return t0;
        }          
        else if (t1 >= 0.0){ //case2
            return t1;
        }           
        else{
            return 0.0; // no interection, case1
        }
            
    }

    //lambert lighting
    vec3 lambertLighting(vec3 p, vec3 n, vec3 lightPos, vec3 lightIntensity)
    {
        // direction from surface point to light
        vec3 lightDir = normalize(lightPos - p);

        // cosine term (Lambert)
        float ndotl = max(dot(normalize(n), lightDir), 0.0);

        // incoming light
        vec3 incomingLight = ndotl * lightIntensity;

        // diffuse material color (white)
        vec3 diffuse = vec3(1.0);

        // final shaded color
        return diffuse * incomingLight;
    }

    void main()
    {
        vec2 uv = (gl_FragCoord.xy - 0.5f * iResolution.xy) / iResolution.y;
        vec3 direction = normalize(vec3(uv, -1.5f)); 
        Ray ray = Ray(vec3(0.0f, 0.0f, 8.0f), direction);

        //center of the screen is 0 because of NDC, stationary
        Sphere earth = Sphere(vec3(0.0f, 0.0f, 0.0f), 1.0f);

        //rotational objects
        //sun and moon orbit earth at diff d and speed
        //time based rotation from iTime
        Sphere sun;
        float sunAngle = iTime * 0.4;               // constant speed
        vec3 sunBase = vec3(8.0, 0.0, 0.0);         // distance from Earth
        sun.center = rotateAboutAxis(
            sunBase,                                // sun point
            earth.center,                            // r about center earth
            vec3(0.0, 1.0, 0.0),                    // r axis y
            sunAngle);                              // r angle
        sun.radius = 1.0;

        Sphere moon;
        float moonAngle = iTime * 1.2;              // faster than Sun
        vec3 moonBase = vec3(2.0, 0.0, 0.0);        // closer to the Earth
        moon.center = rotateAboutAxis(
            moonBase,                               // moon point
            earth.center,                           // r about center earth
            vec3(0.0, 1.0, 0.0),                    // r axis y
            moonAngle);                             // r angle
        moon.radius = 0.2;

        //center of sun = light source
        Light light;
        light.position = sun.center;

        //need 3 time ray interesct each 3 obj
        float t1 = calculate_t_intersect(ray, earth);
        float t2 = calculate_t_intersect(ray, sun);
        float t3 = calculate_t_intersect(ray, moon);

        //t0 = earth
        //t1 = sun
        //t3 = moon


        float t = 0.0;
        int hitId = 0; 
        // 0 = none
        // 1 = earth
        // 2 = sun
        // 3 = moon

        // earth
        if (t1 > 0.0) {
            t = t1;
            hitId = 1;
        }

        // sun
        if (t2 > 0.0 && (t == 0.0 || t2 < t)) {
            t = t2;
            hitId = 2;
        }

        // moon
        if (t3 > 0.0 && (t == 0.0 || t3 < t)) {
            t = t3;
            hitId = 3;
        }

        if (hitId == 0)
        {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0); // space
        }
        else if (hitId == 2) // sun
        {
            FragColor = vec4(1.0, 1.0, 0.8, 1.0);
        }
        else
        {
            vec3 hitpos = ray.origin + t * ray.direction;

            vec3 center =
                (hitId == 1) ? earth.center : moon.center;

            vec3 baseTint =
                (hitId == 1) ? vec3(0.5, 1.0, 1.0)
                            : vec3(0.5);

            vec3 n = normalize(hitpos - center);
            vec3 l = normalize(light.position - hitpos);

            float ambient = 0.1;
            float diff = max(dot(n, l), 0.0);
            float intensity = max(diff, ambient);

            FragColor = vec4(baseTint * intensity, 1.0);
        }

    }

)"