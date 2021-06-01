#include "player.h"
#include <QString>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <algorithm>

Player::Player(glm::vec3 pos, Terrain *terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera), flightMode(true), tiltAngle(0.f),inFluid(false), jumpable(true)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    // sensitivity for rotating the camera based on mouse movements
    float xSensitivity = 0.015f;
    float ySensitivity = 0.015f;
    // tilt the camera, limit the final angle to -90~+90 degree
    float tilt = input.mouseY*ySensitivity*-1;
    tilt = std::clamp(tilt, -89.99f-tiltAngle, 89.99f-tiltAngle);
    tiltAngle += tilt;
    // pan and tilt the camera
    rotateOnUpGlobal(input.mouseX*xSensitivity*-1);
    rotateOnRightLocal(tilt);
    // process the key inputs and compute the physics
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs) {
    // acceleration for key press
    float acc = 15.0f;
    // reset acceleration
    m_acceleration[0] = 0.0f;
    m_acceleration[1] = 0.0f;
    m_acceleration[2] = 0.0f;
    if (flightMode) {
        // if f is pressed switch mode
        if (inputs.fPressed) {
            flightMode = !flightMode;
        } else {

            if (inputs.wPressed) {
                m_acceleration += glm::vec3(m_forward * acc);
            }
            if (inputs.sPressed) {
                m_acceleration -= glm::vec3(m_forward * acc);
            }
            if (inputs.dPressed) {
                m_acceleration += glm::vec3(m_right * acc);
            }
            if (inputs.aPressed) {
                m_acceleration -= glm::vec3(m_right * acc);
            }
            if (inputs.ePressed) {
                m_acceleration += glm::vec3(m_up * acc);
            }
            if (inputs.qPressed) {
                m_acceleration -= glm::vec3(m_up * acc);
            }

        }

    } else {        // walk mode

        if (inputs.fPressed) {
            flightMode = !flightMode;
        } else {
            m_acceleration[1] = -9.8f;
            glm::vec3 noYForward = m_forward;
            noYForward[1] = 0;
            noYForward = glm::normalize(noYForward);
            glm::vec3 noYRight = m_right;
            noYRight[1] = 0;
            noYRight = glm::normalize(noYRight);
            if (inputs.wPressed) {
                m_acceleration += glm::vec3(noYForward * acc);
            }
            if (inputs.sPressed) {
                m_acceleration -= glm::vec3(noYForward * acc);
            }
            if (inputs.dPressed) {
                m_acceleration += glm::vec3(noYRight * acc);
            }
            if (inputs.aPressed) {
                m_acceleration -= glm::vec3(noYRight * acc);
            }
            if (inputs.spacePressed) {
                BlockType blk;
                if (mcr_terrain->hasChunkAt(m_position[0], m_position[2])) {
                    blk = mcr_terrain->getBlockAt(m_position[0],m_position[1],m_position[2]);
                } else {
                    blk = EMPTY;
                }
                if(blk == WATER || blk == LAVA) {
                    m_velocity[1] = 4.0f;
                } else if (jumpable) {
                    m_velocity[1] += 8.0f;
                    jumpable = false;

                }

            }

        }


    }


}

void Player::computePhysics(float dT, Terrain *terrain) {
    // reduce the current speed based on mode
    if (flightMode) {
        m_velocity *= std::pow(0.4f, dT);
    } else {
        // reduce on x and z axis a bit more
        m_velocity[0] *= std::pow(0.3f, dT);
        m_velocity[1] *= std::pow(0.6f, dT);
        m_velocity[2] *= std::pow(0.3f, dT);
    }
    // update the velocity based on acceleration
    m_velocity += m_acceleration * dT;
    if (!flightMode) {
        // if in walk mode, check collision and restrict movement if needed
        glm::vec3 movelimit = collisionCheck(dT);
        glm::vec3 moveVec = m_velocity * dT;
        BlockType block = mcr_terrain->getBlockAt(m_position[0],m_position[1],m_position[2]);
        /*
        if(block == WATER || block == LAVA) {
            m_velocity*=std::pow(0.666f, dT);
        }
        */
        if (std::abs(moveVec[1]) > movelimit[1]) {
            moveVec[1] = moveVec[1] > 0 ? movelimit[1] : -movelimit[1];
            m_velocity[1] = 0;
            jumpable = true;
        }

        if (std::abs(moveVec[0]) > movelimit[0]) {
            moveVec[0] = moveVec[0] > 0 ? movelimit[0] : -movelimit[0];
            m_velocity[0] = 0;

        }

        if (std::abs(moveVec[2]) > movelimit[2]) {
            moveVec[2] = moveVec[2] > 0 ? movelimit[2] : -movelimit[2];
            m_velocity[2] = 0;

        }
        if(block == WATER || block == LAVA) {
            moveAlongVector(moveVec*0.666f);
            inFluid = true;
        } else {
            inFluid = false;
            moveAlongVector(moveVec);
        }
    } else {
        // fly mode, just fly
        moveAlongVector(m_velocity * dT);
    }




}
glm::vec3 Player::collisionCheck(float dT) {
    float x, y, z, minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    // nested loop to iterate over all vertices for checking
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            x = m_position[0] -0.5 +(j/2);
            y = m_position[1] + i;
            z = m_position[2] -0.5 +(j%2);
            try {
                // cast rays in 3 axial directions
                float limitX = std::abs(m_velocity[0]);
                for (int dX = 0; dX < std::abs(m_velocity[0]*dT)+1; dX++) {

                    float currX;
                    if ( m_velocity[0] > 0) {
                        currX = x + dX;
                    } else {
                        currX = x - dX;
                    }
                    BlockType block = mcr_terrain->getBlockAt(currX, y,z);
                    if(block != EMPTY && block!= WATER && block!= LAVA) {
                        limitX = m_velocity[0] > 0 ? std::floor(currX)-x-0.0001 : x-std::ceil(currX)-0.0001;
                        break;
                    }

                }
                minX = limitX < minX ? limitX : minX;



                float limitY = std::abs(m_velocity[1]);
                for (int dY = 0; dY < std::abs(m_velocity[1]*dT)+1; dY++) {

                    float currY;
                    if ( m_velocity[1] > 0) {
                        currY = y + dY;
                    } else {
                        currY = y - dY;
                    }
                    BlockType block = mcr_terrain->getBlockAt(x, currY,z);
                    if(block != EMPTY && block!= WATER && block!= LAVA) {
                        limitY = m_velocity[1] > 0 ? std::floor(currY)-y-0.0001 : y-std::ceil(currY)-0.0001;
                        break;
                    }

                }
                minY = limitY < minY ? limitY :minY;

                float limitZ = std::abs(m_velocity[2]);
                for (int dZ = 0; dZ < std::abs(m_velocity[2]*dT)+1; dZ++) {

                    float currZ;
                    if ( m_velocity[2] > 0) {
                        currZ = z + dZ;
                    } else {
                        currZ = z - dZ;
                    }
                    BlockType block = mcr_terrain->getBlockAt(x, y, currZ);
                    if(block != EMPTY && block!= WATER && block!= LAVA) {
                        limitZ = m_velocity[2] > 0 ? std::floor(currZ)-z-0.0001 : z-std::ceil(currZ)-0.0001;
                        break;
                    }

                }
                minZ = limitZ < minZ ? limitZ : minZ;

            } catch (int e) {
                std::cout << " collision check out of range" << std::endl;
            }


        }
    }


    return glm::vec3(minX, minY, minZ);

}


glm::vec3 Player::getForward() {
    return m_forward;
}


void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str;
    if (!inFluid){
        str = std::string("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    } else {
        str = std::string("( " + std::to_string(m_velocity.x*0.66) + ", " + std::to_string(m_velocity.y*0.66) + ", " + std::to_string(m_velocity.z*0.66) + ")");
    }
    //std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
