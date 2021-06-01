#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <scene/entity.h>
#include <lsystem.h>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this),m_postTint(this), m_ui(this), m_geomQuad(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), &m_terrain), m_time(0.f),  m_inputs(),
      prevFrameTime(QDateTime::currentMSecsSinceEpoch()), river(false),
      m_fbuffer(this, this->width(), this->height(), this->devicePixelRatio()), myInv(this)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    m_ui.create(":/glsl/passthrough.vert.glsl", ":/glsl/ui.frag.glsl");
    m_ui.setGeometryColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
    m_ui.CreateTextureFile();
    m_ui.CreateTextureFile(":/minecraft_textures_all/ascii.png", 3);

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));
    // Call to read the texture file
    m_progLambert.CreateTextureFile();

    m_geomQuad.create();

    m_postTint.create(":/glsl/passthrough.vert.glsl", ":/glsl/water.frag.glsl");




    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
    /*
    myInv.blocks.push_back(BlockItem(this, GRASS, 10, 0));
    myInv.blocks.push_back(BlockItem(this, STONE, 10, 0));
    myInv.blocks.push_back(BlockItem(this, DIRT, 10, 0));
    myInv.blocks.push_back(BlockItem(this, WATER, 10, 0));
    */
    startTime = QDateTime::currentMSecsSinceEpoch();
//    m_terrain.CreateTestScene();

}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_fbuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {

    // use the difference between current time and last tick's time as the dT
    m_terrain.UpdateTerrain(m_player.mcr_position);


    if (!river&&QDateTime::currentMSecsSinceEpoch()-startTime>4000) {

        //createRiver();
        river = true;
    }



    qint64 currTime = QDateTime::currentMSecsSinceEpoch();
    m_player.tick((currTime - prevFrameTime)/1000.0f, m_inputs);



    // clear mouse inputs after a tick
    m_inputs.mouseX = 0;
    m_inputs.mouseY = 0;
    prevFrameTime = currTime;

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BlockType cameraBlock;
    glm::vec3 camPos = m_player.mcr_camera.mcr_position;
    m_progLambert.setCameraPos(camPos);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());


    if (!m_fbuffer.isCreated()) {
        m_fbuffer.create();
    }


    m_fbuffer.bindFrameBuffer();

    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderTerrain();
    m_progLambert.setTime(m_time++);


    glDisable(GL_DEPTH_TEST);

    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);

    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());

    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_terrain.hasChunkAt(camPos[0], camPos[2])) {
        cameraBlock = m_terrain.getBlockAt(camPos[0], camPos[1], camPos[2]);
    } else {
        cameraBlock = EMPTY;
    }
    if (cameraBlock == WATER) {
        m_postTint.setGeometryColor(glm::vec4(0.65f, 0.75f, 1.f, 1.f));
    } else if (cameraBlock == LAVA) {
        m_postTint.setGeometryColor(glm::vec4(1.f, 0.45f, 0.45f, 1.f));
    } else {
        m_postTint.setGeometryColor(glm::vec4(1.f, 1.f, 1.f, 0.f));
    }
    m_fbuffer.bindToTextureSlot(2);

    m_postTint.setTime(m_time++);
    m_postTint.drawPost(m_geomQuad,2);
    myInv.update(&m_ui);


}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {

    m_terrain.draw(&m_progLambert);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_terrain.drawTrans(&m_progLambert);
    glDisable(GL_BLEND);
}


// update the input bundle based on key presses
void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead

    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = true;
    }

    if(e->key() == Qt::Key_I) {
        myInv.show =!myInv.show;
    }

    if (e->key()-Qt::Key_1 >= 0 && e->key()-Qt::Key_1 <= 8) {
        myInv.selectedSlot = e->key()-Qt::Key_1;
    }

}


void MyGL::createRiver() {

    LSystem mySys(&m_terrain);
    mySys.axiom = "FX";
    mySys.expansionRules['X'] = "[+FX][-FX]";
    mySys.iterateAxiom(6);
    mySys.radius = 7.5f;
    mySys.forwardDist = 37.f;
    mySys.turnAngle = 32.f;
    mySys.projectOnTerrain(glm::vec3(20, 131.5, 24), glm::vec3(0,0,1));

}

// reset the key press flags  upon release
void MyGL::keyReleaseEvent(QKeyEvent *e) {

    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = false;
    }


}

// update input's mouse movement fields
void MyGL::mouseMoveEvent(QMouseEvent *e) {

    QPoint mid = this->mapToGlobal(QPoint(width() / 2, height() / 2));
    m_inputs.mouseX += e->globalX()-mid.x();
    m_inputs.mouseY += e->globalY()-mid.y();
    moveMouseToCenter();
}


// place or destroy blocks when LMB or RMB is pressed
void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        glm::ivec3 hitBlock(0,0,0);
        glm::ivec3 incidentBlock(0,0,0);
        float dist = 0;
        try {
            // grid march to check if a block is within 3 units
            if (gridMarch(m_player.mcr_camera.mcr_position, m_player.getForward()*3.f, &m_terrain,
                          &dist, &hitBlock, &incidentBlock)) {
                BlockType destroyedBlock = m_terrain.getBlockAt(hitBlock.x, hitBlock.y, hitBlock.z);
                bool inInventory = false;
                for (auto &item: myInv.blocks) {
                    if (item.myType == destroyedBlock){
                        item.myCount++;
                        inInventory = true;
                        break;
                    }
                }
                if (!inInventory) {
                    myInv.blocks.push_back(BlockItem(this, destroyedBlock, 1, myInv.blocks.size()));
                }
                m_terrain.setBlockAt(hitBlock.x, hitBlock.y, hitBlock.z, EMPTY);
                // need update!!
                m_terrain.getChunkAt(hitBlock.x, hitBlock.z)->create();
                for(auto &c : m_terrain.getChunkAt(hitBlock.x, hitBlock.z)->m_neighbors) {
                    c.second->create();
                }
            }
        } catch (int e) {
            std::cout << "destroy blcok error" << std::endl;
        }

    } else if (e->button() == Qt::RightButton) {
        glm::ivec3 hitBlock(0,0,0);
        glm::ivec3 incidentBlock(0,0,0);
        float dist = 0;
        try {
            // grid march to check if a block is within 3 units, place a block in the incident block location if so
            if (myInv.selectedSlot>=0 && myInv.selectedSlot<=8 && myInv.selectedSlot<myInv.blocks.size()
                    && gridMarch(m_player.mcr_camera.mcr_position, m_player.getForward()*3.f, &m_terrain,
                          &dist, &hitBlock, &incidentBlock)) {
                BlockType selectedType = myInv.blocks[myInv.selectedSlot].myType;
                myInv.blocks.at(myInv.selectedSlot).myCount--;
                m_terrain.setBlockAt(incidentBlock.x, incidentBlock.y, incidentBlock.z, selectedType);
                m_terrain.getChunkAt(incidentBlock.x, incidentBlock.z)->create();

                for(auto &c : m_terrain.getChunkAt(hitBlock.x, hitBlock.z)->m_neighbors) {
                    c.second->create();
                }
            }
        } catch (int e) {
            std::cout << "place blcok error" << std::endl;
        }
    }
}


// taken from class slide, modified to return the block the ray travelled through before hitting the hit block as well
bool MyGL::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, Terrain *terrain, float *out_dist,
                     glm::ivec3 *out_blockHit, glm::ivec3 *out_incidentBlock) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;

        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]))*(-1)-1;
        *out_incidentBlock = glm::ivec3(glm::floor(rayOrigin)) + offset;

        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain->getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}
void MyGL::UpdateTerrainWithHightMap(QImage *HightMap, bool isColor) {
    int XBlockNum  = glm::floor(HightMap->width() / 64.f) * 64;
    int ZBlockNum  = glm::floor(HightMap->height() / 64.f) * 64;
    glm::vec2 playerPos(m_player.mcr_position.x, m_player.mcr_position.z);
    // Find the Left top conner of the chunk and map image to that
    glm::ivec2 mapBeginChunk = 64 * glm::ivec2(glm::floor((playerPos.x - XBlockNum/2) / 64.f),
                                               glm::floor((playerPos.y + ZBlockNum/2) / 64.f));

    // For each Chunk
    for (int i = 0; i < XBlockNum; i += 16) {
        for (int j = 0; j < ZBlockNum; j += 16) {
//            if(m_terrain.hasChunkAt(mapBeginChunk.x + 16 * glm::floor(i / 16.f),
//                                    mapBeginChunk.y - 16 * glm::floor(j / 16.f))) {
//                Chunk* curChunk = m_terrain.getChunkAt(mapBeginChunk.x + 16 * glm::floor(i / 16.f),
//                                                           mapBeginChunk.y - 16 * glm::floor(j / 16.f)).get();
                this->m_terrain.createBlockTypeWorkerIMG(mapBeginChunk.x + i,
                                                mapBeginChunk.y - j,
                                               i, j, HightMap, isColor);

//            }

        }

    }
    this->m_terrain.updateEdgeWithHightMap(mapBeginChunk.x - 16, mapBeginChunk.y + 16, mapBeginChunk.x + XBlockNum, mapBeginChunk.y - ZBlockNum);

    // Update all the Chunks
}
