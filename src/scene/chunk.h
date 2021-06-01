#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "noise.h"
#include <mutex>
//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, SNOW, WATER, LAVA, ICE, SAND, WOOD, LEAVE
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// Using a struct to store all the neighbors information for a block
struct blockNeighbor {
    Direction dir;
    glm::ivec3 offset;
    glm::ivec3 normal;
    // The offset amount after a pos adding the offset
    std::vector <glm::ivec3> quadPosList;
    std::vector <glm::vec2> UVCord;
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine

public:
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    bool ready;

    static Noise noise;
    static std::mutex mtx;
    Chunk(OpenGLContext* context);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z);
    BlockType getBlockAt(int x, int y, int z);
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    void createLandscape(int x, int z);
    void createLandscape(int x, int z, QImage *img, bool isColor);
    void setChunkVBO(std::vector<glm::vec4> iPosCorNorList, std::vector<GLuint> iIdx);
    void setChunkInterleave();
    void setChunkTransVBO(std::vector<glm::vec4> iPosCorNorList, std::vector<GLuint> iIdx);
    void setChunkTransInterleave();
    void createTrees(int x, int y, int z, int randX, int randY, int randZ);
    void createIces(int x, int y, int z, int randX, int randY, int randZ);
    virtual void create();
    virtual GLenum drawMode();
    bool readyToDraw();
    bool readyToDraw(bool);
    void passVBO();
    // Use a array to store all the neighors of a certain block
    std::array<blockNeighbor, 6> neighbors = {
        blockNeighbor{XPOS, glm::ivec3(1,0,0), glm::ivec3(1,0,0),
                      std::vector{glm::ivec3(1,0,0),glm::ivec3(1,1,0), glm::ivec3(1,1,1), glm::ivec3(1,0,1)},
                      std::vector{glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625), glm::vec2(0,0.0625), glm::vec2(0,0)}},

        blockNeighbor{XNEG, glm::ivec3(-1,0,0), glm::ivec3(-1,0,0),
                      std::vector{glm::ivec3(0,1,1),glm::ivec3(0,0,1), glm::ivec3(0,0,0), glm::ivec3(0,1,0)},
                      std::vector{glm::vec2(0,0.0625), glm::vec2(0,0), glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625)}},

        blockNeighbor{YPOS, glm::ivec3(0,1,0), glm::ivec3(0,1,0),
                      std::vector{glm::ivec3(0,1,0),glm::ivec3(1,1,0), glm::ivec3(1,1,1), glm::ivec3(0,1,1)},
                      std::vector{glm::vec2(0,0), glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625), glm::vec2(0,0.0625)}},

        blockNeighbor{YNEG, glm::ivec3(0,-1,0), glm::ivec3(0,-1,0),
                      std::vector{glm::ivec3(0,0,0),glm::ivec3(0,0,1), glm::ivec3(1,0,1), glm::ivec3(1,0,0)},
                      std::vector{glm::vec2(0,0), glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625), glm::vec2(0,0.0625)}},

        blockNeighbor{ZPOS, glm::ivec3(0,0,1), glm::ivec3(0,0,1),
                      std::vector{glm::ivec3(0,0,1),glm::ivec3(1,0,1), glm::ivec3(1,1,1), glm::ivec3(0,1,1)},
                      std::vector{glm::vec2(0,0), glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625), glm::vec2(0,0.0625)}},

        blockNeighbor{ZNEG, glm::ivec3(0,0,-1), glm::ivec3(0,0,-1),
                      std::vector{glm::ivec3(0,1,0),glm::ivec3(0,0,0), glm::ivec3(1,0,0), glm::ivec3(1,1,0)},
                      std::vector{glm::vec2(0,0.0625), glm::vec2(0,0), glm::vec2(0.0625,0), glm::vec2(0.0625, 0.0625)}}
    };
    // Use a interleaved vector to store all the information for a vertex
    // the sequence is pos-col-nor
    // Begin from M2 is pos-col-nor-UV
    std::vector<GLuint> mPosIndices;
    std::vector<glm::vec4> mPosColNorList;

    std::vector<GLuint> mTransPosIndices;
    std::vector<glm::vec4> mTransPosColNorList;
    // Use a map to store all the UV mapping coordinates
    std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaceUVs {
        {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f/16.f, 15.f/ 16.f)},
                                                                   {XNEG, glm::vec2(3.f/16.f, 15.f/ 16.f)},
                                                                   {YPOS, glm::vec2(8.f/16.f, 13.f/ 16.f)},
                                                                   {YNEG, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(3.f/16.f, 15.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(3.f/16.f, 15.f/ 16.f)}}},
        {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{ {XPOS, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {XNEG, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {YPOS, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {YNEG, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(2.f/16.f, 15.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(2.f/16.f, 15.f/ 16.f)}}},
        {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f/16.f, 15.f/ 16.f)},
                                                                   {XNEG, glm::vec2(1.f/16.f, 15.f/ 16.f)},
                                                                   {YPOS, glm::vec2(1.f/16.f, 15.f/ 16.f)},
                                                                   {YNEG, glm::vec2(1.f/16.f, 15.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(1.f/16.f, 15.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(1.f/16.f, 15.f/ 16.f)}}},
        {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{ {XPOS, glm::vec2(2.f/16.f, 11.f/ 16.f)},
                                                                   {XNEG, glm::vec2(2.f/16.f, 11.f/ 16.f)},
                                                                   {YPOS, glm::vec2(2.f/16.f, 11.f/ 16.f)},
                                                                   {YNEG, glm::vec2(2.f/16.f, 11.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(2.f/16.f, 11.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(2.f/16.f, 11.f/ 16.f)}}},
        {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f/16.f, 3.f/ 16.f)},
                                                                   {XNEG, glm::vec2(13.f/16.f, 3.f/ 16.f)},
                                                                   {YPOS, glm::vec2(13.f/16.f, 3.f/ 16.f)},
                                                                   {YNEG, glm::vec2(13.f/16.f, 3.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(13.f/16.f, 3.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(13.f/16.f, 3.f/ 16.f)}}},
        {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{ {XPOS, glm::vec2(13.f/16.f, 1.f/ 16.f)},
                                                                   {XNEG, glm::vec2(13.f/16.f, 1.f/ 16.f)},
                                                                   {YPOS, glm::vec2(13.f/16.f, 1.f/ 16.f)},
                                                                   {YNEG, glm::vec2(13.f/16.f, 1.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(13.f/16.f, 1.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(13.f/16.f, 1.f/ 16.f)}}},
        {ICE, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(3.f/16.f, 11.f/ 16.f)},
                                                                   {XNEG, glm::vec2(3.f/16.f, 11.f/ 16.f)},
                                                                   {YPOS, glm::vec2(3.f/16.f, 11.f/ 16.f)},
                                                                   {YNEG, glm::vec2(3.f/16.f, 11.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(3.f/16.f, 11.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(3.f/16.f, 11.f/ 16.f)}}},
        {SAND, std::unordered_map<Direction, glm::vec2, EnumHash>{ {XPOS, glm::vec2(2.f/16.f, 14.f/ 16.f)},
                                                                   {XNEG, glm::vec2(2.f/16.f, 14.f/ 16.f)},
                                                                   {YPOS, glm::vec2(2.f/16.f, 14.f/ 16.f)},
                                                                   {YNEG, glm::vec2(2.f/16.f, 14.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(2.f/16.f, 14.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(2.f/16.f, 14.f/ 16.f)}}},
        {WOOD, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(4.f/16.f, 14.f/ 16.f)},
                                                                   {XNEG, glm::vec2(4.f/16.f, 14.f/ 16.f)},
                                                                   {YPOS, glm::vec2(4.f/16.f, 14.f/ 16.f)},
                                                                   {YNEG, glm::vec2(4.f/16.f, 14.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(4.f/16.f, 14.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(4.f/16.f, 14.f/ 16.f)}}},
        {LEAVE, std::unordered_map<Direction, glm::vec2, EnumHash>{  {XPOS, glm::vec2(5.f/16.f, 12.f/ 16.f)},
                                                                   {XNEG, glm::vec2(5.f/16.f, 12.f/ 16.f)},
                                                                   {YPOS, glm::vec2(5.f/16.f, 12.f/ 16.f)},
                                                                   {YNEG, glm::vec2(5.f/16.f, 12.f/ 16.f)},
                                                                   {ZPOS, glm::vec2(5.f/16.f, 12.f/ 16.f)},
                                                                   {ZNEG, glm::vec2(5.f/16.f, 12.f/ 16.f)}}}
    };

};
