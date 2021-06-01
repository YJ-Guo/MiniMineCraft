#include "turtle.h"

Turtle::Turtle()
    :position(), orientation(), depth(1)
{}


Turtle::Turtle(glm::vec3 pos, glm::vec3 orient, int depth)
    :position(pos), orientation(orient), depth(depth)
{}
