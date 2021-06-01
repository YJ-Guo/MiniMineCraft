#ifndef NOISE_H
#define NOISE_H
#include <glm/glm.hpp>

class Noise
{
public:
    Noise();
    // get grass height
    int grassHight(int x, int z);
    // get mountainheight
    int mountainHight(int x, int z);
    // get final height
    glm::ivec2 getHight(int x, int z);

    int getTemp(int x, int z);
    int getMois(int x, int z);
    int getBiome(int x, int z);
    // worley noise
    float worley(glm::vec2 xz);
    // perlinNoise
    double perlinNoise(glm::vec2 xz, int size);
    float fall(glm::vec2, glm::vec2);
    // fbm noise
    float fbm(glm::vec2 xz, int i);
    // interpolation
    float interp(float x, float y);
    // simple noise funtion
    glm::vec2 simpleNoise(glm::vec2 v);
};

#endif // NOISE_H
