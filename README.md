# ProceduralPollock

ProceduralPollock is an open source C++ project that generates abstract, math-based art using procedurally generated pixel shaders.

The generated images look like abstract expressionistic art that somewhat resembles the style of Jackson Pollock, hence the name of the project.

This project is one variation in a family of projects that implement similar techniques. Check out the [other versions] of this project to see different approaches and results.

## Build

To build the project for Visual Studio, from the root folder, just run:

```
build\premake5 vs2022
```

The Premake executable is already included in the repository.

Alternatively, you can use any C++ compiler and compile the project directly from the command line. From the project root, do:

```
mkdir bin
g++ -std=c++20 -O3 -D_RELEASE -DUNICODE src/Graphics.cpp src/Shader.cpp src/main.cpp -ld3d11 -ld3dcompiler -o bin\ProceduralPollock.exe
```

You can also use `clang++` or any other C++ compiler.

This will generate an executable file inside the `bin` folder. If you want maximum performance, you should compile using the `-ffast-math -flto -fwhole-program` flags. Also, feel free to use `-D_DEBUG` or `-D_PROFILE` instead of `-D_RELEASE` in case you want to try those other build configurations.

The only external dependencies of this project are C++20 or newer, the standard C++ library, the Win32 API and the DirectX 11 API.

## How it works

The program creates a window using the Win32 API and DirectX 11, then uses a 64-bit seed (usually from the system time, but can be set manually) to procedurally generate a pixel shader. The generation process is deterministic, i.e. the same seed will always generate the same shader. Press `spacebar` to generate a new shader. Depending on the (random) shader complexity, there will be a slight delay during generation.

The project uses a custom PRNG (see `src/RandFS.h`) to procedurally generate a function that receives as input the X and Y coordinates of each pixel, along with the time, and outputs an RGB value for that pixel. From the given seed, it uses different techniques to compose primitive mathematical formulas and generate a single function. This function is then incorporated into a pixel shader, compiled, and rendered on the window.

The technique that this project is based on was originally proposed in the paper [Hash Visualization: a New Technique to improve Real-World Security](https://users.ece.cmu.edu/~adrian/projects/validation/validation.pdf) by Adrian Perrig and Dawn Song. This project is just one possible implementation of the general idea outlined in the paper.

By default, the program uses time as an input to generate animated images. The time value always pass through sine and cosine functions, making the animation loop perfectly. To generate only static images (no animation), open `src/Shader.cpp` and comment out the line `#define ANIMATE`.

In **Profile** and **Release** builds, you can also control the trade-off between shader compilation time and runtime performance by setting the `SHADER_OPTIMIZATION_LEVEL` macro in `src/Graphics.cpp`. Values must range from 0 to 4. In **Debug** builds, this macro always defaults to 0.

`SHADER_OPTIMIZATION_LEVEL 0` provides the fastest compile time, but the worst runtime performance (may reduce FPS), while `SHADER_OPTIMIZATION_LEVEL 4` provides the slowest compile time, but highest runtime performance (maximized FPS). Values 1, 2 and 3 provide intermediate trade-offs between compilation speed and runtime optimization.

Be aware that, on higher optimization levels, pressing the spacebar to generate a new shader may cause noticeable stutters (several seconds) because of the longer shader compilation times.

**Warning**: This program can sometimes produce rapidly changing and flashing colors that may trigger seizures in individuals with photosensitive epilepsy.

## Other versions

Check out the other versions of this project:

- [PerpetualPollock](https://github.com/diegoquintanilha/PerpetualPollock): infinite stream of continuously animated abstract images, never repeating.
- [ProceduralPollockWeb](https://github.com/diegoquintanilha/ProceduralPollockWeb): browser version of ProceduralPollock implemented in WASM, using WebGPU and WebGL.
- PerpetualPollockWeb: browser version of PerpetualPollock implemented in WASM, using WebGPU and WebGL (Coming soon!).

## Future features

Features to be implemented:

- Button to generate new shader
- Custom seed input at runtime
- Separate buttons for generating animated and static images
- Exporting animated images as video files/GIFs, and static images as PNG files

I consider this project finished for now, but in the future I might come back to it and implement some of these. Feel free to contribute or suggest any new features!

Questions and feedback are welcome. Please send them to my email [diego.quintanilha@hotmail.com](mailto:diego.quintanilha@hotmail.com).

