#include "selindicator.h"


SelIndicator::SelIndicator(OpenGLContext *context, int slot, float height)
    :Drawable(context), slot(slot), height(height)
{
}

void SelIndicator::create() {
    float heightToWidth = mp_context->width()*1.f/mp_context->height();

    float unitWidth = height / heightToWidth;
    float hoffset = (-4+slot)*unitWidth;
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    float upper = -1.f+height;
    float lower =  -1.f;
    float left = 0;
    float right = unitWidth;
    glm::vec4 vert_pos[4] {glm::vec4(hoffset+left, lower, 0.98f, 1.f),
                           glm::vec4(hoffset+right, lower, 0.98f, 1.f),
                           glm::vec4(hoffset+right, upper, 0.98f, 1.f),
                           glm::vec4(hoffset+left, upper, 0.98f, 1.f)};
    int iconRow = 7;
    int iconCol = 10;
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
