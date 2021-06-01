#ifndef INVENTORY_H
#define INVENTORY_H

#include "glm_includes.h"
#include <unordered_map>
#include <iostream>
#include <scene/chunk.h>
#include <scene/invbar.h>
#include <scene/blockitem.h>
#include <shaderprogram.h>
#include <selindicator.h>
#include <character.h>

class Inventory
{
private:
    InvBar myInvBar;
    OpenGLContext *myContext;
    SelIndicator mySel;

public:
    bool show;
    int selectedSlot;
    std::vector<BlockItem> blocks;

    Inventory(OpenGLContext *context);
    void update(ShaderProgram *shader);
};



#endif // INVENTORY_H
