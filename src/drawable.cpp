#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufPosColNor(),
      m_Transcount(-1), m_bufTransIdx(), m_bufTransPosColNor(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_poscolnorGenerated(false),
      m_TransidxGenerated(false), m_TransposcolnorGenerated(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufPosColNor);

    mp_context->glDeleteBuffers(1, &m_bufTransIdx);
    mp_context->glDeleteBuffers(1, &m_bufTransPosColNor);

    m_idxGenerated = m_posGenerated = m_norGenerated = m_colGenerated = m_poscolnorGenerated = false;
    m_count = -1;
    m_Transcount = -1;
    m_TransidxGenerated = m_TransposcolnorGenerated = false;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

int Drawable::elemTransCount()
{
    return m_Transcount;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generatePosColNor() {
    m_poscolnorGenerated = true;
     // Create a VBO on our GPU and store its handle in bufPosColNor
    mp_context->glGenBuffers(1, &m_bufPosColNor);
}

void Drawable::generateTransIdx() {
    m_TransidxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufTransIdx
    mp_context->glGenBuffers(1, &m_bufTransIdx);
}

void Drawable::generateTransPosColNor() {
    m_TransposcolnorGenerated = true;
     // Create a VBO on our GPU and store its handle in bufPosColNor
    mp_context->glGenBuffers(1, &m_bufTransPosColNor);
}

void Drawable::generateUV()
{
    m_uvGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufUV);
}


bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindPosColNor() {
    if (m_poscolnorGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosColNor);
    }
    return m_poscolnorGenerated;
}

bool Drawable::bindTransIdx()
{
    if(m_TransidxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufTransIdx);
    }
    return m_TransidxGenerated;
}

bool Drawable::bindTransPosColNor() {
    if (m_TransposcolnorGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTransPosColNor);
    }
    return m_TransposcolnorGenerated;
}

bool Drawable::bindUV()
{
    if(m_uvGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}
