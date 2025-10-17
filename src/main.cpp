#ifndef _WIN32
#error ProceduralPollock is currently only supported on Windows.
#endif

#include <iostream>
#include <chrono>
#include <cmath>

#include "Shader.h"
#include "Graphics.h"

int main()
{
	// Get time at the beginning of the program to use as an initial seed
	auto now = std::chrono::high_resolution_clock::now();
	uint64_t timeStart = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();

	// Generate the first shader using time as seed
	uint64_t currentSeed = timeStart;
	std::string pixelShader = GenerateShaderCode(currentSeed);
	
	// Create window and initialize graphics API
	Graphics graphics(1600, 900, pixelShader.c_str(), int(pixelShader.length()));
	
	bool pressedKey = false;

	while (graphics.UpdateInputs())
	{
		// Get current time
		auto now = std::chrono::high_resolution_clock::now();
		uint64_t currentTime = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();
		const float elapsedTime = float((currentTime - timeStart) * 0.000001);

		// Create new shader if spacebar has been pressed
		if (GetAsyncKeyState(' '))
		{
			if (!pressedKey)
			{
				// Generate, compile and bind a new pixel shader
				// Complex shaders might take a few seconds to compile
				currentSeed = currentTime;
				std::string newShader = GenerateShaderCode(currentSeed);
				graphics.CreatePixelShader(newShader.c_str(), int(newShader.length()));
			}
			pressedKey = true;
		}
		else pressedKey = false;

		// Calculate sin and cos of time to pass as constant buffers to the shader
		float sinTime = 0.5f + 0.5f * std::sinf(0.5f * elapsedTime);
		float cosTime = 0.5f + 0.5f * std::cosf(0.5f * elapsedTime);

		// Run shader and swap buffers
		graphics.UpdateConstantBuffer(sinTime, cosTime, 0.0f, 0.0f);
		graphics.DrawViewportQuad();
		graphics.SwapBuffers();
	}

	return 0;
}

