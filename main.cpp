#include "main.h"
#include <chrono>

Color GetRayColor(Ray& r, Scene& scene, int depth)
{
    // NOTE(mevex): Background/ambient light hack
    v3 unitDir = Unit(r.direction);
    f32 t = 0.5f*(unitDir.y + 1.0f);
    Color falseAmbientColor = Lerp(Color(0.6f, 0.6f, 0.6f), Color(0.5f, 0.7f, 1.0f), t);
    
    if(depth <= 0)
        return falseAmbientColor;
    
    HitRecord rec;
    bool hitResult = scene.Hit(r, ZERO, INFINITY, rec);
    
    if(hitResult)
    {
        // NOTE(mevex): If the light intensity exceeds 1 we get an overexposed color
        f32 lightIntensity = Min(scene.GetLightIntensity(rec.normal, rec.p), 1.0f);
        
        Ray scattered;
        Color attenuation;
        if(rec.material->Scatter(r, rec, attenuation, scattered))
            return attenuation * lightIntensity * GetRayColor(scattered, scene, depth-1);
        else
            return attenuation * lightIntensity;
    }
    
    return falseAmbientColor;
}

bool LoadObj(Mesh& mesh, const char* filename, const char* basepath = NULL, bool triangulate = true)
{
    printf("Loading %s\n", filename);
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);
    
    // TODO(mevex): Take care of this strings stuff
    if (!warn.empty()) {
        printf("WARN: %s\n", warn.c_str());
    }
    
    if (!err.empty()) {
        printf("ERR: %s\n", err.c_str());
    }
    
    if (!ret) {
        printf("Failed to load/parse .obj.\n");
        return false;
    }
    
    for(tinyobj::material_t m : materials)
    {
        Color albedo(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
        Lambertian mat(albedo);
        mesh.AddMaterial(mat);
    }
    
    // NOTE(mevex): This routine works only if the mesh has been triangulated
    int facesCount = (int)shapes[0].mesh.num_face_vertices.size();
    tinyobj::index_t *indexPtr = &shapes[0].mesh.indices[0];
    int *materialIndex = &shapes[0].mesh.material_ids[0];
    for(int i = 0; i < facesCount; i++)
    {
        v3 a(attrib.vertices[3 * indexPtr->vertex_index], attrib.vertices[3 * indexPtr->vertex_index + 1], attrib.vertices[3 * indexPtr->vertex_index + 2]);
        indexPtr += 1;
        v3 b(attrib.vertices[3 * indexPtr->vertex_index], attrib.vertices[3 * indexPtr->vertex_index + 1], attrib.vertices[3 * indexPtr->vertex_index + 2]);
        indexPtr += 1;
        v3 c(attrib.vertices[3 * indexPtr->vertex_index], attrib.vertices[3 * indexPtr->vertex_index + 1], attrib.vertices[3 * indexPtr->vertex_index + 2]);
        indexPtr += 1;
        
        Lambertian *m = &mesh.materials[materialIndex[i]];
        
        Triangle t(a, b, c, m);
        mesh.AddTriangle(t);
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("OBJ loading time: %ims", (int)(d.count()));
    
    //getchar();
    return true;
}

int main()
{
    srand ((u32)time(NULL));
    
    int samplePerPixel = 200;
    int maxDepth = 20;
    
    Canvas canvas(1280, 720, 4);
    //Camera camera(p3(3,9,12), p3(0.5f,3.7f,0), v3(0,1,0), 55, canvas.ratio);
    Camera camera(p3(0,5,12), p3(1,4,-1), v3(0,1,0), 50, canvas.ratio);
    Scene scene;
    
    // NOTE(mevex): Scene creation
    // NOTE(mevex): Materials
    Lambertian ground(Color(0.8f, 0.8f, 0.0f));
    Lambertian center(Color(0.7f, 0.3f, 0.3f));
    Metal left(Color(0.8f, 0.8f, 0.8f), 0.3f);
    Metal right(Color(0.05f, 0.6f, 0.73f), 0.0f);
    VertexColor tri(Color(1,0,0), Color(0,1,0), Color(0,0,1));
    
    // NOTE(mevex): Objects
    Plane p1(p3(0,-0.5f,0), v3(0,1,0), &ground);//-4.1139f
    Sphere s2(p3(-5 ,1.5f, 1), 2.0f, &right);//-3.6139f
    //Sphere s3(p3(-1,0,-1), 0.5f, &left);
    //Sphere s4(p3(1,0,-1), 0.5f, &right);
    //Triangle t5(p3(-1,1,-2), p3(1,1,-2), p3(0,2,-1), &tri);
    
    Mesh fox(p3(0,3.65f,0), p3(-0.8f, 0, 0), 7.4f); // r = 7.4f
    LoadObj(fox, "../models/fox2.obj", "../models/", true);
    
    // NOTE(mevex): Lights
    PointLight l1(p3(-0.5f,10,5), 0.7f);
    AmbientLight l2(0.3f);
    
    scene.Add(&p1);
    scene.Add(&s2);
    //scene.Add(&s3);
    //scene.Add(&s4);
    scene.Add(&fox);
    scene.Add(&l1);
    scene.Add(&l2);
    
    printf("Rendering starts\n");
    printf("Samples per pixel: %d Max depth: %d\n", samplePerPixel, maxDepth);
    auto timerStart = std::chrono::high_resolution_clock::now();
    
    for(int y = canvas.height-1; y >= 0; y--)
    {
        printf("\rProgress: %i%% lines remaining %i/%i", (int)((f32)(canvas.height-y)/(f32)canvas.height*100.99f), y, canvas.height);
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
    auto avgCount = std::chrono::duration_cast<std::chrono::nanoseconds>(timerFinish - timerStart).count() / (canvas.width * canvas.height);
    
    // NOTE(mevex): Pixel order: AABBGGRR
    auto res = stbi_write_png("../renders/render.png", canvas.width, canvas.height, canvas.bytesPerPixel, canvas.memory, 0);
    
    printf("\nRendering time: %ims", (int)(duration.count()));
    printf("\nAverage pixel time: %ins", (int)avgCount);
    
    getchar();
    return 0;
}