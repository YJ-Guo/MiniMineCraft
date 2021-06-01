#include "character.h"

Character::Character(OpenGLContext *context, int slot, char c,int digit)
    : Drawable(context), slot(slot), c(c), digit(digit)
{}

void Character::create() {
    float charsize = 0.05f;
    float heightToWidth = mp_context->width()*1.f/mp_context->height();
    float hoffset = (-4+slot)*0.2f/heightToWidth + digit* charsize/heightToWidth + 0.05f/heightToWidth;
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    float upper = -0.92f;
    float lower =  -0.92f-charsize;
    float left = 0;
    float right = charsize/heightToWidth;
    glm::vec4 vert_pos[4] {glm::vec4(hoffset+left, lower, 0.987f, 1.f),
                           glm::vec4(hoffset+right, lower, 0.987f, 1.f),
                           glm::vec4(hoffset+right, upper, 0.987f, 1.f),
                           glm::vec4(hoffset+left, upper, 0.987f, 1.f)};
    int iconRow = 0;
    int iconCol = 0;
    if(c>='0' && c <= '9') {
        iconRow = 12;
        iconCol = c - '0';
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
