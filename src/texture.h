#pragma once

#include <openglcontext.h>
#include <memory>
#include "glm_includes.h"

class Texture
{
public:
    Texture(OpenGLContext* context);
    ~Texture();

    void create(const char *texturePath);
    void load(int texSlot);
    void bind(int texSlot);

private:
    OpenGLContext* context;
    GLuint m_textureHandle;
    std::shared_ptr<QImage> m_textureImage;
};
