#include "inventory.h"
#include <iostream>

Inventory::Inventory(OpenGLContext *context)
    : myInvBar(context), myContext(context), show(true), selectedSlot(-1), blocks(), mySel(context, -1, 0.2f)
{
}

void Inventory::update(ShaderProgram *shader) {
    if (show) {

        auto iter = blocks.begin();
        while (iter < blocks.end()) {
            if ((*iter).myCount < 1) {
                iter = blocks.erase(iter);
            }
            iter++;
        }

        for (int i = 0; i < blocks.size() && i < 8; i++) {
            blocks[i].slot = i;
            blocks[i].create();
            shader->drawPost(blocks[i],0);
            int count = blocks[i].myCount;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (count  > 9) {
                Character tens(myContext, i, '0'+ count/10, 0);
                Character ones(myContext, i, '0'+ count%10, 1);
                tens.create();
                ones.create();
                shader->drawPost(tens, 3);
                shader->drawPost(ones, 3);
            } else {
                Character ones(myContext, i, '0'+ count, 0);
                ones.destroy();
                ones.create();
                shader->drawPost(ones, 3);
            }
            glDisable(GL_BLEND);
        }


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (selectedSlot >= 0 && selectedSlot <=8 && selectedSlot < blocks.size()) {
            mySel.slot = selectedSlot;
            mySel.create();
            shader->drawPost(mySel,0);
        } else {
            mySel.destroy();
        }
        myInvBar.create();
        shader->drawPost(myInvBar,0);
        glDisable(GL_BLEND);

    }
}

