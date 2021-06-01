#ifndef INVBAR_H
#define INVBAR_H
#include "drawable.h"

class InvBar : public Drawable
{
public:
    InvBar(OpenGLContext* context);
    virtual void create();
    float height;
};

#endif // INVBAR_H
