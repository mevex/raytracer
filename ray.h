#ifndef RAY_H
#define RAY_H

#include "v3.h"

class Ray
{
    public:
    
    p3 origin;
    v3 direction;
    
    Ray(v3 o = {0,0,0}, v3 d = {0,0,0}) : origin(o), direction(d) {}
    
    p3 At(f32 t)
    {
        p3 result = origin + t*direction;
        return result;
    }
};

#endif //RAY_H
