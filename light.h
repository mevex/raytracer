#ifndef LIGHT_H
#define LIGHT_H

#include "v3.h"
#include "main.h"

enum LightType
{
    AMBIENT,
    POINT
};

class Light
{
    public:
    
    enum LightType type;
    virtual f32 ComputeLightning(v3 normal, p3 hitPoint) = 0;
};

class PointLight : public Light
{
    public:
    
    p3 position;
    f32 intensity;
    
    PointLight(p3 p, f32 i) : position(p), intensity(i) 
    {
        type = POINT;
    }
    
    f32 ComputeLightning(v3 normal, p3 hitPoint)
    {
        f32 finalIntensity = 0;
        
        v3 light = position - hitPoint;
        f32 nDotL = Dot(normal, light);
        if(nDotL > 0)
            finalIntensity = intensity * nDotL / (normal.Length() * light.Length());
        
        return finalIntensity;
    }
};

class AmbientLight : public Light
{
    public:
    
    f32 intensity;
    Color color;
    
    AmbientLight(f32 i) : intensity(i)
    {
        type = AMBIENT;
        color = {i,i,i};
    }
    
    AmbientLight(f32 i, Color c) : intensity(i), color(c) {}
    
    f32 ComputeLightning(v3 normal, p3 hitPoint)
    {
        return intensity;
    }
};

#endif //LIGHT_H
