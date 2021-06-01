#ifndef CHARACTER_H
#define CHARACTER_H

#include <drawable.h>

class Character : public Drawable
{
public:
    float xPos;
    float yPos;
    int slot;
    int digit;
    char c;

    Character(OpenGLContext *context,int slot, char c, int digit);
    void create() override;
};

#endif // CHARACTER_H
