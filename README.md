# ProceduralPollock

ProceduralPollock is an open source C++ project that generates abstract, math-based art using procedurally generated pixel shaders.

The program creates a window using the Win32 API and DirectX 11, then uses a 64-bit seed (usually from the system time, but can be set manually) to procedurally generate a pixel shader.

Press `spacebar` to generate a new shader. There might be a bit of lag when generating a new shader, depending on shader complexity (which is random).

The generated images look like abstract expressionistic art, somewhat resembling to the style of Jackson Pollock, hence the name of the project.

This project uses a custom PRNG (see `src/RandFS.h`) to deterministically produce a function that receives as input the X and Y coordinates of each pixel and the time, and outputs an RGB value for that pixel. From the given seed, it uses different techniques to procedurally compose primitive mathematical formulas to generate a single output function for a pixel shader, that is then compiled and displayed on the window.

This technique was originally proposed in the paper [Hash Visualization: a New Technique to improve Real-World Security](https://users.ece.cmu.edu/~adrian/projects/validation/validation.pdf) by Adrian Perrig and Dawn Song. This project is just one possible implementation of the general idea outlined in the paper.

By default, the program uses time as an input to generate animated images. The time value always pass through sine and cosine functions, making the animation loop perfectly. To generate only static images (no animation), open `src/Shader.cpp` and comment out the line `#define ANIMATE`.

Also, please check out other versions of this project:

- PerpetualPollock: infinite stream of animated abstract images, never repeating (Coming soon!)
- ProceduralPollockWeb: browser version implemented with WebGPU (Coming soon!)
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
- Exporting animated images as video files and static images as PNG files

Feel free to contribute or suggest any new features!

Questions and feedback are welcome. Please send them to my email [diego.quintanilha@hotmail.com](mailto:diego.quintanilha@hotmail.com).

