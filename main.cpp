#include "main.h"

// NOTE(mevex): Global variables

color GetRayColor(Ray& r, Scene& scene, int depth)
{
    HitRecord rec;
    bool result = scene.Hit(r, 0.0001f, 100.0f, rec);
    
    
    if(depth <= 0)
        return color(0,0,0);
    
    
    if(result)
    {
        p3 target = rec.p + rec.normal + v3::RandomInUnitSphere();
        Ray newR(rec.p, target);
        return 0.5f * GetRayColor(newR, scene, depth-1);
        
    }
    v3 unitDir = Unit(r.direction);
    rec.t = 0.5f*(unitDir.y + 1.0f);
    return (1.0f-rec.t)*color(1.0f, 1.0f, 1.0f) + rec.t*color(0.5f, 0.7f, 1.0f);
}

int main()
{
    srand ((u32)time(NULL));
    
    int samplePerPixel = 100;
    int maxDepth = 50;
    
    Canvas canvas(1280, 720, 4);
    Camera camera(2.0f, canvas.ratio, v3(0,0,0));
    Scene scene;
    
    // NOTE(mevex): Scene creation
    Sphere s1(p3(0,0,-1), 0.5f);
    Sphere s2(p3(0,-100.5,-1), 100);
    scene.Add(&s1);
    scene.Add(&s2);
    
    for(int y = canvas.height-1; y >= 0; y--)
    {
        printf("\rProgress: %i%% lines remaining %i/%i", (int)((f32)(canvas.height-y)/(f32)canvas.height*100.99f), y+1, canvas.height);
        for(int x = 0; x < canvas.width; x++)
        {
            color c(0,0,0);
            for(int i = 0; i < samplePerPixel; i++)
            {
                f32 u = ((f32)x + RandomFloat()) / (f32)(canvas.width - 1);
                f32 v = ((f32)y + RandomFloat()) / (f32)(canvas.height - 1);
                
                Ray r = camera.GetRay(u, v);
                c += GetRayColor(r, scene, maxDepth);
                
            }
            canvas.SetPixel(x, y, c, samplePerPixel);
        }
    }
    
    // NOTE(mevex): Pixel order: AABBGGRR
    auto res = stbi_write_png("../renders/render.png", canvas.width, canvas.height, canvas.bytesPerPixel, canvas.memory, 0);
    
    printf("stop!");
    
    return 0;
}