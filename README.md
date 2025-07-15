# ProceduralPollock

ProceduralPollock is an open source C++ project that generates abstract, math-based art using procedurally generated pixel shaders.

The generated images look like abstract expressionistic art, somewhat resembling to the style of Jackson Pollock, hence the name of the project.

## How it works

The program creates a window using the Win32 API and DirectX 11, then uses a 64-bit seed (usually from the system time, but can be set manually) to procedurally generate a pixel shader. The generation process is deterministic, i.e. the same seed will always generate the same shader.

Press `spacebar` to generate a new shader. Depending on the (random) shader complexity, there migth be a slight delay during generation.

This project uses a custom PRNG (see `src/RandFS.h`) to procedurally generate a function that receives as input the X and Y coordinates of each pixel, along with the time, and outputs an RGB value for that pixel. From the given seed, it uses different techniques to compose primitive mathematical formulas and generate a single function. This function is then incorporated into a pixel shader, compiled, and rendered on the window.

The technique that this project is based on was originally proposed in the paper [Hash Visualization: a New Technique to improve Real-World Security](https://users.ece.cmu.edu/~adrian/projects/validation/validation.pdf) by Adrian Perrig and Dawn Song. This project is just one possible implementation of the general idea outlined in the paper.

By default, the program uses time as an input to generate animated images. The time value always pass through sine and cosine functions, making the animation loop perfectly. To generate only static images (no animation), open `src/Shader.cpp` and comment out the line `#define ANIMATE`.

Also, please check out other versions of this project:

- PerpetualPollock: infinite stream of animated abstract images, never repeating (Coming soon!)
- [ProceduralPollockWeb](https://github.com/diegoquintanilha/ProceduralPollockWeb): browser version implemented with WebGPU/WebGL
- PerpetualPollockWeb: browser version of PerpetualPollock (Coming soon!)

## Build

To build the project for Visual Studio with Premake, run `premake5 vs2022` from the root folder.

Alternatively, you can use any C++ compiler and compile the files located in the `src` folder directly. The only external dependencies of this project are the Win32 API, the DirectX 11 API and the standard library.

Support for CMake will be added in the future.

## Future features

Features to be implemented:

- Button to generate new shader
- Custom seed input at runtime
- Separate buttons for generating animated and static images
- Exporting animated images as video files/GIFs and static images as PNG files

Feel free to contribute or suggest any new features!

Questions and feedback are welcome. Please send them to my email [diego.quintanilha@hotmail.com](mailto:diego.quintanilha@hotmail.com).
