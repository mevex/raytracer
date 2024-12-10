# Ray Tracer
This is a simple raytracer I made as a project for my BSc thesis about 3D rendering. This project will never implement hardware (GPU) rendering, but will keep evolving for a while. If you are curious about it's future see the section below.

## Updates
Since this project has served it's purpose of giving a rough idea of the tecniques and constraints of raytracing (pathtracing), the next step is to use it as a sample in order to improve my low level optimization skills.
There are three main topics that I want to get confortable with and those are:
- **SIMD instructions:** SSE in this case
- **Assembly:** getting comfortable reading and understanding disassembled code
- **Multithreading:** once the code is fully optimized using SIMD instructions, the next step is to make it multithreaded.

## External resources
Below there are listed all the books and additional libraries I used to build the ray tracer
- [Ray Tracing in One Weekend - The Book Series](https://raytracing.github.io/)
- [Computer Graphics from Scratch, Gabriel Gambetta](https://gabrielgambetta.com/computer-graphics-from-scratch/)
- [Scratchpixel](https://www.scratchapixel.com/index.php)
- [Sean Barrett's STB libraries for image handling](https://github.com/nothings/stb)
- [Tiny OBJ Loader](https://github.com/tinyobjloader/tinyobjloader)

## Folder structure
This is how I organize my folders for the project 
|Folders|Info|
|--|--|
|build  |all the build files will be created here|
|code   |the source code (a.k.a. this repo) will be located here|
|renders|the rendered images will be created here named "render.png"|
|models |all the available models will be stored here|
