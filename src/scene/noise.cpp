#include "noise.h"
#include <iostream>
Noise::Noise()
{

}
// return a random noise vec2
glm::vec2 Noise::simpleNoise(glm::vec2 v) {
    return glm::vec2(abs((sin(glm::dot(v, glm::vec2(79.19, 213.1)))) ),
                abs((sin(glm::dot(v, glm::vec2(40.01, 119.7)))) ));
}
// gets the hight of grass biome
int Noise::grassHight(int x, int z){
    float res = worley(glm::vec2(x , z ));
//    std::cout << "res: " << res<< std::endl;
    return (int)(res *(150 - 129) + 130);
}

// gets the height of mountain biome
int Noise::mountainHight(int x, int z){
    float res = fbm(glm::vec2(x, z) / 1024.f, 1);
//        float res = glm::sin( fbm(glm::vec2(x, z));

    return (int)(res*res*res *(255 - 120) + 120);
}

int Noise::getTemp(int x, int z) {
    x += 123456;
    z += 654321;
    float res = worley(glm::vec2(x , z ) * 5.f);
    double perlin = perlinNoise(glm::vec2(x,z) / 3.f, 256);
//    std::cout << perlin << std::endl;
    return perlin * 100 + (res - 0.5) * 5;
}

int Noise::getMois(int x, int z) {
    x += 654321;
    z += 400000;

    float res = worley(glm::vec2(x , z ) * 5.f);
    double perlin = perlinNoise(glm::vec2(x,z), 256);
    perlin = (perlin + 1) * 0.5f;

    return perlin * 100 + (res - 0.5) * 5;

}
// 1 : normal montain or plain
// 2 : desert
// 3 : snow trunda
// 4 : beach
int Noise::getBiome(int x, int z) {
    int temp = getTemp(x ,z);
    int mois = getMois(x ,z);
//    std::cout << mois << std::endl;
    if(temp <= -5 ) {
        return 3;
    }

    if(temp >= 5) {
        if(mois < 25) {
            return 2;
        }
        if(mois > 45) {
            return 4;
        }
    }

    return 1;

}
// returns the final height of terrains
glm::ivec2 Noise::getHight(int x, int z) {
//    int temp = getTemp(x ,z);
    int mois = getMois(x ,z);
    int bio = getBiome(x ,z);
     x += 50000;
     z += 50000;
     int grass = grassHight(x, z);
     int mountain = mountainHight(x, z);

     double perlin = perlinNoise(glm::vec2(x,z), 256);
     perlin = (perlin + 1) * 0.5f;

     perlin = glm::smoothstep(0.5, 0.7, perlin);
     double moiss = glm::smoothstep(0.35, 0.65, mois / 100.0);
//     std::cout << moiss << std::endl;
     int res = glm::mix(grass, mountain, perlin);
     int beach = glm::mix(res, res - (int)(worley(glm::vec2(x , z ) / 3.f) * 100), moiss);
//     return glm::ivec2(1, res);
     if(perlin < 0.4) {
         if(bio == 1) {
//             std::cout << bio << std::endl;
             return glm::ivec2(1, res);
         }
         if(bio == 4) {
//            std::cout << bio << std::endl;
            return glm::ivec2(4, beach);
         }
         if(bio == 2) {
//             std::cout << bio << std::endl;
             return glm::ivec2(2, res);
         }
         if(bio == 3) {
//             std::cout << bio << std::endl;
             return glm::ivec2(3, res);
         }

     } else {
         if(bio == 1) {
//             std::cout << bio << std::endl;
             return glm::ivec2(1, res);
         }
         if(bio == 4) {
//             std::cout << 4 << std::endl;
            return glm::ivec2(1, res);
         }
         if(bio == 2) {
//             std::cout << 2 << std::endl;
             return glm::ivec2(1, res);
         }
         if(bio == 3) {
//             std::cout << bio << std::endl;
             return glm::ivec2(3, res);
         }

     }




//    return grass;
}

// returns a worley noise value
float Noise::worley(glm::vec2 xz) {
    xz /= 100;
    xz = xz + fbm(xz / 16.f, 0);
    glm::vec2 integer = glm::floor(xz);
    glm::vec2 fac = glm::fract(xz);
    glm::vec2 closest;
    float min1 = 2.f;
    float min2 = 2.f;
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            glm::vec2 other = glm::vec2(i, j);
            glm::vec2 randomOther = simpleNoise(other + integer);
            float  distance = glm::length(other + randomOther - fac);
            if(distance < min1) {
                min2 = min1;
                min1 = distance;
                closest = randomOther;
            } else if (distance < min2) {
                min2 = distance;
            }
        }
    }
    float h = min2 - min1;
//    h = glm::max(0.f, h - 0.1f) / 0.9f;

    return h * (glm::sin(glm::length(simpleNoise(closest))) * 0.f + 1);

}

// returns a perlinoise used for mountain height
double Noise::perlinNoise(glm::vec2 xz, int size) {
    xz /= (float)size;
    glm::vec2 integer = glm::floor(xz);
//    glm::vec2 fac = glm::fract(xz);
    float res = 0.f;
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 2; j++) {
            res += fall(xz, integer + glm::vec2(i, j));
        }
    }
//    res =  res * res;
//    res = res * 0.75 + 0.25;
    return res;
}
// called by perlinnoise
float Noise::fall(glm::vec2 v1, glm::vec2 v2) {
    glm::vec2 t = glm::abs(v1 - v2);
    glm::vec2 t2 = glm::vec2(1.f) - 6.f * glm::vec2(pow(t.x, 5.f), pow(t.y, 5.f))
            + 15.f * glm::vec2(pow(t.x, 4.f), pow(t.y, 4.f))
            - 10.f * glm::vec2(pow(t.x, 3.f), pow(t.y, 3.f));
    glm::vec2 grad = 2.f * glm::normalize(simpleNoise(v2 + 1.f))  - glm::vec2(1.f);
    glm::vec2 diff = v1 - v2;
    float res = glm::dot(diff, grad);
    return res * t2.x * t2.y;
}

// used to create a rather less noisy function with other noise funtions as base noise funtion
float Noise::fbm(glm::vec2 xz, int j) {
    xz *= 16.f;
    float x =  xz.x;
    float y = xz.y;
    float total = 0;
        float persistence = 0.5f;
        int octaves = 8;

        for(int i = 1; i <= octaves; i++) {
            float freq = pow(2.f, i);
            float amp = pow(persistence, i);
            if(j  == 1) {
                total += (1 - glm::abs(perlinNoise(glm::vec2(x * freq, y * freq), 3) ))* amp;
            } else {
                total += interp(x * freq,
                                       y * freq) * amp;
            }

        }
        return total;
}

// interpolation.
float Noise::interp(float x, float y) {
    int intX = int(floor(x));
    float fractX = glm::fract(x);
    int intY = int(floor(y));
    float fractY = glm::fract(y);

    float v1 = glm::length(simpleNoise(glm::vec2(intX, intY)));
    float v2 = glm::length(simpleNoise(glm::vec2(intX + 1, intY)));
    float v3 = glm::length(simpleNoise(glm::vec2(intX, intY + 1)));
    float v4 = glm::length(simpleNoise(glm::vec2(intX + 1, intY + 1)));

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}
