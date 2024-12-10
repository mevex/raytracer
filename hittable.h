#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "simd.h"
#include "v3.h"

class Material;
class Lambertian;

struct HitRecord
{
    p3 p;
    v3 normal;
    f32 t = INFINITY;
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
    virtual void Hit(Ray r[4], f32 tMin[4], f32 tMax[4], HitRecord rec[4]) = 0;
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
        v3 co = r.origin - center;
        f32 a = r.direction.LengthSquared();
        f32 halfB = Dot(co, r.direction);
        f32 c = co.LengthSquared() - radius*radius;
        
        f32 halfBSquared = halfB*halfB;
        f32 aTimesC = a*c;
        f32 discriminant = halfBSquared - aTimesC;
        if(discriminant < 0)
            return false;
        
        // NOTE(mevex): Find the nearest root that lies in the acceptable range
        f32 sqrtDis = sqrt(discriminant);
        f32 root = (-halfB - sqrtDis) / a;
        if(root < tMin || root > tMax)
        {
            return false;
        }
        
        rec.p = r.At(root);
        rec.t = root;
        v3 outNormal = (rec.p - center) / radius;
        rec.SetFaceNormal(r, outNormal);
        rec.material = material;
        
        return true;
    }

    void Hit(Ray r[4], f32 tMin[4], f32 tMax[4], HitRecord rec[4]) override
    {
        ++HitCounter;
        u64 cycleBegin = __rdtsc();

        // v3 co = r.origin - center;
        wide_f32 rayOriginX = WideFloatSetIndividual(r[3].origin.x, r[2].origin.x, r[1].origin.x, r[0].origin.x);
        wide_f32 rayOriginY = WideFloatSetIndividual(r[3].origin.y, r[2].origin.y, r[1].origin.y, r[0].origin.y);
        wide_f32 rayOriginZ = WideFloatSetIndividual(r[3].origin.z, r[2].origin.z, r[1].origin.z, r[0].origin.z);
        wide_f32 centerX = WideFloatSetAll(center.x);
        wide_f32 centerY = WideFloatSetAll(center.y);
        wide_f32 centerZ = WideFloatSetAll(center.z);

        wide_f32 coX = WideFloatSubtract(rayOriginX, centerX);
        wide_f32 coY = WideFloatSubtract(rayOriginY, centerY);
        wide_f32 coZ = WideFloatSubtract(rayOriginZ, centerZ);

        // f32 a = r.direction.LengthSquared();
        wide_f32 rayDirectionX = WideFloatSetIndividual(r[3].direction.x, r[2].direction.x, r[1].direction.x, r[0].direction.x);
        wide_f32 rayDirectionY = WideFloatSetIndividual(r[3].direction.y, r[2].direction.y, r[1].direction.y, r[0].direction.y);
        wide_f32 rayDirectionZ = WideFloatSetIndividual(r[3].direction.z, r[2].direction.z, r[1].direction.z, r[0].direction.z);

        wide_f32 a = WideFloatAdd(WideFloatSquare(rayDirectionX), WideFloatAdd(WideFloatSquare(rayDirectionY), WideFloatSquare(rayDirectionZ)));
        a = WideFloatAdd(WideFloatMultiply(rayDirectionX, rayDirectionX), WideFloatAdd(WideFloatMultiply(rayDirectionY, rayDirectionY), WideFloatMultiply(rayDirectionZ, rayDirectionZ)));

        // f32 halfB = Dot(co, r.direction);
        wide_f32 halfB = WideFloatAdd(WideFloatMultiply(coX, rayDirectionX), WideFloatAdd(WideFloatMultiply(coY, rayDirectionY), WideFloatMultiply(coZ, rayDirectionZ)));

        // f32 c = co.LengthSquared() - radius*radius;
        wide_f32 radiusSquared = WideFloatSetAll(radius*radius);
        wide_f32 coLengthSquared = WideFloatAdd(WideFloatSquare(coX), WideFloatAdd(WideFloatSquare(coY), WideFloatSquare(coZ)));

        wide_f32 c = WideFloatSubtract(coLengthSquared, radiusSquared);

        //f32 discriminant = halfBSquared - aTimesC;
        wide_f32 discriminant = WideFloatSubtract(WideFloatSquare(halfB), WideFloatMultiply(a, c));

        // if discriminant is not greater than 0 return false
        // if all the results are false, stop the execution here
        wide_f32 zero = WideFloatSetAll(0.0f);
        wide_i32 wideResults = WideCastFloatToInt(WideFloatGreater(discriminant, zero));
        wide_i32 onesMask = WideIntSetAll(0xFFFFFFFF);
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }

        // NOTE(mevex): Find the nearest root that lies in the acceptable range
        // f32 sqrtDis = sqrt(discriminant);
        wide_f32 sqrtDis = WideFloatSqrt(discriminant);

        // f32 root = (-halfB - sqrtDis) / a;
        wide_f32 halfBNegated = WideFloatSubtract(zero, halfB);
        wide_f32 root = WideFloatDivide(WideFloatSubtract(halfBNegated, sqrtDis), a);

        // NOTE(mevex): check that the root is between the min-max range
        wide_f32 wideTMin = WideFloatSetIndividual(tMin[3], tMin[2], tMin[1], tMin[0]); 
        wide_f32 wideTMax = WideFloatSetIndividual(tMax[3], tMax[2], tMax[1], tMax[0]); 
        wide_i32 rootLessThanTMax = WideCastFloatToInt(WideFloatLess(root, wideTMax));
        wide_i32 rootGreaterThanTMin = WideCastFloatToInt(WideFloatGreater(root, wideTMin));
        wideResults = WideIntAnd(wideResults, WideIntAnd(rootLessThanTMax, rootGreaterThanTMin));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }

        // TODO(mevex): simd this once the function takes simd arguments
        // rec.p = r.At(root) = origin + t*direction;
        wide_f32 recPX = WideFloatAdd(rayOriginX , WideFloatMultiply(root, rayDirectionX));
        wide_f32 recPY = WideFloatAdd(rayOriginY , WideFloatMultiply(root, rayDirectionY));
        wide_f32 recPZ = WideFloatAdd(rayOriginZ , WideFloatMultiply(root, rayDirectionZ));

        for(int i = 0; i < 4; ++i)
        {
            if (ExtractInt(wideResults, i))
            {
                rec[i].p = v3(ExtractFloat(recPX, i), ExtractFloat(recPY, i), ExtractFloat(recPZ, i));
                rec[i].t = ExtractFloat(root, i);
                v3 outNormal = (rec[i].p - center) / radius;
                rec[i].SetFaceNormal(r[i], outNormal);
                rec[i].material = material;
            }
        }

        u64 cycleEnd = __rdtsc();
        HitCycles += cycleEnd - cycleBegin;
    }
    
    bool SimpleHit(Ray& r, f32 tMin, f32 tMax)
    {
        v3 oc = r.origin - center;
        f32 a = r.direction.LengthSquared();
        f32 halfB = Dot(oc, r.direction);
        f32 c = oc.LengthSquared() - radius*radius;
        
        f32 discriminant = halfB*halfB - a*c;
        return discriminant > 0;
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
        f32 denom = Dot(r.direction, normal);
        if(Abs(denom) <= ZERO)
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

    void Hit(Ray r[4], f32 tMin[4], f32 tMax[4], HitRecord rec[4])
    {
        ++HitCounter;
        u64 cycleBegin = __rdtsc();

        // f32 denom = Dot(r.direction, normal);
        wide_f32 rayDirectionX = WideFloatSetIndividual(r[3].direction.x, r[2].direction.x, r[1].direction.x, r[0].direction.x);
        wide_f32 rayDirectionY = WideFloatSetIndividual(r[3].direction.y, r[2].direction.y, r[1].direction.y, r[0].direction.y);
        wide_f32 rayDirectionZ = WideFloatSetIndividual(r[3].direction.z, r[2].direction.z, r[1].direction.z, r[0].direction.z);
        wide_f32 normalX = WideFloatSetAll(normal.x);
        wide_f32 normalY = WideFloatSetAll(normal.y);
        wide_f32 normalZ = WideFloatSetAll(normal.z);
        
        wide_f32 denom = WideFloatAdd(WideFloatMultiply(rayDirectionX, normalX), WideFloatAdd(WideFloatMultiply(rayDirectionY, normalY), WideFloatMultiply(rayDirectionZ, normalZ)));

        wide_f32 zero = WideFloatSetAll(ZERO);
        wide_f32 zeroNegated = WideFloatSubtract(WideFloatSetAll(0.0f), zero);
        wide_i32 onesMask = WideIntSetAll(0xFFFFFFFF);

        //if not(denom < -ZERO || denom > ZERO)
        wide_i32 wideResults = WideIntOr(WideCastFloatToInt(WideFloatLess(denom, zeroNegated)), WideCastFloatToInt(WideFloatGreater(denom, zero)));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }
        
        // f32 num = Dot((point - r.origin), normal);
        wide_f32 pointX = WideFloatSetAll(point.x);
        wide_f32 pointY = WideFloatSetAll(point.y);
        wide_f32 pointZ = WideFloatSetAll(point.z);
        wide_f32 rayOriginX = WideFloatSetIndividual(r[3].origin.x, r[2].origin.x, r[1].origin.x, r[0].origin.x);
        wide_f32 rayOriginY = WideFloatSetIndividual(r[3].origin.y, r[2].origin.y, r[1].origin.y, r[0].origin.y);
        wide_f32 rayOriginZ = WideFloatSetIndividual(r[3].origin.z, r[2].origin.z, r[1].origin.z, r[0].origin.z);
        wide_f32 pointMinusRayOriginX = WideFloatSubtract(pointX, rayOriginX);
        wide_f32 pointMinusRayOriginY = WideFloatSubtract(pointY, rayOriginY);
        wide_f32 pointMinusRayOriginZ = WideFloatSubtract(pointZ, rayOriginZ);
        wide_f32 num = WideFloatAdd(WideFloatMultiply(pointMinusRayOriginX, normalX), WideFloatAdd(WideFloatMultiply(pointMinusRayOriginY, normalY), WideFloatMultiply(pointMinusRayOriginZ, normalZ)));

        // f32 t = num / denom;
        wide_f32 t = WideFloatDivide(num, denom);
        
        // if(t > tMin && t < tMax)
        wide_f32 wideTMin = WideFloatSetIndividual(tMin[3], tMin[2], tMin[1], tMin[0]); 
        wide_f32 wideTMax = WideFloatSetIndividual(tMax[3], tMax[2], tMax[1], tMax[0]); 
        wideResults = WideIntAnd(wideResults, WideIntAnd(WideCastFloatToInt(WideFloatGreater(t, wideTMin)), WideCastFloatToInt(WideFloatLess(t, wideTMax))));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }


        wide_f32 rayAtTX = WideFloatAdd(rayOriginX , WideFloatMultiply(t, rayDirectionX));
        wide_f32 rayAtTY = WideFloatAdd(rayOriginY , WideFloatMultiply(t, rayDirectionY));
        wide_f32 rayAtTZ = WideFloatAdd(rayOriginZ , WideFloatMultiply(t, rayDirectionZ));
        for(int i = 0; i < 4; ++i)
        {
            if (ExtractInt(wideResults, i))
            {
                rec[i].p = v3(ExtractFloat(rayAtTX, i), ExtractFloat(rayAtTY, i), ExtractFloat(rayAtTZ, i));
                rec[i].t = ExtractFloat(t, i);
                rec[i].SetFaceNormal(r[i], normal);
                rec[i].material = material;
            }
        }

        u64 cycleEnd = __rdtsc();
        HitCycles += cycleEnd - cycleBegin;
    }
};

class Triangle : public Hittable
{
    public:
    
    p3 a,b,c;
    v3 edge1;
    v3 edge2;
    v3 normal;
    
    v3 ab, bc, ca;
    
    Material *material;
    
    Triangle(p3 v0, p3 v1, p3 v2, Material *m) : a(v0), b(v1), c(v2),  material(m)
    {
        edge1 = b - a;
        edge2 = c - a;
        normal = Cross(edge1, edge2);
        
        // NOTE(mevex): This are used for the non MOLLER_TRUMBORE implementation
        ab = b-a;
        bc = c-b;
        ca = a-c;
    }
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) override
    {
        // NOTE(mevex): MOLLER TRUMBORE ALGORITHM
        // NOTE(mevex): See scratchpixel's explanation of the algorithm for details on how it works and the variable names
        
        v3 T = r.origin - a;
        
        v3 P = Cross(r.direction, edge2);
        v3 Q = Cross(T, edge1);
        
        f32 determinant = Dot(P, edge1);
        // NOTE(mevex): if the triangle and the ray are parallel (therefore there is no intersection) or if the triangle is backfacing the ray
        if(Abs(determinant) <= ZERO)
            return false;
        
        f32 inverseDet = 1.0f / determinant;
        
        f32 u = Dot(P, T) * inverseDet;
        f32 v = Dot(Q, r.direction) * inverseDet;
        if(u < 0 || v < 0 || u+v > 1)
            return false;
        
        f32 t = Dot(Q, edge2) * inverseDet;
        if(t < tMin || t > tMax)
            return false;
        
        rec.p = r.At(t);
        rec.t = t;
        rec.SetFaceNormal(r, normal);
        rec.material = material;
        rec.SetBarycentrics(u, v);
        
        return true;
    }

    void Hit(Ray r[4], f32 tMin[4], f32 tMax[4], HitRecord rec[4])
    {
        ++HitCounter;
        u64 cycleBegin = __rdtsc();

        // NOTE(mevex): we know for sure that the origin is the same for every ray
        // v3 T = r.origin - a;
        v3 T = r[0].origin - a;
        wide_f32 TX = WideFloatSetAll(T.x);
        wide_f32 TY = WideFloatSetAll(T.y);
        wide_f32 TZ = WideFloatSetAll(T.z);

        v3 P0 = Cross(r[0].direction, edge2);
        v3 P1 = Cross(r[1].direction, edge2);
        v3 P2 = Cross(r[2].direction, edge2);
        v3 P3 = Cross(r[3].direction, edge2);
        wide_f32 PX = WideFloatSetIndividual(P3.x, P2.x, P1.x, P0.x);
        wide_f32 PY = WideFloatSetIndividual(P3.y, P2.y, P1.y, P0.y);
        wide_f32 PZ = WideFloatSetIndividual(P3.z, P2.z, P1.z, P0.z);

        v3 Q = Cross(T, edge1);
        wide_f32 QX = WideFloatSetAll(Q.x);
        wide_f32 QY = WideFloatSetAll(Q.y);
        wide_f32 QZ = WideFloatSetAll(Q.z);
        wide_f32 edge1X = WideFloatSetAll(edge1.x);
        wide_f32 edge1Y = WideFloatSetAll(edge1.y);
        wide_f32 edge1Z = WideFloatSetAll(edge1.z);

        // f32 determinant = Dot(P, edge1);
        wide_f32 determinant = WideFloatAdd(WideFloatMultiply(PX, edge1X), WideFloatAdd(WideFloatMultiply(PY, edge1Y), WideFloatMultiply(PZ, edge1Z)));
        
        wide_f32 zero = WideFloatSetAll(ZERO);
        wide_f32 zeroNegated = WideFloatSubtract(WideFloatSetAll(0.0f), zero);
        wide_i32 onesMask = WideIntSetAll(0xFFFFFFFF);

        //if not(determinant < -ZERO || determinant > ZERO)
        wide_i32 wideResults = WideIntOr(WideCastFloatToInt(WideFloatLess(determinant, zeroNegated)), WideCastFloatToInt(WideFloatGreater(determinant, zero)));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }

        // f32 inverseDet = 1.0f / determinant;
        wide_f32 inverseDet = WideFloatDivide(WideFloatSetAll(1.0f), determinant);
        
        // f32 u = Dot(P, T) * inverseDet;
        wide_f32 DotPT = WideFloatAdd(WideFloatMultiply(PX, TX), WideFloatAdd(WideFloatMultiply(PY, TY), WideFloatMultiply(PZ, TZ)));
        wide_f32 wideU = WideFloatMultiply(DotPT, inverseDet);
        
        // f32 v = Dot(Q, r.direction) * inverseDet;
        wide_f32 rayDirectionX = WideFloatSetIndividual(r[3].direction.x, r[2].direction.x, r[1].direction.x, r[0].direction.x);
        wide_f32 rayDirectionY = WideFloatSetIndividual(r[3].direction.y, r[2].direction.y, r[1].direction.y, r[0].direction.y);
        wide_f32 rayDirectionZ = WideFloatSetIndividual(r[3].direction.z, r[2].direction.z, r[1].direction.z, r[0].direction.z);
        wide_f32 DotQDir = WideFloatAdd(WideFloatMultiply(QX, rayDirectionX), WideFloatAdd(WideFloatMultiply(QY, rayDirectionY), WideFloatMultiply(QZ, rayDirectionZ)));
        wide_f32 wideV = WideFloatMultiply(DotQDir, inverseDet);

        // if not(u > 0 && v > 0 && u+v < 1)
        wide_f32 trueZero = WideFloatSetAll(0.0f);
        wide_i32 uGreaterThanZero = WideCastFloatToInt(WideFloatGreater(wideU, trueZero));
        wide_i32 vGreaterThanZero = WideCastFloatToInt(WideFloatGreater(wideV, trueZero));
        wide_i32 uPlusVLessThanOne = WideCastFloatToInt(WideFloatLess(WideFloatAdd(wideU, wideV), WideFloatSetAll(1.0f)));
        wideResults = WideIntAnd(wideResults, WideIntAnd(uPlusVLessThanOne, WideIntAnd(uGreaterThanZero, vGreaterThanZero)));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }
        
        // f32 t = Dot(Q, edge2) * inverseDet;
        wide_f32 edge2X = WideFloatSetAll(edge2.x);
        wide_f32 edge2Y = WideFloatSetAll(edge2.y);
        wide_f32 edge2Z = WideFloatSetAll(edge2.z);
        wide_f32 dotQEdge2 = WideFloatAdd(WideFloatMultiply(QX, edge2X), WideFloatAdd(WideFloatMultiply(QY, edge2Y), WideFloatMultiply(QZ, edge2Z)));
        wide_f32 t = WideFloatMultiply(dotQEdge2, inverseDet);

        // if not(t > tMin && t < tMax)
        wide_f32 wideTMin = WideFloatSetIndividual(tMin[3], tMin[2], tMin[1], tMin[0]); 
        wide_f32 wideTMax = WideFloatSetIndividual(tMax[3], tMax[2], tMax[1], tMax[0]); 
        wideResults = WideIntAnd(wideResults, WideIntAnd(WideCastFloatToInt(WideFloatGreater(t, wideTMin)), WideCastFloatToInt(WideFloatLess(t, wideTMax))));
        if (WideIntTestAllZeros(onesMask, wideResults))
        {
            return;
        }

        // TODO(mevex): simd this once the function takes simd arguments
        // rec.p = r.At(t);
        wide_f32 rayOriginX = WideFloatSetIndividual(r[3].origin.x, r[2].origin.x, r[1].origin.x, r[0].origin.x);
        wide_f32 rayOriginY = WideFloatSetIndividual(r[3].origin.y, r[2].origin.y, r[1].origin.y, r[0].origin.y);
        wide_f32 rayOriginZ = WideFloatSetIndividual(r[3].origin.z, r[2].origin.z, r[1].origin.z, r[0].origin.z);
        wide_f32 rayAtTX = WideFloatAdd(rayOriginX , WideFloatMultiply(t, rayDirectionX));
        wide_f32 rayAtTY = WideFloatAdd(rayOriginY , WideFloatMultiply(t, rayDirectionY));
        wide_f32 rayAtTZ = WideFloatAdd(rayOriginZ , WideFloatMultiply(t, rayDirectionZ));

        for(int i = 0; i < 4; ++i)
        {
            if (ExtractInt(wideResults, i))
            {
                rec[i].p = v3(ExtractFloat(rayAtTX, i), ExtractFloat(rayAtTY, i), ExtractFloat(rayAtTZ, i));
                rec[i].t = ExtractFloat(t, i);
                rec[i].SetFaceNormal(r[i], normal);
                rec[i].SetBarycentrics(ExtractFloat(wideU, i), ExtractFloat(wideV, i));
                rec[i].material = material;
            }
        }

        u64 cycleEnd = __rdtsc();
        HitCycles += cycleEnd - cycleBegin;
    }
};

class Mesh : public Hittable
{
    public:
    
    p3 position;
    Sphere boundingSphere;
    
    vector<Lambertian> materials;
    vector<Triangle> triangles;
    
    Mesh(p3 p, p3 relSpherePos, f32 sphereRadius) : position(p)
    {
        boundingSphere = Sphere(relSpherePos + position, sphereRadius);
    }
    
    void AddTriangle(Triangle t)
    {
        t.a += position;
        t.b += position;
        t.c += position;
        triangles.push_back(t);
    }
    
    void AddMaterial(Lambertian &m)
    {
        materials.push_back(m);
    }
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec) override
    {
        bool result = false;
        f32 closestT = tMax;
        HitRecord tmpRec = {};
        
        if(!boundingSphere.SimpleHit(r, tMin, closestT))
            return false;
        
        for(Triangle t : triangles)
        {
            if(t.Hit(r, tMin, closestT, tmpRec))
            {
                result = true;
                closestT = tmpRec.t;
                rec = tmpRec;
            }
        }
        
        return result;
    }

    void Hit(Ray r[4], f32 tMin[4], f32 tMax[4], HitRecord rec[4])
    {
        // Chek if the bounding sphere is ever hit by any ray
        bool shouldTest = false;
        for (i32 i = 0; i < 4; i++)
        {
            if(boundingSphere.SimpleHit(r[i], tMin[i], tMax[i]))
            {
                shouldTest = true;
                break;
            }
        }

        if (shouldTest)
        {
            for(Triangle t : triangles)
            {
                HitRecord tempRec[4] = {};
                t.Hit(r, tMin, tMax, tempRec);
                for (i32 i = 0; i < 4; i++)
                {
                    if (tempRec[i].t != INFINITY && tempRec[i].t < rec[i].t)
                    {
                        tMax[i] = tempRec[i].t;
                        rec[i] = tempRec[i];
                    }
                }
            }
        }
    }
};

#endif //HITTABLE_H