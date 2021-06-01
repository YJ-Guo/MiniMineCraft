#ifndef BLOCKITEM_H
#define BLOCKITEM_H

#include "drawable.h"
#include <scene/chunk.h>

class BlockItem : public Drawable
{
public:
    BlockItem(OpenGLContext *context, BlockType type, int count, int slot);

    BlockType myType;
    int myCount;
    int slot;
    float iconsize;
    virtual void create();
};

#endif // BLOCKITEM_H
