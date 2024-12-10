#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"
#include "v3.h"
#include "hittable.h"

class Material
{
    public:
    virtual bool Scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) = 0;
};

class Lambertian : public Material
{
    public:
    
    Color albedo;
    
    Lambertian(Color a) : albedo(a) {}
    
    bool Scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) override
    {
        ++ScatterCounter;
        u64 cycleBegin = __rdtsc();

        p3 scatterDirection = rec.normal + v3::RandomUnitVector();
        // NOTE(mevex): If RandomUnitVector() returns a vector that is the opposite of the normal
        if(scatterDirection.NearZero())
            scatterDirection = rec.normal;
        
        scattered = {rec.p, Unit(scatterDirection)};
        attenuation = albedo;

        u64 cycleEnd = __rdtsc();
        ScatterCycles += (u64)(cycleEnd - cycleBegin);
        return true;
    }
};

class Metal : public Material
{
    public:
    
    Color albedo;
    f32 fuzz;
    
    Metal(Color a, f32 f) : albedo(a), fuzz(Min(f, 1.0f)) {}
    
    bool Scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) override
    {
        ++ScatterCounter;
        u64 cycleBegin = __rdtsc();

        if(fuzz == 1.0f)
            attenuation = albedo;
        
        v3 reflected = Reflect(rIn.direction, rec.normal) + fuzz*v3::RandomUnitVector();
        scattered = {rec.p, reflected};
        attenuation = albedo;

        u64 cycleEnd = __rdtsc();
        ScatterCycles += cycleEnd - cycleBegin;
        return (Dot(scattered.direction, rec.normal) > 0);
    }
};

// NOTE(mevex): Debugging material for triangles
class VertexColor : public Material
{
    public:
    
    Color a,b,c;
    
    VertexColor(Color c1, Color c2, Color c3) : a(c1), b(c2), c(c3) {}
    
    bool Scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) override
    {
        ++ScatterCounter;
        u64 cycleBegin = __rdtsc();

        // NOTE(mevex): This material scatters like a Lambertian but returns the color based on the barycentric coordinates of the triangle
        p3 scatterDirection = rec.normal + v3::RandomUnitVector();
        // NOTE(mevex): If RandomUnitVector() returns a vector that is the opposite of the normal
        if(scatterDirection.NearZero())
            scatterDirection = rec.normal;
        
        scattered = {rec.p, scatterDirection};
        attenuation = rec.u*a + rec.v*b + rec.w*c;

        u64 cycleEnd = __rdtsc();
        ScatterCycles += cycleEnd - cycleBegin;
        return true;
    }
};

#endif //MATERIAL_H
