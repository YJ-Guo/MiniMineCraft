#version 150
uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture; // The sampler to read in the texture

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them

in vec2 fs_UV;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main(void)
{
    vec4 original = texture(u_Texture, fs_UV);
    out_Col = original*u_Color;
}
