# Ray Tracer
This is a simple raytracer I made as a project for my BSc thesis about 3D rendering. This project will never implement hardware (GPU) rendering, but will keep evolving for a while. If you are curious about it's future see the section below.

## Updates
Since this project has served it's purpose of giving a rough idea of the tecniques and constraints of raytracing (pathtracing), the next step is to use it as a sample in order to improve my low level optimization skills.
There are three main topics that I want to get confortable with:
- **Multithreading:** The easier next step is to make it multithreaded.
- **SIMD instructions:** Once the code is multithreaded, I will implement the algorithms in AVX2.
- **Assembly:** As a consequence of the previous two, I would like to get comfortable with reading and understanding disassembled code

## External resources
Below there are listed all the books and additional libraries I used to build the ray tracer
- [Ray Tracing in One Weekend - The Book Series](https://raytracing.github.io/)
- [Computer Graphics from Scratch, Gabriel Gambetta](https://gabrielgambetta.com/computer-graphics-from-scratch/)
- [Scratchpixel](https://www.scratchapixel.com/index.php)
- [Sean Barrett's STB library for image handling](https://github.com/nothings/stb)
- [Tiny OBJ Loader](https://github.com/tinyobjloader/tinyobjloader)

## Build instructions
Just run build.bat in an environment where MSVC is configured. This will create a build directory with the executable.

## Folder structure
This is how the structure of the project should be. This is important because the executable uses hardcoded local paths for locating the 3D models (.obj) and to place the rendered image.

|Folders|Info|
|--|--|
|build  |Executable and build resulting files|
|code   |Source code location. Surpisingly...|
|renders|Rendered image "render.png"|
|models |Available 3D models|
