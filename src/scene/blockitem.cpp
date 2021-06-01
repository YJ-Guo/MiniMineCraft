#include "blockitem.h"

BlockItem::BlockItem(OpenGLContext *context, BlockType type, int count, int slot)
    : Drawable(context), myType(type), myCount(count), slot(slot), iconsize(0.2f)
{}


void BlockItem::create() {
    if (myCount<1) {
        return;
    }
    float heightToWidth = mp_context->width()*1.f/mp_context->height();
    float hoffset = (-4+slot)*iconsize/heightToWidth;
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    float upper = -1.f+iconsize*0.9f;
    float lower =  -1.f+iconsize*0.1f;
    float left = 0.1f*iconsize/heightToWidth;
    float right = 0.9f*iconsize/heightToWidth;
    glm::vec4 vert_pos[4] {glm::vec4(hoffset+left, lower, 0.988f, 1.f),
                           glm::vec4(hoffset+right, lower, 0.988f, 1.f),
                           glm::vec4(hoffset+right, upper, 0.988f, 1.f),
                           glm::vec4(hoffset+left, upper, 0.988f, 1.f)};
    int iconRow = 0;
    int iconCol = 0;
    switch (myType) {
    case GRASS :
        iconRow = 15;
        iconCol = 3;
        break;
    case DIRT:
        iconRow = 15;
        iconCol = 2;
        break;
    case STONE:
        iconRow = 15;
        iconCol = 1;
        break;
    case SNOW:
        iconRow = 11;
        iconCol = 2;
        break;
    case WATER:
        iconRow = 3;
        iconCol = 15;
        break;
    case LAVA:
        iconRow = 0;
        iconCol = 15;
        break;
    case ICE:
        iconRow = 11;
        iconCol = 3;
        break;
    case SAND:
        iconRow = 14;
        iconCol = 2;
        break;
    case WOOD :
        iconRow = 14;
        iconCol = 4;
        break;
    case LEAVE :
        iconRow = 12;
        iconCol = 5;
        break;
    default :
        iconRow = 7;
        iconCol = 10;
        break;

    }
    float unitSize = 1.f/16;
    glm::vec2 vert_UV[4] {glm::vec2(iconCol*unitSize, iconRow*unitSize),
                          glm::vec2(iconCol*unitSize+1.f/16, iconRow*unitSize),
                          glm::vec2(iconCol*unitSize+1.f/16, iconRow*unitSize+1.f/16),
                          glm::vec2(iconCol*unitSize, iconRow*unitSize+1.f/16)};


    m_count = 6;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);


    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_pos, GL_STATIC_DRAW);
    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), vert_UV, GL_STATIC_DRAW);


}
