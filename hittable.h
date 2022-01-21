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
    
    f32 u,v,w;
    
    inline void SetFaceNormal(Ray& r, v3& outNormal)
    {
        frontFace = Dot(r.direction, outNormal) < 0;
        normal = frontFace ? outNormal : -outNormal;
    }
    
    inline void SetBarycentrics(f32 U, f32 V)
    {
        w = 1.0f - U - V;
        this->u = U;
        this->v = V;
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
        if(Abs(denom) < ZERO || denom > 0)
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

class Triangle : public Hittable
{
    public:
    
    p3 a,b,c;
    v3 normal;
    f32 triangleAreaDoubled;
    v3 edge1, edge2;
    v3 ab, bc, ca;
    
    Material *material;
    
    Triangle(p3 v0, p3 v1, p3 v2, Material *m) : a(v0), b(v1), c(v2), material(m)
    {
        edge1 = b - a;
        edge2 = c - a;
        normal = Cross(edge1, edge2);
        triangleAreaDoubled = normal.Length();
        
        // NOTE(mevex): This are used for the non MOLLER_TRUMBORE implementation
        ab = b-a;
        bc = c-b;
        ca = a-c;
    }
    
#define MOLLER_TRUMBORE 1
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) override
    {
        // NOTE(mevex): The function culls backfaces
#if MOLLER_TRUMBORE
        // NOTE(mevex): See scratchpixel's explanation of the algorithm for details on how it works and the variable names
        
        v3 T = r.origin - a;
        
        v3 P = Cross(r.direction, edge2);
        v3 Q = Cross(T, edge1);
        
        f32 determinant = Dot(P, edge1);
        // NOTE(mevex): if the triangle and the ray are parallel (therefore there is no intersection) or if the triangle is backfacing the ray
        // NOTE(mevex): The determinand is equal to -Dot(r.direction, normal), so the condition can be simplified like this
        if(determinant < ZERO)
            return false;
        
        f32 inverseDet = 1.0f / determinant;
        
        f32 u = Dot(P, T) * inverseDet;
        f32 v = Dot(Q, r.direction) * inverseDet;
        if(u < 0 || v < 0 || u+v > 1)
            return false;
        
        f32 t = Dot(Q, edge2) * inverseDet;
        
        rec.p = r.At(t);
        rec.t = t;
        rec.SetFaceNormal(r, normal);
        rec.material = material;
        rec.SetBarycentrics(u, v);
        
        return true;
#else
        // NOTE(mevex): Triangle-plane intersection
        f32 denom = Dot(r.direction, normal);
        if(Abs(denom) < ZERO || denom > 0)
            return false;
        
        f32 num = Dot((a - r.origin), normal);
        f32 t = num / denom;
        
        if(t < tMin || t > tMax)
            return false;
        // NOTE(mevex): Inside-outside test
        p3 p = r.At(t);
        v3 N;
        
        v3 ap = p-a;
        N = Cross(ab, ap);
        if(Dot(normal, N) < 0)
            return false;
        
        v3 bp = p-b;
        N = Cross(bc, bp);
        f32 uAreaDoubled = N.Length();
        if(Dot(normal, N) < 0)
            return false;
        
        v3 cp = p-c;
        N = Cross(ca, cp);
        f32 vAreaDoubled = N.Length();
        if(Dot(normal, N) < 0)
            return false;
        
        // NOTE(mevex): Barycentric coordinates
        f32 u = uAreaDoubled / triangleAreaDoubled;
        f32 v = vAreaDoubled / triangleAreaDoubled;
        
        rec.p = p;
        rec.t = t;
        rec.SetFaceNormal(r, normal);
        rec.material = material;
        rec.SetBarycentrics(u, v);
        
        return true;
#endif
    }
};

#endif //HITTABLE_H