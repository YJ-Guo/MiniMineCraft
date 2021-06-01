#ifndef SELINDICATOR_H
#define SELINDICATOR_H

#include "glm_includes.h"
#include <drawable.h>

class SelIndicator : public Drawable
{
public:
    int slot;
    float height;
    SelIndicator(OpenGLContext *context, int slot, float height);
    void create() override;
};

#endif // SELINDICATOR_H
