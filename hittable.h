#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "v3.h"

struct HitRecord
{
    p3 p;
    v3 normal;
    f32 t;
    bool frontFace;
    
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

class Sphere : public Hittable {
    public:
    
    p3 center;
    f32 radius;
    
    Sphere(p3 cen = {0,0,0}, f32 r = 0) : center(cen), radius(r) {}
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec)
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
        
        return true;
    }
};

#endif //HITTABLE_H
