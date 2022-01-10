#ifndef MAIN_H
#define MAIN_H

// NOTE: This is a collection of utility macros and typedefs

#include <cstdint>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef u8 byte;

#define local_persist static
#define global_variable static
#define shared_function static

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Abs(a) ((a) < 0 ? -(a) : (a))

#include <limits>
#undef INFINITY
#define INFINITY std::numeric_limits<float>::infinity()
#define PI 3.1415926535897932385f
#define ZERO 1e-6f

// NOTE(mevex): Utility Functions

inline f32 DegreesToRadians(f32 degrees)
{
    f32 result = degrees * PI / 180.0f;
    return result;
}

#include <time.h>
inline f32 RandomFloat()
{
    // Returns a random real number in [0,1)
    f32 result = rand() / (RAND_MAX + 1.0f);
    return result;
}

inline f32 RandomFloat(f32 min, f32 max)
{
    // Returns a random real number in [min,max)
    f32 result = min + (max-min)*RandomFloat();
    return result;
}


#include "v3.h"
#include "ray.h"
#include "hittable.h"
#include "material.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <vector>
using std::vector;


class Canvas
{
    public:
    
    void* memory;
    i32 width;
    i32 height;
    i32 bytesPerPixel;
    f32 ratio;
    
    Canvas(i32 w, i32 h, i32 bpp)
    {
        width = w;
        height = h;
        bytesPerPixel = bpp;
        ratio = (f32)w / (f32)h;
        memory = malloc(bpp * w * h);
    }
    
    void SetPixel(i32 x, i32 y, f32 red, f32 green, f32 blue)
    {
        u8 r,g,b;
        
        r = (u8)(255.99f * sqrt(red));
        g = (u8)(255.99f * sqrt(green));
        b = (u8)(255.99f * sqrt(blue));
        
        red = Min(red, 255);
        green = Min(green, 255);
        blue = Min(blue, 255);
        
        i32* pixel = (i32*)memory;
        pixel += (height-y-1)*width + x;
        *pixel = 255<<24 | b << 16 | g << 8 | r;
    }
    
    void SetPixel(i32 x, i32 y, Color c)
    {
        SetPixel(x, y, c.r, c.g, c.b);
    }
    
    void SetPixel(i32 x, i32 y, Color c, int spp)
    {
        f32 scale = 1.0f / spp;
        c *= scale;
        SetPixel(x, y, c.r, c.g, c.b);
    }
    
    ~Canvas()
    {
        // NOTE(mevex): no need to free the memory since the canvas will be destroyed only when the program closes
    }
};

class Camera
{
    public:
    p3 position;
    
    v3 vpHorizontal;
    v3 vpVertical;
    p3 vpLowerLeftCorner;
    
    Camera(p3 pos, v3 direction, v3 viewUp, f32 verticalFOV, f32 aspectRatio)
    {
        f32 theta = DegreesToRadians(verticalFOV);
        f32 h = tan(theta/2);
        f32 vpHeight = 2.0f * h;
        f32 vpWidth = vpHeight * aspectRatio;
        f32 focalLength = 1.0f;
        
        position = pos;
        v3 w = Unit(position - direction);
        v3 u = Unit(Cross(viewUp, w));
        v3 v = Cross(w, u);
        
        vpHorizontal = v3(vpWidth, 0, 0);
        vpVertical = v3(0, vpHeight, 0);
        vpLowerLeftCorner = position - vpHorizontal/2.f - vpVertical/2.f  - v3(0, 0, focalLength);
    }
    
    inline Ray GetRay(f32 u, f32 v)
    {
        v3 rayDirection = vpLowerLeftCorner + u*vpHorizontal + v*vpVertical - position;
        Ray result(position, rayDirection);
        return result;
    }
};

class Scene
{
    public:
    
    vector<Hittable *> objects;
    
    Scene()
    {
        // TODO(mevex): Aggiungere cose?
    }
    
    void Add(Hittable *obj)
    {
        // TODO(mevex): Modificare e rendere più facile da usare
        objects.push_back(obj);
    }
    
    bool Hit(Ray& r, f32 tMin, f32 tMax, HitRecord& rec)
    {
        bool result = false;
        f32 closestT = tMax;
        HitRecord tmpRec = {};
        
        for(auto& obj : objects)
        {
            if(obj->Hit(r, tMin, closestT, tmpRec) && tmpRec.frontFace)
            {
                result = true;
                closestT = tmpRec.t;
                rec = tmpRec;
            }
        }
        
        return result;
    }
};

#endif //MAIN_H
