#include "chunk.h"
#include <iostream>
Noise Chunk::noise = Noise();
std::mutex Chunk::mtx = std::mutex();
Chunk::Chunk(OpenGLContext* context) : Drawable(context), m_blocks(),
    m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
    ready(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) {
    // For blocks at the edge of a chuck
    // check the neighbor chunk for certain block type
    if (y < 0 || y > 255) {
        return EMPTY;
    }
    if ( x < 0 && m_neighbors[XNEG] != nullptr) {
        return m_neighbors[XNEG]->getBlockAt(15, y, z);
    }

    if ( x > 15 && m_neighbors[XPOS] != nullptr) {
        return m_neighbors[XPOS]->getBlockAt(0, y, z);
    }

    if ( z < 0 && m_neighbors[ZNEG] != nullptr) {
        return m_neighbors[ZNEG]->getBlockAt(x, y, 15);
    }

    if ( z > 15 && m_neighbors[ZPOS] != nullptr) {
        return m_neighbors[ZPOS]->getBlockAt(x, y, 0);
    }

    // For the coner cases, draw the face anyway
    if (x < 0 || x > 15 || y < 0 || y > 255 || z < 0 || z > 15) {
        return EMPTY;
    }

    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    mtx.lock();
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
    mtx.unlock();
}


GLenum Chunk::drawMode() {
    return GL_TRIANGLES;
}

glm::vec4 getColor(BlockType type) {
    glm::vec4 resColor(0,0,0,1);
    if (type != EMPTY) {
        switch (type) {
        case GRASS:
            resColor = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
            break;
        case DIRT:
            resColor = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
            break;
        case STONE:
            resColor = glm::vec4(0.5f);
            break;
        case SNOW:
            resColor = glm::vec4(1.f, 1.f, 1.f ,0.f);

        default:
            break;
        }
    }
    return resColor;
}

// Return 1 for animatable types and 0 for otherwise
int IsAnimatable(BlockType type) {
    if (type == LAVA || type == WATER) {
        return 1;
    }
    return 0;
}

// Return true is the tested block is opaque
bool IsOpaque(BlockType type) {
    if (type == WATER || type == ICE) {
        return false;
    }
    return true;
}

void Chunk::setChunkInterleave() {
    mPosColNorList.clear();
    mPosIndices.clear();
    int indexCounter = 0;
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 256; ++y) {
            for (int z = 0; z < 16; ++z) {
                // Iterate over all the
                if (getBlockAt(x, y, z) != EMPTY && IsOpaque(getBlockAt(x, y, z))) {
                    glm::vec4 blockColor = getColor(getBlockAt(x, y, z));
                    // Iterate over all the neighbors of this block to find the vertices that
                    // need to be rendered (facing out)
                    // First check for all the corner case
                    for (blockNeighbor &eachNeighbor : neighbors) {
                        glm::ivec3 neighborPos(glm::ivec3(x,y,z) + eachNeighbor.offset);
                        // Render face for empty and transparent block
                        if (getBlockAt(neighborPos.x, neighborPos.y, neighborPos.z) == EMPTY
                                || !IsOpaque(getBlockAt(neighborPos.x, neighborPos.y, neighborPos.z))) {
                            for (int i = 0; i < 4; ++i) {
                                // Append Position
                                mPosColNorList.push_back(glm::vec4(glm::ivec3(x,y,z) + eachNeighbor.quadPosList[i], 1));
                                // Append Color
                                mPosColNorList.push_back(blockColor);
                                // Append Normal
                                mPosColNorList.push_back(glm::vec4(eachNeighbor.normal, 0));
                                // Append UV coord
                                mPosColNorList.push_back(glm::vec4(eachNeighbor.UVCord[i] + blockFaceUVs[getBlockAt(x,y,z)][eachNeighbor.dir],
                                        IsAnimatable(getBlockAt(x,y,z)),1));
                            }

                            // Add the index for pos to form 2 triangles
                            mPosIndices.push_back(indexCounter);
                            mPosIndices.push_back(indexCounter + 1);
                            mPosIndices.push_back(indexCounter + 2);
                            mPosIndices.push_back(indexCounter);
                            mPosIndices.push_back(indexCounter + 2);
                            mPosIndices.push_back(indexCounter + 3);
                            indexCounter += 4;
                        }
                    }
                }
            }
        }
    }
}

void Chunk::setChunkVBO(std::vector<glm::vec4> iPosCorNorList, std::vector<GLuint> iIdx) {
    m_count = iIdx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, iIdx.size() * sizeof(GLuint), iIdx.data(), GL_STATIC_DRAW);

    generatePosColNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosColNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, iPosCorNorList.size() * sizeof(glm::vec4), iPosCorNorList.data(), GL_STATIC_DRAW);
}

void Chunk::create() {
    setChunkInterleave();
    setChunkVBO(mPosColNorList, mPosIndices);
    setChunkTransInterleave();
    setChunkTransVBO(mTransPosColNorList, mTransPosIndices);
}

void Chunk::passVBO() {
    setChunkVBO(mPosColNorList, mPosIndices);
    setChunkTransVBO(mTransPosColNorList, mTransPosIndices);
}
bool Chunk::readyToDraw() {
    return this->ready;
}

bool Chunk::readyToDraw(bool r) {
    this->ready = r;
    return r;
}
float computeColorDistance(QColor q1, QColor q2) {
    float d = glm::sqrt((q1.red() - q2.red())* (q1.red() - q2.red()) +
                        (q1.green() - q2.green())* (q1.green() - q2.green()) +
                        (q1.blue() - q2.blue())* (q1.blue() - q2.blue())) / 442.f;
    return d;
}

void Chunk::createLandscape(int x, int z, QImage *img, bool isColor) {


        for (int i = 0; i < 16 ; ++i) {
            for (int j = 0; j < 16; ++j) {
                BlockType bt = STONE;
                int height;
                if(isColor) {
                    height = glm::clamp((img->pixelColor(x+i, z - j + 15).red() * 11 +  img->pixelColor(x+i, z - j + 15).green() * 16+
                             img->pixelColor(x+i, z - j + 15).blue() * 5) / 32, 10, 255);
                    float distance = 100000000.f;
                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(255, 255, 255)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(255, 255, 255));
                        bt = SNOW;
                    }

                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(150, 70, 0)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(150, 70, 0));
                        bt = DIRT;
                    }

                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(76, 153, 0)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(76, 153, 0));
                        bt = GRASS;
                    }

                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(128, 128, 128)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(128, 128, 128));
                        bt = STONE;
                    }

                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(51, 153, 255)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(51, 153, 255));
                        bt = WATER;
                    }

                    if(computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(255, 0, 0)) < distance) {
                        distance = computeColorDistance(img->pixelColor(x + i, z - j + 15), QColor(255, 255, 255));
                        bt = LAVA;
                    }


                } else {
                    height = glm::clamp(img->pixelColor(x + i, z - j + 15).red(), 1, 255);
                }

    //            int height = 128;
                for (int y = height; y < 256; ++y) {
                    this->setBlockAt(i,y,j, EMPTY);
                }
                for (int y = 0; y < height; ++y) {
                    // Set all the blocks in height map as stone
                    this->setBlockAt(i,y,j, bt);
                }
            }
        }


}

void Chunk::createLandscape(int x, int z) {
    int biome;
    int h;
    float random = noise.simpleNoise(glm::vec2(x * 1213, z * 321)).x;
    int treePositionx = random * 7 + 4;
    int treePositionz = random * 3 + 5;

    for (int i = 0; i < 16 ; ++i) {
        for (int j = 0; j < 16; ++j) {
            // get the height
            glm::ivec2 n = noise.getHight(x + i, z + j);
            biome = n.x;
            h = n.y;
            // for safety purpose. clamp it.
            h = glm::clamp(0, 255, h);
            if(biome == 2) {
                for(int y = 0; y <= h; y++) {
                    if(y <= 128 ) {
                        // 0 - 128 set as stone
                        setBlockAt(i, y, j, STONE);
                    } else {
                                setBlockAt(i, y, j, SAND);

                      }
                    }

            } else if(biome == 3) {
                for(int y = 0; y <= h; y++) {
                    if(y <= 128 ) {
                        // 0 - 128 set as stone
                        setBlockAt(i, y, j, STONE);
                    } else {
                                setBlockAt(i, y, j, SNOW);

                      }
                    }

                if(random > 0.85 && i == treePositionx && j == treePositionz) {

                        createIces(i, h, j, x, h, z);

                }

            } else if( biome == 4) {
                for(int y = 0; y <= h || y <= 128; y++) {
                    if(y <= h) {
                        setBlockAt(i, y, j, SAND);
                    } else {
                        setBlockAt(i, y, j, WATER);
                    }
                }
            } else {

                for(int y = 0; y <= h; y++) {
                    if(y <= 128 ) {
                        // 0 - 128 set as stone
                        setBlockAt(i, y, j, STONE);
                    } else {
                        if(h <= 155) {
                            // 129 - 155 set as grass
                            if(y == h) {
                                setBlockAt(i, y, j, GRASS);
                                continue;
                            }
                            setBlockAt(i, y, j, DIRT);
                        } else {
                            // 156 - 199 set as stone, if > 200, set the top one as snow.
                             if(y >= 200 && y == h) {
                                 setBlockAt(i, y, j, SNOW);
                             } else {
                                 setBlockAt(i, y, j, STONE);
                             }
                        }
                    }
                }

                if(random > 0.4 && i == treePositionx && j == treePositionz) {
                    if(h < 140) {
                        createTrees(i, h, j, x, h, z);
                    }
                }
}


        }
    }





}

void Chunk::createIces(int x, int y, int z, int randX, int randY, int randZ) {
    int randomHeight = glm::floor(glm::clamp(noise.simpleNoise(glm::vec2(randX, randZ)).x * 32, 4.f, 32.f));

    for(int h = 0; h < randomHeight; h++) {
        for(int i = x - 4; i < x + 4; i++) {
            for(int j =  z - 4; j < z + 4; j++) {
                if((i-x) * (i-x) + (j-z) * (j-z) <= (randomHeight - h) * 0.2) {
                    setBlockAt(i, y + h , j, ICE);
                }
            }
        }
    }
}
void Chunk::createTrees(int x, int y, int z, int randX, int randY, int randZ) {

    int randomHeight = glm::floor(glm::clamp(noise.simpleNoise(glm::vec2(randX, randZ)).x * 8, 4.f, 8.f));
//    std::cout << noise.simpleNoise(glm::vec2(x, z)).x << std::endl;
    for(int i = 0; i < randomHeight + 4; i++) {
        setBlockAt(x, y + i, z, WOOD);
    }
//    setBlockAt(x, y, z, WOOD);
//    setBlockAt(x, y + 1, z, WOOD);
//    setBlockAt(x, y + 2, z, WOOD);
//    setBlockAt(x, y + 3, z, WOOD);
    int rand = noise.simpleNoise(glm::vec2(randX, randY + 1)).x * 2;
    for(int h = 1; h < 5; h++) {
        for(int i = x - h - rand; i <= x + h + rand; i++) {
            for(int j = z - h - rand; j <= z + h + rand; j ++) {
                if(noise.simpleNoise(glm::vec2(i + randomHeight + h + randX, j + randomHeight + h + randZ)).x > 0.6) {
                    setBlockAt(i, y + randomHeight + h -1, j, LEAVE);
                }

            }
        }
    }
    for(int h = 4; h >= 0; h--) {
        for(int i = x - h - rand; i <= x + h + rand; i++) {
            for(int j = z - h - rand; j <= z + h + rand; j ++) {
                if(noise.simpleNoise(glm::vec2(i + randomHeight + 8  - h + randX, j + randomHeight + 8 - h + randZ)).x > 0.6) {
                    setBlockAt(i, y + randomHeight + 8 - h, j, LEAVE);
                }

            }
        }
    }


//    rand = noise.simpleNoise(glm::vec2(randX, randY + 4)).x * 2;
//    for(int i = x -2 - rand; i <= x + 2 + rand; i++) {
//        for(int j = z - 2 - rand; j <= z + 2 + rand; j ++) {
//            setBlockAt(i, y + randomHeight + 3, j, LEAVE);
//        }
//    }
//    rand = noise.simpleNoise(glm::vec2(randX, randY + 5)).x * 2;
//    for(int i = x -1- rand; i <= x + 1 + rand; i++) {
//        for(int j = z - 1 - rand; j <= z + 1 + rand; j ++) {
//            setBlockAt(i, y + randomHeight + 4, j, LEAVE);
//        }
//    }

}

// New for M2
// Set up VBO data for transparent blocks
void Chunk::setChunkTransInterleave() {
    mTransPosColNorList.clear();
    mTransPosIndices.clear();
    int indexCounter = 0;
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 256; ++y) {
            for (int z = 0; z < 16; ++z) {
                // Iterate over all the transparent blocks
                if (getBlockAt(x, y, z) != EMPTY && !IsOpaque(getBlockAt(x, y, z))) {
                    glm::vec4 blockColor = getColor(getBlockAt(x, y, z));
                    // Iterate over all the neighbors of this block to find the vertices that
                    // need to be rendered (facing out)
                    // First check for all the corner case
                    for (blockNeighbor &eachNeighbor : neighbors) {
                        glm::ivec3 neighborPos(glm::ivec3(x,y,z) + eachNeighbor.offset);
                        // Render face for empty and transparent block
                        if (getBlockAt(neighborPos.x, neighborPos.y, neighborPos.z) == EMPTY) {
                            for (int i = 0; i < 4; ++i) {
                                // Append Position
                                mTransPosColNorList.push_back(glm::vec4(glm::ivec3(x,y,z) + eachNeighbor.quadPosList[i], 1));
                                // Append Color
                                mTransPosColNorList.push_back(blockColor);
                                // Append Normal
                                mTransPosColNorList.push_back(glm::vec4(eachNeighbor.normal, 0));
                                // Append UV coord
                                mTransPosColNorList.push_back(glm::vec4(eachNeighbor.UVCord[i] + blockFaceUVs[getBlockAt(x,y,z)][eachNeighbor.dir],
                                        IsAnimatable(getBlockAt(x,y,z)),0));
                            }

                            // Add the index for pos to form 2 triangles
                            mTransPosIndices.push_back(indexCounter);
                            mTransPosIndices.push_back(indexCounter + 1);
                            mTransPosIndices.push_back(indexCounter + 2);
                            mTransPosIndices.push_back(indexCounter);
                            mTransPosIndices.push_back(indexCounter + 2);
                            mTransPosIndices.push_back(indexCounter + 3);
                            indexCounter += 4;
                        }
                    }
                }
            }
        }
    }
}

void Chunk::setChunkTransVBO(std::vector<glm::vec4> iPosCorNorList, std::vector<GLuint> iIdx) {
    m_Transcount = iIdx.size();

    generateTransIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, iIdx.size() * sizeof(GLuint), iIdx.data(), GL_STATIC_DRAW);

    generateTransPosColNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransPosColNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, iPosCorNorList.size() * sizeof(glm::vec4), iPosCorNorList.data(), GL_STATIC_DRAW);
}
