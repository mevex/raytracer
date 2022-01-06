#ifndef V3_H
#define V3_H

#include <cmath>

class v3
{
    public:
    union
    {
        f32 e[3];
        struct
        {
            f32 x, y, z;
        };
        struct
        {
            f32 r, g, b;
        };
    };
    
    v3(f32 e0 = 0, f32 e1 = 0, f32 e2 = 0) : e{e0, e1, e2} {}
    
    inline f32 LengthSquared()
    {
        f32 result = e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        return result;
    }
    
    inline f32 Length()
    {
        f32 result = sqrt(LengthSquared());
        return result;
    }
    
    inline shared_function v3 Random()
    {
        v3 result = {RandomFloat(), RandomFloat(), RandomFloat()};
        return result;
    }
    
    inline shared_function v3 Random(f32 min, f32 max)
    {
        v3 result = {RandomFloat(min,max), RandomFloat(min,max), RandomFloat(min,max)};
        return result;
    }
    
    shared_function v3 RandomInUnitSphere()
    {
        while(true)
        {
            v3 p = Random(-1, 1);
            if(p.LengthSquared() <= 1)
                return p;
        }
    }
    
    inline v3& operator+= (v3& v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }
    
    inline v3& operator*= (f32 t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }
};

inline v3 operator+ (v3& v, v3& w)
{
    v3 result = v3(v.e[0] + w.e[0], v.e[1] + w.e[1], v.e[2] + w.e[2]);
    return result;
}

inline v3 operator- (v3& v, v3& w)
{
    v3 result = v3(v.e[0] - w.e[0], v.e[1] - w.e[1], v.e[2] - w.e[2]);
    return result;
}

inline v3 operator- (v3& v)
{
    v3 result = v3(-v.e[0], -v.e[1], -v.e[2]);
    return result;
}

inline v3 operator* (v3 v, f32 t)
{
    v3 result = v3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
    return result;
}


inline v3 operator* (f32 t, v3 v)
{
    v3 result = v * t;
    return result;
}


inline v3 operator/ (v3 v, f32 t)
{
    v3 result = v * (1.0f/t);
    return result;
}

inline f32 Dot(v3& v, v3&w)
{
    f32 result = v.e[0] * w.e[0] + v.e[1] * w.e[1] + v.e[2] * w.e[2];
    return result;
}

inline v3 Cross(v3& v, v3& w) 
{
    v3 result = {v.e[1] * w.e[2] - v.e[2] * w.e[1],
        v.e[2] * w.e[0] - v.e[0] * w.e[2],
        v.e[0] * w.e[1] - v.e[1] * w.e[0]};
    return result;
}

inline v3  Unit(v3& v)
{
    v3 result = v / v.Length();
    return result;
}

typedef v3 p3;
typedef v3 color;

#endif //V3_H
