#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "main.h"

/* 
f32 HitSphere(p3& center, f32 radius, Ray& r) {
    v3 oc = r.origin - center;
    f32 a = r.direction.LengthSquared();
    f32 halfB = Dot(oc, r.direction);
    f32 c = oc.LengthSquared() - radius*radius;
    
    f32 discriminant = halfB*halfB - a*c;
    if(discriminant < 0)
        return -1;
    else
        return (-halfB -sqrt(discriminant)) / (2.0f*a);
}
 */

color GetRayColor(Ray& r, Sphere& s)
{
    // TODO(mevex): Placeholder code for now
    HitRecord rec;
    bool result = s.Hit(r, 0.01f, 100.0f, rec);
    
    if(result)
    {
        return 0.5f * (rec.normal + color(1,1,1));
    }
    v3 unitDir = Unit(r.direction);
    rec.t = 0.5f*(unitDir.y + 1.0f);
    return (1.0f-rec.t)*color(1.0f, 1.0f, 1.0f) + rec.t*color(0.5f, 0.7f, 1.0f);
}

int main()
{
    Canvas canvas(1280, 720, 4);
    Camera camera(2.0f, canvas.ratio, v3(0,0,0));
    Sphere s(p3(0,0,-1), 0.5f);
    
    for(int y = canvas.height-1; y >= 0; y--)
    {
        printf("\rProgress: %i%% line %i/%i", (int)((f32)y/(f32)canvas.height*100.99f), y+1, canvas.height);
        for(int x = 0; x < canvas.width; x++)
        {
            f32 u = (f32)x / (f32)(canvas.width - 1);
            f32 v = (f32)y / (f32)(canvas.height - 1);
            
            v3 rayDirection = camera.vpLowerLeftCorner + u*camera.vpHorizontal + v*camera.vpVertical - camera.position;
            Ray r(camera.position, rayDirection);
            color c = GetRayColor(r, s);
            
            canvas.SetPixel(x, y, c);
        }
    }
    
    // NOTE(mevex): Pixel order: AABBGGRR
    auto res = stbi_write_png("../renders/render.png", canvas.width, canvas.height, canvas.bytesPerPixel, canvas.memory, 0);
    
    printf("stop!");
    
    return 0;
}