#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
// object we're rendering. In this assignment,
// this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
// This allows us to transform the object's normals properly
// if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
// We've written a static matrix for you to use for HW2,
// but in HW3 you'll have to generate one yourself

uniform vec3 u_CameraPos;

uniform int u_Time;         // A uniform time variable for making animated texture

uniform sampler2D u_Texture; // The sampler to read in the texture

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

in vec4 vs_UV;              // The array of vertex UVs passed to the shader. The third bit is animate or not. The last bit is opaque or not.

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_UV;
out float fs_Trans;           // Tells if the vertex is Opaque, 1 for opaque
out float fs_Fluid;

out vec4 fs_CameraPos; // Used for computing blinn-phong shading

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
// the geometry in the fragment shader.

vec2 random2( vec2 p ) {
    return fract(sin(0.0000001 * vec2(dot(p, vec2(127.1, 311.7)),
                 dot(p, vec2(269.5,183.3))))* 43758.5453);
}

void main()
{
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_Trans = vs_UV.w;
    fs_Fluid = vs_UV.z;
    // If the input UV Coord is Water or Lava
    if (abs(fs_Fluid - 1) < 0.0001) {
        fs_UV = vs_UV.xy + vec2(0.1250 * abs(sin(u_Time / 10)), 0);
    } else {
        // For static bolcks
        fs_UV = vs_UV.xy;
    }

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.

    fs_CameraPos = vec4(u_CameraPos, 1);

    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    fs_Pos = modelposition;

//    if (abs(fs_Fluid - 1) < 0.0001) {
//        fs_LightVec = fs_CameraPos - modelposition;
//    } else {
//        fs_LightVec = (lightDir);  // Compute the direction in which the light source lies
//    }
    fs_LightVec = (lightDir);

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                              // used to render the final positions of the geometry's vertices

    if (abs(fs_Fluid - 1) < 0.0001) {
        vec2 randomXZ = random2(vec2(modelposition.x + modelposition.z));
        float randomFluid = randomXZ.x + randomXZ.y + 0.5 * sin((modelposition.x + modelposition.z + u_Time/ 20.f) / 2.0) + 0.1 * cos(modelposition.x - modelposition.z);
        gl_Position.y += randomFluid;
        fs_Nor.z += randomFluid;
        fs_Nor.x += randomFluid;
    }

}
