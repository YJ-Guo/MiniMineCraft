#ifndef LSYSTEM_H
#define LSYSTEM_H

#include "glm_includes.h"
#include "turtle.h"
#include <stack>
#include <unordered_map>
#include <iostream>
#include <scene/terrain.h>


class LSystem
{

public:
    typedef void (LSystem::*Rule)(void);
    std::stack<Turtle> turtleStack;
    std::unordered_map<char, std::string> expansionRules;
    std::unordered_map<char, Rule> charToAction;
    std::string axiom;
    Terrain *m_terrain;

    float radius;
    float forwardDist;
    float turnAngle;



public:
    LSystem(Terrain *mcr_terrain);
    void iterateAxiom(int iterations);

    float sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r );

    void projectOnTerrain(glm::vec3 origin, glm::vec3 direction);

    void moveAndDraw();
    void makeTurtle();
    void closeTurtle();
    void turnLeft();
    void turnRight();
};

#endif // LSYSTEM_H
