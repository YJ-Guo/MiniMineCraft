#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture; // The sampler to read in the texture

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UV;
in float fs_Trans;           // Tells if the vertex is Opaque, 1 for opaque
in float fs_Fluid;            // Tells if the vertex is fluid, 1 for fluid
in vec4 fs_CameraPos;         // Used for computing blinn-phong shading

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.f;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

void main()
{
    // Material base color (before shading)
//        vec4 diffuseColor = fs_Col;
       vec4 diffuseColor = texture(u_Texture, fs_UV);

        diffuseColor = diffuseColor * (0.5 * fbm(fs_Pos.xyz) + 0.5);

        float fogDistance = distance(fs_Pos.y, fs_CameraPos.y);

        vec4 fogBase = vec4(0.824f, 0.824f, 0.824f, 1.f);

//        if (fogDistance < 300 && fogDistance > 100 && abs(fs_Trans - 1) < 0.0001) {
//            diffuseColor = vec4(vec3(fogDistance * (clamp(fogBase - diffuseColor, 0.f, 1.f)) / 300.f +  diffuseColor), 1);
//        } else if (fogDistance >= 300) {
//            diffuseColor = fogBase;
//        }

        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);

        // Add Blinn-Phong Specular term
        vec4 V = fs_CameraPos - fs_Pos;                     // The view vector
        vec4 H = (V + fs_LightVec) / 2.f;                   // The bisector of view vector and light vector

        // The exponention term in Blinn_Phong Model
        float exp = 100;
        float specularIntensity = 0.f;
        if (abs(fs_Fluid - 1) < 0.001){
            float specularIntensity = max(pow(dot(normalize(H), normalize(fs_Nor)), exp), 0);
        }

        float ambientTerm = 0.2;

        //Add a small float value to the color multiplier
        //to simulate ambient lighting. This ensures that faces that are not
        //lit by our point light are not completely black.
        float lightIntensity = diffuseTerm + ambientTerm + specularIntensity;


        // Compute final shaded color
        out_Col = vec4(clamp(diffuseColor.rgb * lightIntensity,0,1), diffuseColor.a);

}
