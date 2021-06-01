# Mini Minecraft
This is a group project to create an interactive 3D world exploration and alteration program in the style of the popular computer game Minecraft. 
## My Contribution 
### Efficient Terrain Rendering and Chunking
Aim: Only render the faces at the visible surface.


1. Make Chunk Class inherit from Drawable and add the creat() function.

2. Write a helper function to get the hard-code color of a certain BlockType.

3. Set up a struct to store the neighbor's direction, offset, normal and vertex list to create a quadrangle face. And use a array to store all six neighbors.

4. In the create() function, iterate over all the blocks in a chunk(16x16x256). For each block, if it's not empty then iterate over all the neighbors. If the neighbor is empty, add the corresponding quadrangle face vertices to a list. Meanwhile, add this face's normal and color to the list to formulate a interleaved information list for passing data to GPU. Also, the vertex index list should be properly set.

5. Make some modification in ShaderProgram.cpp to make sure it recognize this interleaved list of data. Make sufficient changes in the code to support transferring the data to GPU by VBO.

6. Then write a function in terrain.cpp to support the function of game world expansion. By checking the nearest 2*16 positions, if the position is not occupied by a chunk, then add a chunk.

7. The chunks are created only once in runtime. Each one of them are created in function Terrain::instantiateChunkAt() when they are created.
### Texturing and Texture Animation
Aim: Adding texture to the building blocks.


1. Read in the texture file with a function in shaderprogram into GPU, link it with a Sample2D for use in a shader.

2. Append the UV data as a vec4 to the interleaved vector. 
This is done with the suggestion by Prof.Mally to build a vector of Materials and corresponding UV coordinates. Taking advantage of all the data positions in this vec4, let the first two be the UV coord, the third data as whether to animate and the fourth data as whether is opaque(not used).


3. Pass this data as VBO and reserve place for this data on GPU in shaderprogram.cpp.

4. In the shader, read the texture and apply it as a color to the final render. For animatable material like water and lava, use a sin function to read the texture smoothly along the "U" coordinate as if it were animated.

5. Copy paste the previous function for building the Transparent VBO. Set up some handles in the drawable and shaderprogram to support 2 VBOs for one 
Drawable object. Then pass render the 2 VBOs in sequence. First draw the opaque one, the draw the transparent one. For the transparent one, enable the blend function from OpenGL and disable it after this draw. 


### Post-process Camera Overlay , Water waves , Greyscale/Color image file as height map
Aim: Create vivid fluid lake and support loading custom image to change landscape.


1. Post-process Camera Overlay: To implement this feature, the current scene rendered is stored to a framebuffer and stored as a Texture in GPU. Then when player enters a fluid block such as water or lava, by using the post process shading technique, I apply some custom noise to the UV coordinates when using the texture() functiion to read the rendered scene as texture. The difficulty would be passing the texture correctly and making proper distortion to the UV coordinates. 

2. Water waves and proper shading: For fluid like water behavior, I applied some changes to the vertex shader. By changing the y value of gl_position, we can change the output height of certain block. Since I reserved some memory space when passing the UV data (vec4 vs vec2) in building the interleaved VBO, I used the last two floats in UV data to represent the fluidity and transparency of the block. Then even in the vertex shader I could tell what kind of block I'm facing. As a consequence, I can properly apply the fluid behavior to fluids. Then I used Blinn-Phong shading to deal with the high light for the fluid blocks. To make this work, I also applied distortions to the Normal of the fluid blocks with respect to their change in height y value.

3. Reading image as height map: To read in a image, I modified the GUI to support using Ctrl+O to open a color image and using Ctrl+I to open a greyscale image. The difficulty of this work would be mapping the image with chunks correctly. The origin of the image for getPixelColor function is the upper left corner but the origin of the chunk is the bottom left corner. So I spend some time to deal with the iteration carefully. The other difficulty would be generating new chunk area to match the image if there were no chunks. This step is help with Wangjin with his multi-thread system. The greyscale image mapping is straight forward that the greyscale value is directly mapped to height of the blocks. For the color map, there are two steps. First is to change it to grayscale, which is already learnd in the shader fun HW. Second is to match the color image with proper block type. I used a color difference comparison that matches the similar color composition blocks with the color image.

### Demonstration Video Link
Milestone 1:[Youtube](https://www.youtube.com/watch?v=on543jdF1_M)
<iframe width="896" height="504" src="https://www.youtube.com/watch?v=on543jdF1_M" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
Milestone 2:[Youtube](https://www.youtube.com/watch?v=gHd7pCU9_6I)
<iframe width="896" height="504" src="https://www.youtube.com/watch?v=gHd7pCU9_6I" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
Milestone 3:[Youtube](https://www.youtube.com/watch?v=uU9GbxQBwPU)
<iframe width="896" height="504" src="https://www.youtube.com/watch?v=uU9GbxQBwPU" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

