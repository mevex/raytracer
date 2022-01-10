#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "v3.h"

class Material;

struct HitRecord
{
    p3 p;
    v3 normal;
    f32 t;
    bool frontFace;
    Material *material;
    
    inline void SetFaceNormal(Ray& r, v3& outNormal)
    {
        frontFace = Dot(r.direction, outNormal) < 0;
        normal = frontFace ? outNormal : -outNormal;
    }
};

class Hittable
{
    public:
    virtual bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) = 0;
};

class Sphere : public Hittable
{
    public:
    
    p3 center;
    f32 radius;
    Material *material;
    
    Sphere(p3 cen = {0,0,0}, f32 r = 0, Material *m = 0) : center(cen), radius(r), material(m) {}
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) override
    {
        v3 oc = r.origin - center;
        f32 a = r.direction.LengthSquared();
        f32 halfB = Dot(oc, r.direction);
        f32 c = oc.LengthSquared() - radius*radius;
        
        f32 discriminant = halfB*halfB - a*c;
        if(discriminant < 0)
            return false;
        
        // NOTE(mevex): Find the nearest root that lies in the acceptable range
        f32 sqrtDis = sqrt(discriminant);
        f32 root = (-halfB - sqrtDis) / a;
        if(root < tMin || root > tMax)
        {
            root = (-halfB + sqrtDis) / a;
            if(root < tMin || root > tMax)
                return false;
        }
        
        rec.p = r.At(root);
        rec.t = root;
        v3 outNormal = (rec.p - center) / radius;
        rec.SetFaceNormal(r, outNormal);
        rec.material = material;
        
        return true;
    }
};

class Plane : public Hittable
{
    public:
    
    p3 point;
    v3 normal;
    Material *material;
    
    Plane(p3 p, v3 n, Material *m = 0) : point(p), normal(n), material(m) {}
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) override
    {
        // NOTE(mevex): If the dot product is > 0 then normal is facing away from the ray so we hit the "back face" a.k.a. not worth proceding
        f32 denom = Dot(r.direction, normal);
        // TODO(mevex): Performance test with multiple planes
        if(Abs(denom) < ZERO)// || denom > 0)
            return false;
        
        f32 num = Dot((point - r.origin), normal);
        f32 t = num / denom;
        
        if(t < tMin || t > tMax)
            return false;
        
        rec.p = r.At(t);
        rec.t = t;
        rec.SetFaceNormal(r, normal);
        rec.material = material;
        
        return true;
    }
};

#endif //HITTABLE_H
