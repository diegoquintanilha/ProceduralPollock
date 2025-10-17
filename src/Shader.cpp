#include "Shader.h"

#include <iostream>

#define RANDFS_IMPLEMENTATION
#include "RandFS.h"

// Comment the line below to generate static images
#define ANIMATE

std::string GenerateShaderCode(uint64_t seed)
{

	seed = Hash::UInt64(seed);

	// Uncomment here to set a specific seed
//	seed = 420ULL;

	Random rand(seed);

	// Depths between 6 and 12 tend to generate interesting images
	// Add two random values to bias towards the middle (9)
	int maxDepth = rand.IntBetween(3, 7) + rand.IntBetween(3, 7);
	
	// This generates very interesting images, but some computers can't handle it
	// Uncomment at your own risk
	//maxDepth = 12;

	#pragma region Function definitions

	static constexpr char functionDefinitions[] =
	R"(
	
	// 1 input
	
	float fInv(float x)
	{
		return 1.0f - x;
	}
	
	float fSqr(float x)
	{
		return x * x;
	}
	
	float fSqrt(float x)
	{
		return sqrt(x);
	}
	
	float fSmooth(float x)
	{
		float x2 = x * x;
		float x3 = x2 * x;
		return x2 + x2 + x2 - x3 - x3;
	}

	float fSharp(float x)
	{
		return x * (x * (x + x - 3.0f) + 2.0f);
	}
	
	// -------------------------------------
	// 2 inputs
	
	float fAdd(float x, float y)
	{
		float res = x + y;
		if (res > 1.0f)
			return 2.0f - res;
		return res;
	}
	
	float fSub(float x, float y)
	{
		float res = x - y;
		if (res < 0.0f)
			return -res;
		return res;
	}
	
	float fMul(float x, float y)
	{
		return x * y;
	}
		
	float fDiv(float x, float y)
	{
		float min = x, max = y;
		if (x > y)
		{
			min = y;
			max = x;
		}
		if (max < 0.0001f)
			max = 0.0001f;
		return min / max;
	}
	
	float fAvg(float x, float y)
	{
		return (x + y) * 0.5f;
	}
	
	float fGeom(float x, float y)
	{
		return sqrt(x * y);
	}
	
	float fHarm(float x, float y)
	{
		float den = x + y;
		if (den < 0.0001f)
			den = 0.0001f;
		return (2.0f * x * y) / den;
	}

	float fHypo(float x, float y)
	{
		return 0.70710678f * sqrt(x * x + y * y); // Scale by 1 / sqrt(2)
	}	
	
	float fMin(float x, float y)
	{
		return x < y ? x : y;
	}

	float fMax(float x, float y)
	{
		return x > y ? x : y;
	}
	
	float fPow(float x, float y)
	{
		if (x < 0.01f)
			x = 0.01f;
		if (x > 0.99f)
			x = 0.99f;
		float exp = exp2(4.0f * y - 2.0f);
		return pow(x, exp);
	}

	float fBell(float x, float y)
	{
		if (x < 0.01f)
			x = 0.01f;
		if (x > 0.99f)
			x = 0.99f;
		float y2 = y * y;
		return pow(4.0f * x * (1.0f - x), 20.0f * y2 * y2 + 0.3f);
	}
	
	float fWave(float x, float y)
	{
		const float MAX_FREQUENCY = 6.0f * 3.1415927f;
		return 0.5f + 0.5f * cos(MAX_FREQUENCY * x * y);
	}
	
	float fWaveDamp(float x, float y)
	{
		const float FREQUENCY_FACTOR = 3.0f * 3.1415927f;
		const float SHIFT_FACTOR = 1.0f / 6.0f;
		float osc = ldexp(cos(FREQUENCY_FACTOR * x * (y + SHIFT_FACTOR)), -x * x);
		return osc * osc;
	}

	// -------------------------------------
	// 3 inputs
	
	float fLerp(float x, float y, float z)
	{
		return (1.0f - z) * x + z * y;
	}
	
	float fSmoothLerp(float x, float y, float z)
	{
		float z2 = z * z;
		float z3 = z2 * z;
		float smooth = z2 + z2 + z2 - z3 - z3;
		return smooth * (y - x) + x;
	}
	
	float fMlerp(float x, float y, float z)
	{
		if (x < 0.0001f)
			x = 0.0001f;
		if (y < 0.0001f)
			y = 0.0001f;
		return x * pow(y / x, z);
	}
	
	float fClamp(float x, float y, float z)
	{
		float min = x, max = y;
		if (x > y)
		{
			min = y;
			max = x;
		}
		if (z < min)
			return min;
		else if (z > max)
			return max;
		return z;
	}
	
	// -------------------------------------
	// 4 inputs
		
	float fDist(float x, float y, float z, float w)
	{
		float dx = x - z;
		float dy = y - w;
		return 0.70710678f * sqrt(dx * dx + dy * dy); // Scale by 1 / sqrt(2)
	}
	
	float fDistLine(float x, float y, float z, float w)
	{
		if (z < 0.499f)
		{
			float m = tan(z * 3.1415927f);
			float n = (1.0f - w) * (1.0f + m) - m;
			float c = (x + y * m - m * n) / (m * m + 1.0f);
			float dx = c - x;
			float dy = m * c + n - y;
			return 0.70710678f * sqrt(dx * dx + dy * dy);
		}
		else if (z > 0.501f)
		{
			float m = tan(z * 3.1415927f);
			float n = w - m * w;
			float c = (x + y * m - m * n) / (m * m + 1.0f);
			float dx = c - x;
			float dy = m * c + n - y;
			return 0.70710678f * sqrt(dx * dx + dy * dy);
		}
		else
		{
			return 0.70710678f * abs(w - x);
		}
	}

	// -------------------------------------
	// Masks

	float3 fInv3(float3 v)
	{
		return float3(1.0f, 1.0f, 1.0f) - v;
	}

	float3 fAdd3(float3 v, float x)
	{
		float3 res = v + float3(x, x, x);
		return lerp(res, 2.0f - res, step(1.0f, res));
	}
	
	float3 fSub3(float3 v, float x)
	{
		float3 res = v - float3(x, x, x);
		return lerp(-res, res, step(0.0f, res));
	}

	)";

	#pragma endregion

	#pragma region Main function

	std::string mainFunction =
	R"(

	cbuffer ConstantBuffer
	{
		float4 buf;
	};

	float4 main(float2 uv : TEXCOORD) : SV_TARGET
	{
		float invX = 1.0f - uv.x;
		float invY = 1.0f - uv.y;
		float sinTime = buf.x;
		float cosTime = buf.y;

		float3 rgb = float3(@, @, @);
		rgb = @MASK@;

		return float4(rgb, 1.0f);
	}
	
	)";

	#pragma endregion

	static const char* values[] =
	{
		"uv.x", // Normalized x coordinate
		"uv.y", // Normalized y coordinate
		"invX", // 1.0f - uv.x
		"invY", // 1.0f - uv.y
#ifdef ANIMATE
		"sinTime", // sin(time)
		"cosTime", // cos(time)
#endif
		"#", // Random constant
		"#" // Double the chance
	};
	const int valuesSize = sizeof(values) / sizeof(const char*);
	
	static const char* functions[] =
	{
		"fInv(@)",
		"fSqr(@)",
		"fSqrt(@)",
		"fSmooth(@)",
		"fSharp(@)",
		"fAdd(@, @)",
		"fSub(@, @)",
		"fMul(@, @)",
		"fInv(fMul(@, @))", // Compensate for bias
		"fDiv(@, @)",
		"fAvg(@, @)",
		"fGeom(@, @)",
		"fHarm(@, @)",
		"fHypo(@, @)",
		"fMin(@, @)",
		"fMax(@, @)",
		"fPow(@, @)",
		"fBell(@, @)",
		"fInv(fBell(@, @))", // Compensate for bias
		"fWave(@, @)",
		"fWave(@, @)", // Double the chance
		"fWaveDamp(@, @)",
		"fInv(fWaveDamp(@, @))",
		"fLerp(@, @, @)",
		"fSmoothLerp(@, @, @)",
		"fMlerp(@, @, @)",
//		"fClamp(@, @, @)", // This generates ugly discontinuities
		"fDist(@, @, @, @)", // Compare variables to variables
		"fDist(@, @, #, #)", // Compare variables to fixed point
		"fDist(uv.x, uv.y, @, @)", // Compare pixel coords to variables
		"fDist(uv.x, uv.y, #, #)", // Compare pixel coords to fixed point
		"fInv(fDist(@, @, @, @))", // Compensate for bias
		"fInv(fDist(@, @, #, #))", // Compensate for bias
		"fInv(fDist(uv.x, uv.y, @, @))", // Compensate for bias
		"fInv(fDist(uv.x, uv.y, #, #))", // Compensate for bias
		"fDistLine(@, @, @, @)", // Compare variables to variables
		"fDistLine(@, @, #, #)", // Compare variables to fixed line
		"fDistLine(uv.x, uv.y, @, @)", // Compare pixel coords to variable line
		"fDistLine(uv.x, uv.y, #, #)", // Compare pixel coords to fixed line
		"fInv(fDistLine(@, @, @, @))", // Compensate for bias
		"fInv(fDistLine(@, @, #, #))", // Compensate for bias
		"fInv(fDistLine(uv.x, uv.y, @, @))", // Compensate for bias
		"fInv(fDistLine(uv.x, uv.y, #, #))" // Compensate for bias
	};
	const int functionsSize = sizeof(functions) / sizeof(const char*);

	static const char* masks[] =
	{
		"rgb",
		"rgb", // Repeat to increase the chance of no mask
		"rgb", // Repeat to increase the chance of no mask
		"fAdd3(rgb, @)",
		"fSub3(rgb, @)",
		"fAdd3(fSub3(rgb, @), @)",
		"fSub3(fAdd3(rgb, @), @)",
		"fInv3(fAdd3(rgb, @))",
		"fInv3(fSub3(rgb, @))",
		"fInv3(fAdd3(fSub3(rgb, @), @))",
		"fInv3(fSub3(fAdd3(rgb, @), @))"
	};
	const int masksSize = sizeof(masks) / sizeof(const char*);

	// Replace mask token for one of the masks selected randomly
	std::string maskToken("@MASK@");
	size_t maskPos = mainFunction.find(maskToken);
	const char* mask = rand.Element(masks, masksSize);
	mainFunction.replace(maskPos, maskToken.length(), mask);

	// Run until maxDepth because at maxDepth all tokens must be replaced by constants
	for (int i = 0; i <= maxDepth; i++)
	{
		// Find all '@' tokens and replace for '$' tokens
		// This marks all tokens for replacement in this iteration
		for (char& c : mainFunction)
		{
			if (c == '@')
				c = '$';
		}

		// Replace all '$' tokens for either a function or a value
		size_t pos = mainFunction.find('$');
		while (pos != std::string::npos)
		{
			// Decide whether to replace the token with a function or a fixed value
			// At depth 0, it is guaranteed to use a function, and at MAX_DEPTH it is guaranteed to use a fixed value
			// The progression is quadratic, which makes it more likely to choose functions over values than if the chance progressed linearly
			std::string replacement = rand.IntBetween(1, maxDepth * maxDepth) > i * i
				? rand.Element(functions, functionsSize)
				: rand.Element(values, valuesSize);

			mainFunction.replace(pos, 1, replacement);
			pos = mainFunction.find('$');
		}
	}

	// Replace '#' tokens with random constants
	size_t pos = mainFunction.find('#');
	while (pos != std::string::npos)
	{
		mainFunction.replace(pos, 1, std::to_string(rand.FloatO()) + 'f');
		pos = mainFunction.find('#');
	}

//	std::cout << mainFunction << std::endl;
	std::cout << "Shader seed: " << seed << std::endl;

	return functionDefinitions + mainFunction;
}

