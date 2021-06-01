#ifndef TURTLE_H
#define TURTLE_H
#include "glm_includes.h"


class Turtle
{
public:
    glm::vec3 position;
    glm::vec3 orientation;
    int depth;


public:
    Turtle();
    Turtle(Turtle* old);
    Turtle(glm::vec3 pos, glm::vec3 orient, int depth);

};

#endif // TURTLE_H
