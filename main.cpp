#include "main.h"
// NOTE(mevex): Global variables
#include <chrono>

Color GetRayColor(Ray& r, Scene& scene, int depth)
{
    HitRecord rec;
    bool result = scene.Hit(r, 0.0001f, 100.0f, rec);
    
    if(depth <= 0)
        return Color(0,0,0);
    
    
    if(result)
    {
        Ray scattered;
        Color attenuation;
        if(rec.material->scatter(r, rec, attenuation, scattered))
            return attenuation * GetRayColor(scattered, scene, depth-1);
        return Color(0,0,0);
    }
    
    v3 unitDir = Unit(r.direction);
    rec.t = 0.5f*(unitDir.y + 1.0f);
    return (1.0f-rec.t)*Color(1.0f, 1.0f, 1.0f) + rec.t*Color(0.5f, 0.7f, 1.0f);
}

int main()
{
    srand ((u32)time(NULL));
    
    int samplePerPixel = 32;
    int maxDepth = 8;
    
    Canvas canvas(1280, 720, 4);
    Camera camera(p3(0,0,4), v3(0,0,-1), v3(0,1,0), 50.0f, canvas.ratio);
    Scene scene;
    
    // NOTE(mevex): Scene creation
    Lambertian ground(Color(0.8f, 0.8f, 0.0f));
    Lambertian center(Color(0.7f, 0.3f, 0.3f));
    Metal left(Color(0.8f, 0.8f, 0.8f), 0.3f);
    Metal right(Color(0.8f, 0.6f, 0.2f), 1.0f);
    VertexColor tri(Color(1,0,0), Color(0,1,0), Color(0,0,1));
    
    //Sphere s1(p3(0,-100.5,-1), 100, &ground);
    Plane p1(p3(0,-0.5f,0), v3(0,1,0), &ground);
    Sphere s2(p3(0,0,0), 0.5f, &center);
    Sphere s3(p3(-1,0,-1), 0.5f, &left);
    Sphere s4(p3(1,0,-1), 0.5f, &right);
    Triangle t5(p3(-1,1,-2), p3(1,1,-2), p3(0,2,-1), &tri);
    //Triangle t6(p3(-1,1.2f,-2), p3(1,1.2f,-2), p3(0,2.2f,-1), &tri);
    //Triangle t7(p3(-1,1.4f,-2), p3(1,1.4f,-2), p3(0,2.4f,-1), &tri);
    //Sphere s6(p3(-1,1,-2), 0.1f, &tri);
    //Sphere s7(p3(1,1,-2), 0.1f, &tri);
    //Sphere s8(p3(0,2,-1), 0.1f, &tri);
    
    scene.Add(&p1);
    scene.Add(&s2);
    scene.Add(&s3);
    scene.Add(&s4);
    scene.Add(&t5);
    //scene.Add(&t6);
    //scene.Add(&t7);
    //scene.Add(&s6);
    //scene.Add(&s7);
    //scene.Add(&s8);
    
    printf("Rendering starts!\n");
    auto timerStart = std::chrono::high_resolution_clock::now();
    
    for(int y = canvas.height-1; y >= 0; y--)
    {
        printf("\rProgress: %i%% lines remaining %i/%i", (int)((f32)(canvas.height-y)/(f32)canvas.height*100.99f), y+1, canvas.height);
        for(int x = 0; x < canvas.width; x++)
        {
            Color c(0,0,0);
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
    
    auto timerFinish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerFinish - timerStart);
    //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(timerFinish - timerStart);
    auto avgCount = duration.count() / (canvas.width * canvas.height);
    
    // NOTE(mevex): Pixel order: AABBGGRR
    auto res = stbi_write_png("../renders/render.png", canvas.width, canvas.height, canvas.bytesPerPixel, canvas.memory, 0);
    
    printf("\nTime elapsed: %ims", (int)(duration.count()));
    getchar();
    
    return 0;
}