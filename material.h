#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"
#include "v3.h"
#include "hittable.h"

class Material
{
    public:
    virtual bool scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) = 0;
};

class Lambertian : public Material
{
    public:
    
    Color albedo;
    
    Lambertian(Color a) : albedo(a) {}
    
    virtual bool scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) override
    {
        p3 scatterDirection = rec.normal + v3::RandomUnitVector();
        // NOTE(mevex): If RandomUnitVector() returns a vector that is the opposite of the normal
        if(scatterDirection.NearZero())
            scatterDirection = rec.normal;
        
        scattered = {rec.p, scatterDirection};
        attenuation = albedo;
        return true;
    }
};

class Metal : public Material
{
    public:
    
    Color albedo;
    f32 fuzz;
    
    Metal(Color a, f32 f) : albedo(a), fuzz(Min(f, 1.0f)) {}
    
    virtual bool scatter(Ray& rIn, HitRecord& rec, Color& attenuation, Ray& scattered) override
    {
        if(fuzz == 1.0f)
            attenuation = albedo;
        
        v3 reflected = Reflect(rIn.direction, rec.normal) + fuzz*v3::RandomInUnitSphere();
        scattered = {rec.p, reflected};
        attenuation = albedo;
        return (Dot(scattered.direction, rec.normal) > 0);
    }
};

#endif //MATERIAL_H