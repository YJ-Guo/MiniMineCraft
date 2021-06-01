#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), chunksWaitlist(), mutex_chunks(),mutex_waitList(), m_generatedTerrain(),m_ChunksWithoutVBO(), m_ChunksWithVBO(), mut_withoutVBO(), mut_withVBO(),
      mp_context(context), noise(), threads()/*, m_geomCube(context)*/
{}

Terrain::~Terrain() {
    // m_geomCube.destroy();
    // tranverse m_chunks to destory all the chunks?
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z)
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Get block at Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p)  {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z)  {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).

    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    this->mutex_chunks.lock();
    bool res =  m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
    this->mutex_chunks.unlock();
    return res;
}

bool Terrain::hasZoneAt(int x, int z) {
//    std::cout << "has zone at" << std::endl;
    int xFloor = static_cast<int>(glm::floor(x / 64.f));
    int zFloor = static_cast<int>(glm::floor(z / 64.f));
    return m_generatedTerrain.find(toKey(64 * xFloor, 64 * zFloor)) != m_generatedTerrain.end();
}

uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    mutex_chunks.lock();
    uPtr<Chunk>& temp =  m_chunks[toKey(16 * xFloor, 16 * zFloor)];
    mutex_chunks.unlock();
    return temp;
}


//const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
//    int xFloor = static_cast<int>(glm::floor(x / 16.f));
//    int zFloor = static_cast<int>(glm::floor(z / 16.f));
//    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
//}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Set block at Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}
// for a given xz, cooodinate, create the column
//void Terrain::createLandscape(int x, int z) {
//    std::cout << "in create landescape" << std::endl;
//    for (int i = 0; i < 16 ; ++i) {
//        for (int j = 0; j < 16; ++j) {
//            // get the height
//            int h = noise.getHight(x + i, z + j);
//            // for safety purpose. clamp it.
//            h = glm::clamp(0, 255, h);
//            for(int y = 0; y <= h; y++) {
//                if(y <= 128 ) {
//                    // 0 - 128 set as stone
//                    setBlockAt(x + i, y, z + j, STONE);
//                } else {
//                    if(h <= 155) {
//                        // 129 - 155 set as grass
//                        if(y == h) {
//                            setBlockAt(x + i, y, z + j, GRASS);
//                            continue;
//                        }
//                        setBlockAt(x + i, y, z + j, DIRT);
//                    } else {
//                        // 156 - 199 set as stone, if > 200, set the top one as snow.
//                         if(y >= 200 && y == h) {
//                             setBlockAt(x + i, y, z + j, SNOW);
//                         } else {
//                             setBlockAt(x + i, y, z + j, STONE);
//                         }
//                    }
//                }
//            }
//        }
//    }
////    std::cout << "end create landescape" << std::endl;
//}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();
    mutex_chunks.lock();
    m_chunks[toKey(x, z)] = move(chunk);
    mutex_chunks.unlock();
    // Add color for init scene
    cPtr->createLandscape(x, z);
    // Set the neighbor pointers of itself and its neighbors

    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(ShaderProgram *shaderProgram) {
    mutex_chunks.lock();
    for (auto &eachChunk : m_chunks){
        if(!eachChunk.second->readyToDraw()) {
            continue;
        }
        glm::ivec2 chunkCoord = toCoords(eachChunk.first);
        shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(chunkCoord.x, 0, chunkCoord.y)));
        shaderProgram->drawInterleaved(*eachChunk.second.get());

    }
    mutex_chunks.unlock();
}

void Terrain::drawTrans(ShaderProgram *shaderProgram) {
    mutex_chunks.lock();
    for (auto &eachChunk : m_chunks){
        //eachChunk.second->create();
        if(!eachChunk.second->readyToDraw()) {
            continue;
        }
        glm::ivec2 chunkCoord = toCoords(eachChunk.first);
        shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(chunkCoord.x, 0, chunkCoord.y)));
        shaderProgram->drawTransInterleaved(*eachChunk.second.get());
    }
    mutex_chunks.unlock();
}

void Terrain::UpdateNeighborChunks(int x, int z) {
    // Recreate the 4 neighbors of a chunk after chunk generation
    if(hasChunkAt(x, z + 16)) {
        m_chunks[toKey(x, z + 16)]->destroy();
        m_chunks[toKey(x, z + 16)]->create();
    }
    if(hasChunkAt(x, z - 16)) {
        m_chunks[toKey(x, z - 16)]->destroy();
        m_chunks[toKey(x, z - 16)]->create();
    }
    if(hasChunkAt(x + 16, z)) {
        m_chunks[toKey(x + 16, z)]->destroy();
        m_chunks[toKey(x + 16, z)]->create();
    }
    if(hasChunkAt(x - 16, z)) {
        m_chunks[toKey(x - 16, z)]->destroy();
        m_chunks[toKey(x - 16, z)]->create();
    }
}

void Terrain::UpdateTerrain(const glm::vec3 &playerPos) {
    // Get player's x and z coord
    int x = playerPos.x;
    int z = playerPos.z;


    for (int i = -2; i < 3; i++) {
        for (int j = -2; j < 3; j++) {
            int xEdge = x + i * 64;
            int zEdge = z + j * 64;
            if(!hasZoneAt(xEdge, zEdge)) {
//                std::cout << "begin to create blockTypeWoker"<< std::endl;
                createBlockTypeWorker(xEdge, zEdge);
//                std::cout << "begin to create blockTypeWoker end"<< std::endl;
            }
        }
    }

    // lock and unlock for shared memory.
    mut_withoutVBO.lock();
    bool empty1 = m_ChunksWithoutVBO.empty();
    mut_withoutVBO.unlock();

    // create the VBO workder for each chunks in the list
    while(!empty1) {

        mut_withoutVBO.lock();
        auto it = m_ChunksWithoutVBO.begin();
        int64_t tempKey = *it;
        m_ChunksWithoutVBO.erase(it);
        empty1 = m_ChunksWithoutVBO.empty();
        mut_withoutVBO.unlock();
        createVBOWorkers(tempKey);
    }

    // create the VBO workder for each chunks in the list
    mutex_waitList.lock();
    bool empty2 = chunksWaitlist.empty();
    mutex_waitList.unlock();
    while(!empty2) {
        mutex_waitList.lock();

        int64_t tempKey = chunksWaitlist.back();
        chunksWaitlist.pop_back();
        empty2 = m_ChunksWithoutVBO.empty();
        mutex_waitList.unlock();

        mutex_chunks.lock();
        m_chunks[tempKey]->destroy();
        mutex_chunks.unlock();
        createVBOWorkers(tempKey);

    }


    mut_withVBO.lock();
    bool empty3 = m_ChunksWithVBO.empty();
    mut_withVBO.unlock();
    // in each tick, only pass one vbo for a chunk to reduce the latency
    if(!empty3) {
        mut_withVBO.lock();
        int64_t tempKey = m_ChunksWithVBO.back();
        m_ChunksWithVBO.pop_back();
        empty3 = m_ChunksWithVBO.empty();
        mut_withVBO.unlock();
        passVBO(tempKey);

    }

    this->join();

}

// pass the vbo data to gpu
void Terrain::passVBO(int64_t key) {
    mutex_chunks.lock();
    Chunk *c = this->m_chunks[key].get();
    mutex_chunks.unlock();
    c->passVBO();
    mutex_waitList.lock();
    c->readyToDraw(true);
    mutex_waitList.unlock();
}

// create each vbo worker
void Terrain::createVBOWorkers(int64_t key) {
//    std::cout << "in to create VBOWoker"<< std::endl;
    threads.push_back(std::thread(&Terrain::VBOWorker, this, key));
}

// create each blocktypeworker
void Terrain::blockTypeWorker(int x, int z) {
//    std::cout << "in  blockTypeWoker"<< std::endl;
    uPtr<Chunk> temp = mkU<Chunk>(mp_context);
    temp->createLandscape(x, z);
    Chunk *cPtr = temp.get();

    mutex_chunks.lock();
    m_chunks[toKey(x, z)] = move(temp);
    mutex_chunks.unlock();

    // setup neibours.
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = getChunkAt(x, z + 16);
        cPtr->linkNeighbor(chunkNorth, ZPOS);
//        mutex_waitList.lock();
//        if(chunkNorth->nCount == 4 && chunkNorth->readyToDraw()) {
//            chunkNorth->readyToDraw(false);
//            chunksWaitlist.push_back(toKey(x, z + 16));
////            chunkNorth->finished = true;
//        }
//        mutex_waitList.unlock();
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = getChunkAt(x, z - 16);
        cPtr->linkNeighbor(chunkSouth, ZNEG);


    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = getChunkAt(x + 16, z);
        cPtr->linkNeighbor(chunkEast, XPOS);


    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = getChunkAt(x - 16, z);
        cPtr->linkNeighbor(chunkWest, XNEG);


    }

    mut_withoutVBO.lock(); 
    m_ChunksWithoutVBO.insert(toKey(x, z ));
    mut_withoutVBO.unlock();
}

// vboworker
void Terrain::VBOWorker(int64_t key) {
//    std::cout << "in  VBOWoker"<< std::endl;
    mutex_chunks.lock();
    Chunk *chunk = this->m_chunks[key].get();
    mutex_chunks.unlock();
    chunk->setChunkInterleave();
    chunk->setChunkTransInterleave();
    mut_withVBO.lock();
    m_ChunksWithVBO.push_back(key);
    mut_withVBO.unlock();
//    std::cout << "end  VBOworker"<< std::endl;
}
void Terrain::createBlockTypeWorker(int x, int z) {
//    std::cout << "in create blockTypeWoker"<< std::endl;
    int xFloor = 64 * static_cast<int>(glm::floor(x / 64.f));
    int zFloor = 64 * static_cast<int>(glm::floor(z / 64.f));
    for(int i = 0; i < 64; i += 16) {
        for(int j = 0; j < 64; j += 16) {
//            std::cout << "Iteration create blockTypeWoker"<< std::endl;

            threads.push_back(std::thread(&Terrain::blockTypeWorker,this,xFloor + i, zFloor + j));
//            std::cout << "Iteration create blockTypeWoker end"<< std::endl;
        }
    }
    this->m_generatedTerrain.insert(toKey(xFloor, zFloor));
}

// only join one thread each time.
void Terrain::join() {

    if(threads.size() != 0) {
        threads.front().join();
        threads.pop_front();
    }
}
void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
    // m_geomCube.create();

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
//    for(int x = 0; x < 64; x += 16) {
//        for(int z = 0; z < 64; z += 16) {
//            instantiateChunkAt(x, z);
//            // Creat the current chunk
//            m_chunks[toKey(x,z)]->create();
//            UpdateNeighborChunks(x,z);
//        }
//    }
//    // Tell our existing terrain set that
//    // the "generated terrain zone" at (0,0)
//    // now exists.
//    m_generatedTerrain.insert(toKey(0, 0));

//    // Create the basic terrain floor
//    for(int x = 0; x < 64; ++x) {
//        for(int z = 0; z < 64; ++z) {
//            if((x + z) % 2 == 0) {
//                setBlockAt(x, 128, z, STONE);
//            }
//            else {
//                setBlockAt(x, 128, z, DIRT);
//            }
//        }
//    }
//    // Add "walls" for collision testing
//    for(int x = 0; x < 64; ++x) {
//        setBlockAt(x, 129, 0, GRASS);
//        setBlockAt(x, 130, 0, GRASS);
//        setBlockAt(x, 129, 63, GRASS);
//        setBlockAt(0, 130, x, GRASS);
//    }
//    // Add a central column
//    for(int y = 129; y < 140; ++y) {
//        setBlockAt(32, y, 32, GRASS);
//    }
}

void Terrain::addToWaitList(int64_t key) {
    mut_withoutVBO.lock();
    m_ChunksWithoutVBO.insert(key);
    mut_withoutVBO.unlock();
}


void Terrain::blockTypeWorkerIMG(int x, int z, int rx, int rz, QImage* img, bool isColor) {
//    std::cout << "in block img" << std::endl;
    Chunk *cPtr = nullptr;

    if(hasChunkAt(x,z)) {

        mutex_chunks.lock();
        cPtr = m_chunks[toKey(x,z)].get();
        cPtr->destroy();
        cPtr->readyToDraw(false);
        mutex_chunks.unlock();
    } else {
        uPtr<Chunk> temp = mkU<Chunk>(mp_context);
        mutex_chunks.lock();
        m_chunks[toKey(x, z)] = move(temp);
        mutex_chunks.unlock();
        cPtr = m_chunks[toKey(x, z)].get();
    }

//    if(cPtr == nullptr) return;
//    cPtr->readyToDraw(false);
//    std::cout << "in block img unlock" << std::endl;
    cPtr->createLandscape(rx, rz, img, isColor);

    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = getChunkAt(x, z + 16);
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = getChunkAt(x, z - 16);
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = getChunkAt(x + 16, z);
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = getChunkAt(x - 16, z);
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    mut_withoutVBO.lock();
    if(m_ChunksWithoutVBO.find(toKey(x, z)) == m_ChunksWithoutVBO.end()) {
        m_ChunksWithoutVBO.insert(toKey(x, z ));
    }
    mut_withoutVBO.unlock();

}

void Terrain::createBlockTypeWorkerIMG(int x, int z, int rx, int rz, QImage* img, bool isColor) {
    threads.push_back(std::thread(&Terrain::blockTypeWorkerIMG,this, x, z, rx, rz, img, isColor));
    if((x % 64 == 0) && (z % 64 == 0)) {
        this->m_generatedTerrain.insert(toKey(x,z));
    }
}

void Terrain::updateEdgeWithHightMap(int x1, int z1, int x2, int z2) {
    for(int x = x1; x <= x2; x+= 16) {
        if(hasChunkAt(x, z1)) {
            this->mutex_chunks.lock();
            this->m_chunks[toKey(x, z1)]->readyToDraw(false);
            this->mutex_chunks.unlock();
            mut_withoutVBO.lock();
            m_ChunksWithoutVBO.insert(toKey(x, z1 ));
            mut_withoutVBO.unlock();
        }

        if(hasChunkAt(x, z2)) {
            this->mutex_chunks.lock();
            this->m_chunks[toKey(x, z2)]->readyToDraw(false);
            this->mutex_chunks.unlock();
            mut_withoutVBO.lock();
            m_ChunksWithoutVBO.insert(toKey(x, z2));
            mut_withoutVBO.unlock();
        }
    }

    for(int z = z2 + 16; z < z1; z+=16) {
        if(hasChunkAt(x1, z)) {
            this->mutex_chunks.lock();
            this->m_chunks[toKey(x1, z)]->readyToDraw(false);
            this->mutex_chunks.unlock();
            mut_withoutVBO.lock();
            m_ChunksWithoutVBO.insert(toKey(x1, z));
            mut_withoutVBO.unlock();
        }

        if(hasChunkAt(x2, z)) {
            this->mutex_chunks.lock();
            this->m_chunks[toKey(x2, z)]->readyToDraw(false);
            this->mutex_chunks.unlock();
            mut_withoutVBO.lock();
            m_ChunksWithoutVBO.insert(toKey(x2, z));
            mut_withoutVBO.unlock();
        }

    }

}
