#include "lsystem.h"




LSystem::LSystem(Terrain *terrain)
    :turtleStack(), expansionRules(), charToAction(), axiom(), m_terrain(terrain),
      radius(5), forwardDist(10), turnAngle(30)
{
    srand(7456321);
}

void LSystem::iterateAxiom(int iterations)
{

    std::cout << "Axiom Before Iterations: " << axiom << std::endl;
    for (int i = 0; i < iterations; i++) {
        int curr = 0;
        while (curr < axiom.length()) {
            char c = axiom[curr];
            // if there is an expansion rule for this char, expand if rand result is over a threshold
            if (expansionRules.count(c)) {
                if (curr <= 1 || rand() < (RAND_MAX*0.85) ) {
                    std::string expanded = expansionRules[c];
                    axiom.replace(curr, 1, expanded);
                    curr += expanded.length();
                } else if (rand() < (RAND_MAX*0.5)){
                    std::string expanded = "+FX";
                    axiom.replace(curr, 1, expanded);
                    curr += expanded.length();
                } else {
                    std::string expanded = "-FX";
                    axiom.replace(curr, 1, expanded);
                    curr += expanded.length();
                }

            } else {
                curr++;
            }

        }

        std::cout << "Axiom After Iteration " << i+1 << " :" << axiom << std::endl;

    }
}

void LSystem::projectOnTerrain(glm::vec3 origin, glm::vec3 direction) {
    Turtle newTurtle(origin, direction, 0);
    turtleStack.push(newTurtle);
    for (int i = 0; i < axiom.length(); i++) {
        switch (axiom[i]) {

        case 'F':
            moveAndDraw();
            moveAndDraw();
            break;
        case '[':
            makeTurtle();
            break;
        case ']':
            closeTurtle();
            break;
        case '+':
            turnLeft();
            break;
        case '-':
            turnRight();
            break;
        }
    }

}



float LSystem::sdCapsule( glm::vec3 p, glm::vec3 a, glm::vec3 b, float r )
{
  glm::vec3 pa = p - a, ba = b - a;
  float h = glm::clamp( glm::dot(pa,ba)/glm::dot(ba,ba), 0.0f, 1.0f);
  return glm::length( pa - ba*h ) - r;
}




void LSystem::moveAndDraw() {
    glm::vec3 start = turtleStack.top().position;
    float distance = forwardDist * std::pow(0.9, turtleStack.top().depth);
    glm::vec3 end = start+ distance*0.5f*turtleStack.top().orientation;

    m_terrain->UpdateTerrain(start);
    m_terrain->UpdateTerrain(start*0.5f+end*0.5f);
    m_terrain->UpdateTerrain(end);

    turtleStack.top().position = end;
    if(rand() < (RAND_MAX*0.5)) {
        glm::vec4 newOrient(turtleStack.top().orientation, 0.f);
        glm::mat4 rot = glm::rotate(glm::mat4(), glm::radians(12.f), glm::vec3(0,1,0));
        newOrient = newOrient * rot;
        turtleStack.top().orientation = glm::vec3(newOrient);
    } else {
        glm::vec4 newOrient(turtleStack.top().orientation, 0.f);
        glm::mat4 rot = glm::rotate(glm::mat4(), glm::radians(-12.f), glm::vec3(0,1,0));
        newOrient = newOrient * rot;
        turtleStack.top().orientation = glm::vec3(newOrient);
    }
    float r = radius*std::pow(0.85,turtleStack.top().depth);
    for (int x = floor(glm::min(start[0], end[0])-r); x <= floor(glm::max(start[0], end[0])+r); x++) {
        for (int z = floor(glm::min(start[2], end[2])-r); z <= floor(glm::max(start[2], end[2])+r); z++) {
            bool flag = false;
            for(int y = floor(start[1]-r); y<256; y++) {
                if(m_terrain->getBlockAt(x, y, z) == WOOD) {
                    for(int y1 = 100; y1 < y; y1++) {
                        m_terrain->setBlockAt(x, y, z, WOOD);
                    }
                    flag  = true;
                    break;
                }
            }
            if(flag) continue;
            for (int y = floor(start[1]-r); y<256; y++) {

                if (sdCapsule(glm::vec3(x+0.5,y+0.5,z+0.5), start, end, r) < 0) {
                    if (y<=128) {
                        m_terrain->setBlockAt(x,y,z, WATER);
                    } else {
                        m_terrain->setBlockAt(x,y,z, EMPTY);
                    }
                } else if (y>=start[1] && (sdCapsule(glm::vec3(x+0.5,start[1],z+0.5),start,end, r) < 0)) {
                    while (y <256) {
                        if(m_terrain->getBlockAt(x, y, z) == WOOD) {
                            int y1 = y-1;
                            while(m_terrain->getBlockAt(x, y1, z) == WATER) {
                                m_terrain->setBlockAt(x, y1, z, WOOD);
                                y1--;
                            }

                            break;
                        }
                        if(m_terrain->getBlockAt(x, y, z) == LEAVE) {
                            break;
                        }

                        m_terrain->setBlockAt(x,y,z, EMPTY);
                        y++;
                    }
                    break;
                }

            }
        }
    }

    for (float x = glm::min(start[0], end[0])-r; x <= glm::max(start[0], end[0])+r+16; x+=16) {
        for (float z = glm::min(start[2], end[2])-r; z <= glm::max(start[2], end[2])+r+16; z+=16) {
            m_terrain->getChunkAt(x,z)->create();
        }

    }



}

void LSystem::turnLeft() {

    // +-30% random variation in angle

    float randFactor = ((float)rand()/RAND_MAX) - 0.5;
    float turnWithRand = turnAngle * (1+ randFactor*0.6);
    glm::mat4 rotateMat =  glm::rotate(glm::mat4(), glm::radians(turnWithRand), glm::vec3(0,1,0));
    glm::vec4 newOri = rotateMat * glm::vec4(turtleStack.top().orientation, 0.f);
    turtleStack.top().orientation = glm::vec3(newOri.x,newOri.y,newOri.z);

}



void LSystem::turnRight() {
    // +-30% random variation in angle
    float randFactor = ((float)rand()/RAND_MAX) - 0.5;
    float turnWithRand = turnAngle * (1+ randFactor*0.6);
    glm::mat4 rotateMat =  glm::rotate(glm::mat4(), glm::radians(-turnWithRand), glm::vec3(0,1,0));
    glm::vec4 newOri = rotateMat * glm::vec4(turtleStack.top().orientation, 0.f);
    turtleStack.top().orientation = glm::vec3(newOri.x,newOri.y,newOri.z);

}

void LSystem::makeTurtle() {
    Turtle newTurtle;
    newTurtle.position = turtleStack.top().position;
    newTurtle.orientation = turtleStack.top().orientation;
    newTurtle.depth = turtleStack.top().depth+1;
    turtleStack.push(newTurtle);
}

void LSystem::closeTurtle() {
    turtleStack.pop();
}
