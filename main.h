#ifndef MAIN_H
#define MAIN_H

// NOTE: This is a collection of macros and typedefs widely used throughout the project

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
#define internal_function static

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#include "v3.h"
#include "ray.h"
#include "hittable.h"

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
    
    void SetPixel(i32 x, i32 y, u8 red, u8 green, u8 blue)
    {
        red = Min(red, 255);
        green = Min(green, 255);
        blue = Min(blue, 255);
        
        i32* pixel = (i32*)memory;
        pixel += (height-y-1)*width + x;
        *pixel = 255<<24 | blue << 16 | green << 8 | red;
    }
    
    void SetPixel(i32 x, i32 y, f32 red, f32 green, f32 blue)
    {
        u8 r,g,b;
        
        r = (u8)(255.99f * red);
        g = (u8)(255.99f * green);
        b = (u8)(255.99f * blue);
        
        SetPixel(x, y, r, g, b);
    }
    
    void SetPixel(i32 x, i32 y, color c)
    {
        SetPixel(x, y, c.r, c.g, c.b);
    }
    
    ~Canvas()
    {
        // NOTE(mevex): no need to free the memory since the canvas will be destroyed when the program closes
    }
};

class Camera
{
    public:
    f32 vpWidth;
    f32 vpHeight;
    f32 focalLength;
    
    p3 position;
    //p3 direction;
    v3 vpHorizontal;
    v3 vpVertical;
    p3 vpLowerLeftCorner;
    
    Camera(f32 vph, f32 ratio, p3 pos)
    {
        vpHeight = vph;
        vpWidth = ratio*vph;
        focalLength = 1.0f;
        
        position = pos;
        vpHorizontal = {vpWidth, 0, 0};
        vpVertical = {0, vpHeight, 0};
        vpLowerLeftCorner = position - vpHorizontal/2.f - vpVertical/2.f  - v3(0, 0, focalLength);
    }
};

#endif //MAIN_H
