#version 150
uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture; // The sampler to read in the texture
uniform int u_Time;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them

in vec2 fs_UV;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

#define DISPLACE .125

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                 dot(p, vec2(269.5,183.3))))* 43758.5453);
}

float WorleyNoise(vec2 uv) {
    uv *= 10; // Change the space to 10 * 10
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            vec2 neighbor = vec2(float(x), float(y)); // Direction in which neighbor cell lies
            vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighbor’s Voronoi point
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

//Calculate the squared length of a vector
float length2(vec2 p){
    return dot(p,p);
}


float fworley(vec2 p) {
    //Stack noise layers
        return sqrt(sqrt(sqrt(
                WorleyNoise(p * 0.8 + 0.0005 * u_Time) *
                sqrt(WorleyNoise(p * 0.98 + 0.12 + -0.001 * u_Time)) *
                sqrt(sqrt(WorleyNoise(p * -0.99976 + 0.0003 * u_Time))))));
}


void main(void)
{
    vec4 original = texture(u_Texture, fs_UV);
    // If we are in Water or Lava
    if (abs(u_Color.w - 1) < 0.0001) {
        // make a copy of fs_UV
        vec2 uv = fs_UV.xy;
        // Calculate the t for corrdinates change
        float t = fworley(vec2((uv.x / 12.0), (uv.y / 18.0)));

        uv.y += cos(0.99754 * t - 0.2674) - 0.87645;
        uv.x -= 0.5 * (cos(0.99754 * t - 0.2674) - 0.87645);

        original = texture(u_Texture, uv);
    }

    out_Col = original * u_Color;
}
