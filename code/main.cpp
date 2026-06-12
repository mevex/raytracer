#include <intrin.h>
#include <cstdio>
#include "main.h"
#include <chrono>

// u64 GetRayColorCycles = 0;
// u64 HitCycles = 0;
// u64 ScatterCycles = 0;

// u64 GetRayColorCounter = 0;
// u64 HitCounter = 0;
// u64 ScatterCounter = 0;

#define RUN_FAST 0

Color GetRayColorFast(Ray rays[4], Scene &scene, int depth, Color falseAmbientColor)
{
    // ++GetRayColorCounter;
    u64 cycleBegin = __rdtsc();
    // TODO(mevex): Move false ambient color and ray calculation here

    if (depth <= 0)
    {
        return falseAmbientColor;
    }

    Color attenuations[4] = {falseAmbientColor, falseAmbientColor, falseAmbientColor, falseAmbientColor};
    while (depth)
    {
        HitRecord recs[4] = {};
        f32 closestTs[4] = {INFINITY, INFINITY, INFINITY, INFINITY};
        f32 tMin[4] = {ZERO, ZERO, ZERO, ZERO};

        for (auto &obj : scene.objects)
        {
            HitRecord tempRecs[4] = {};
            // obj->Hit(rays[0], ZERO, INFINITY, tempRec[0]);
            obj->Hit(rays, tMin, closestTs, tempRecs);
            for (int i = 0; i < 4; ++i)
            {
                if (tempRecs[i].t != INFINITY && tempRecs[i].t < recs[i].t)
                {
                    closestTs[i] = tempRecs[i].t;
                    recs[i] = tempRecs[i];
                }
            }
        }

        for (int i = 0; i < 4; ++i)
        {
            if (recs[i].t != INFINITY)
            {
                // NOTE(mevex): If the light intensity exceeds 1 we get an overexposed color
                f32 lightIntensity = Min(scene.GetLightIntensity(recs[i].normal, recs[i].p), 1.0f);

                Color newAttenuation;
                if (recs[i].material->Scatter(rays[i], recs[i], newAttenuation, rays[i]))
                {
                    attenuations[i] = attenuations[i] * lightIntensity * newAttenuation;
                }
                else
                {
                    attenuations[i] = attenuations[i] * lightIntensity * newAttenuation;
                    // NOTE(mevex): this is a bit of a hack to avoid testing this again
                    recs[i].t = 0.0f;
                }
            }
            else
            {
                break;
            }
        }
        --depth;
    }

    u64 cycleEnd = __rdtsc();
    // GetRayColorCycles += cycleEnd - cycleBegin;

    return attenuations[0] + attenuations[1] + attenuations[2] + attenuations[3];
}

Color GetRayColor(Ray &r, Scene &scene, int depth)
{
    // ++GetRayColorCounter;
    u64 cycleBegin = __rdtsc();

    // NOTE(mevex): Background/ambient light hack
    v3 unitDir = Unit(r.direction);
    f32 t = 0.5f * (unitDir.y + 1.0f);
    Color falseAmbientColor = Lerp(Color(0.6f, 0.6f, 0.6f), Color(0.5f, 0.7f, 1.0f), t);

    if (depth <= 0)
        return falseAmbientColor;

    HitRecord rec;
    bool hitResult = scene.Hit(r, ZERO, INFINITY, rec);

    if (hitResult)
    {
        // NOTE(mevex): If the light intensity exceeds 1 we get an overexposed color
        f32 lightIntensity = Min(scene.GetLightIntensity(rec.normal, rec.p), 1.0f);

        Ray scattered;
        Color attenuation;
        if (rec.material->Scatter(r, rec, attenuation, scattered))
            return attenuation * lightIntensity * GetRayColor(scattered, scene, depth - 1);
        else
            return attenuation * lightIntensity;
    }

    u64 cycleEnd = __rdtsc();
    // GetRayColorCycles += cycleEnd - cycleBegin;

    return falseAmbientColor;
}

bool LoadObj(Mesh &mesh, const char *filename, const char *basepath = NULL, bool triangulate = true)
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
    if (!warn.empty())
    {
        printf("WARN: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("ERR: %s\n", err.c_str());
    }

    if (!ret)
    {
        printf("Failed to load/parse .obj.\n");
        return false;
    }

    for (tinyobj::material_t m : materials)
    {
        Color albedo(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
        Lambertian mat(albedo);
        mesh.AddMaterial(mat);
    }

    // NOTE(mevex): This routine works only if the mesh has been triangulated
    int facesCount = (int)shapes[0].mesh.num_face_vertices.size();
    tinyobj::index_t *indexPtr = &shapes[0].mesh.indices[0];
    int *materialIndex = &shapes[0].mesh.material_ids[0];
    for (int i = 0; i < facesCount; i++)
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
    printf("OBJ loading time: %ims\n", (int)(d.count()));

    // getchar();
    return true;
}

#include <windows.h>

// NOTE: Prevents the compiler AND the processor to reorder reads across this boundary
#define ReadBoundary \
    _ReadBarrier();  \
    _mm_lfence();
// NOTE: Prevents the compiler AND the processor to reorder writes across this boundary
#define WriteBoundary \
    _WriteBarrier();  \
    _mm_sfence();
// NOTE: Prevents the compiler AND the processor to reorder reads and writes across this boundary
#define ReadWriteBoundary \
    _ReadWriteBarrier();  \
    _mm_mfence();

struct Job
{
    i32 threadIdx;
    char *string;
};

// TODO: make it a circular buffer
struct JobsQueue
{
public:
    volatile u32 jobsCount;
    volatile u32 nextJobIndex;

    Job *jobs;
    HANDLE semaphore;

    JobsQueue(u64 queueSize)
    {
        jobsCount = 0;
        nextJobIndex = 0;
        jobs = (Job *)malloc(sizeof(Job) * queueSize);
    }

    void PushJob(char *string)
    {
        jobs[jobsCount].string = string;
        jobs[jobsCount].threadIdx = -1; // Impossible default

        // NOTE: We want to make sure that the string is actually stored
        // before incrementing the jobs count
        WriteBoundary;

        jobsCount++;
        ReleaseSemaphore(semaphore, 1, 0);
    }

    bool GetNextJob(Job *outJob, i32 threadIdx)
    {
        while (nextJobIndex < jobsCount)
        {
            i32 currentIndex = nextJobIndex;
            if (currentIndex < jobsCount)
            {
                i32 jobIndex = InterlockedCompareExchange(&nextJobIndex, currentIndex + 1, currentIndex);
                if (jobIndex == currentIndex)
                {
                    *outJob = jobs[jobIndex];
                    outJob->threadIdx = threadIdx;

                    WriteBoundary;
                    printf("Job Index: %d", jobIndex);

                    return true;
                }
            }
        }

        return false;
    }
};

struct ThreadParameters
{
    JobsQueue *queue;
    i32 threadIdx;
};

inline void ExecuteJob(Job *job)
{
    printf("Thread %d: %s\n", job->threadIdx, job->string);
}

DWORD WINAPI ThreadProcedure(void *parameter)
{
    ThreadParameters *param = (ThreadParameters *)parameter;

    Job job;
    while (true)
    {
        if (param->queue->GetNextJob(&job, param->threadIdx))
        {
            ExecuteJob(&job);
        }
        else
        {
            // Wait on semaphore
            WaitForSingleObject(param->queue->semaphore, INFINITE);
        }
    }

    return 0;
}

#define THREADS_COUNT 3

inline void Vanilla(i32 samplePerPixel, i32 maxDepth, Canvas &canvas, Camera &camera, Scene &scene)
{
    for (int y = canvas.height - 1; y >= 0; y--)
    {
        for (int x = 0; x < canvas.width; x++)
        {
            Color c(0, 0, 0);

            for (int i = 0; i < samplePerPixel; i++)
            {
                f32 u = ((f32)x + RandomFloat()) / (f32)(canvas.width - 1);
                f32 v = ((f32)y + RandomFloat()) / (f32)(canvas.height - 1);

                Ray randomizedRay = camera.GetRay(u, v);
                c += GetRayColor(randomizedRay, scene, maxDepth);
            }
            canvas.SetPixel(x, y, c, samplePerPixel);
        }
    }
}

inline void Multithread(u32 samplePerPixel, u32 maxDepth, Canvas &canvas, Camera &camera, Scene &scene, u32 tileSize, JobsQueue &queue)
{
}

int main()
{
#if 0

    JobsQueue jobsQueue(32);

    HANDLE semaphore = CreateSemaphore(0, THREADS_COUNT, THREADS_COUNT, 0);
    jobsQueue.semaphore = semaphore;

    ThreadParameters params[THREADS_COUNT];
    for (i32 i = 0; i < THREADS_COUNT; ++i)
    {
        params[i].queue = &jobsQueue;
        params[i].threadIdx = i;
        CreateThread(0, 0, &ThreadProcedure, &params[i], 0, 0);
    }

    jobsQueue.PushJob("String A1");
    jobsQueue.PushJob("String A2");
    jobsQueue.PushJob("String A3");
    jobsQueue.PushJob("String A4");
    jobsQueue.PushJob("String A5");
    jobsQueue.PushJob("String A6");
    jobsQueue.PushJob("String A7");
    jobsQueue.PushJob("String A8");
    jobsQueue.PushJob("String A9");
    jobsQueue.PushJob("String A10");

    Sleep(2000);
    printf("\n");

    jobsQueue.PushJob("String B1");
    jobsQueue.PushJob("String B2");
    jobsQueue.PushJob("String B3");
    jobsQueue.PushJob("String B4");
    jobsQueue.PushJob("String B5");
    jobsQueue.PushJob("String B6");
    jobsQueue.PushJob("String B7");
    jobsQueue.PushJob("String B8");
    jobsQueue.PushJob("String B9");
    jobsQueue.PushJob("String B10");

    // NOTE: We use also the main thread do execute jobs along with the other threads
    Job job;
    while (jobsQueue.GetNextJob(&job, THREADS_COUNT + 1))
    {
        ExecuteJob(&job);
    }

    printf("\nFinished!\n");

    return 0;
#else
    srand((u32)time(NULL));

    u32 samplePerPixel = 1;
    u32 maxDepth = 1;

    Canvas canvas(1280, 720, 4);
    // Camera camera(p3(3,9,12), p3(0.5f,3.7f,0), v3(0,1,0), 55, canvas.ratio);
    Camera camera(p3(0, 5, 12), p3(1, 4, -1), v3(0, 1, 0), 50, canvas.ratio);

    // NOTE(mevex): Scene creation
    // NOTE(mevex): Materials
    Lambertian ground(Color(0.8f, 0.8f, 0.0f));
    Lambertian center(Color(0.7f, 0.3f, 0.3f));
    Metal left(Color(0.8f, 0.8f, 0.8f), 0.3f);
    Metal right(Color(0.05f, 0.6f, 0.73f), 0.0f);
    VertexColor tri(Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1));

    // NOTE(mevex): Objects
    Plane p1(p3(0, -0.5f, 0), v3(0, 1, 0), &ground); //-4.1139f
    Sphere s2(p3(-5, 1.5f, 1), 2.0f, &right);        //-3.6139f
    // Sphere s3(p3(-1,0,-1), 0.5f, &left);
    // Sphere s4(p3(1,0,-1), 0.5f, &right);
    // Triangle t5(p3(-1,1,-2), p3(1,1,-2), p3(0,2,-1), &tri);

    Mesh fox(p3(0, 3.65f, 0), p3(-0.8f, 0, 0), 7.4f); // r = 7.4f
    LoadObj(fox, "../models/fox2.obj", "../models/", true);

    // NOTE(mevex): Lights
    PointLight l1(p3(-0.5f, 10, 5), 0.7f);
    AmbientLight l2(0.3f);

    Scene scene;
    scene.Add(&p1);
    scene.Add(&s2);
    // scene.Add(&s3);
    // scene.Add(&s4);
    scene.Add(&fox);
    scene.Add(&l1);
    scene.Add(&l2);

    printf("--- Rendering starts ---\n");
    printf("Samples per pixel: %d Max depth: %d\n", samplePerPixel, maxDepth);
    auto timerStart = std::chrono::high_resolution_clock::now();
    auto cyclesStart = __rdtsc();

    Vanilla(samplePerPixel, maxDepth, canvas, camera, scene);

    auto timerFinish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerFinish - timerStart);
    auto avgCount = std::chrono::duration_cast<std::chrono::nanoseconds>(timerFinish - timerStart).count() / (canvas.width * canvas.height);

    // NOTE(mevex): Pixel order: AABBGGRR
    auto res = stbi_write_png("../renders/render.png", canvas.width, canvas.height, canvas.bytesPerPixel, canvas.memory, 0);

    printf("\nRendering time: %ims\n", (int)(duration.count()));
    printf("Average pixel time: %ins\n", (int)avgCount);

    // printf("GetRay Count:    %llu,  AVG Cycles: %llu \n", GetRayColorCounter, GetRayColorCycles / GetRayColorCounter);
    // printf("Hit Count:    %llu,  AVG Cycles: %llu \n", HitCounter, HitCycles / HitCounter);
    // printf("Scatter Count:    %llu,  AVG Cycles: %llu \n", ScatterCounter, ScatterCycles / ScatterCounter);

    getchar();
    return 0;
#endif
}
