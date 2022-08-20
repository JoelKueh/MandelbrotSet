cbuffer Inputs : register (b0)
{
	float centerX;
	float centerY;
	float width;
	float windWidth;
	float windHeight;
};

RWTexture2D<unorm float4> tex : register(u0);

double2 cmplxSquare(double2 input)
{
	double2 output;
	output.x = input.x * input.x - input.y * input.y;
	output.y = 2 * input.x * input.y;
	return output;
}

float4 colorizer(int iterations)
{
	iterations = iterations % 16;
	switch (iterations)
	{
	case 0: return float4(0.1f, 0.03f, 0.1f, 1.0f);
	case 1: return float4(0.26f, 0.12f, 0.06f, 1.0f);
	case 2: return float4(0.42f, 0.2f, 0.01f, 1.0f);
	case 3: return float4(0.6f, 0.34f, 0.0f, 1.0f);
	case 4: return float4(0.8f, 0.5f, 0.0f, 1.0f);
	case 5: return float4(1.0f, 0.67f, 0.0f, 1.0f);
	case 6: return float4(0.97f, 0.79f, 0.37f, 1.0f);
	case 7: return float4(0.95f, 0.91f, 0.75f, 1.0f);
	case 8: return float4(0.83f, 0.93f, 0.97f, 1.0f);
	case 9: return float4(0.53f, 0.71f, 0.9f, 1.0f);
	case 11: return float4(0.22f, 0.49f, 0.82f, 1.0f);
	case 12: return float4(0.09f, 0.32f, 0.69f, 1.0f);
	case 13: return float4(0.05f, 0.17f, 0.54f, 1.0f);
	case 14: return float4(0.0f, 0.03f, 0.39f, 1.0f);
	case 15: return float4(0.02f, 0.02f, 0.29f, 1.0f);
	case 16: return float4(0.04f, 0.0f, 0.18f, 1.0f);
	}
	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

[numthreads(4, 4, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	double2 c;
	double texWidth = windWidth * 4;
	double texHeight = windHeight * 4;

	float scaleFactor = width / texWidth;
	float halfwayPixelW = texWidth / 2;
	float halfwayPixelH = texHeight / 2;

	c.x = (double)((DTid.x - halfwayPixelW) * scaleFactor) + centerX;
	c.y = (double)((DTid.y - halfwayPixelH) * scaleFactor) + centerY;

	double2 z = c;

	for (int i = 0; i < 1000; i++)
	{
		z = cmplxSquare(z) + c;

		if (length(z) > 2.0f)
		{
			tex[DTid.xy] = colorizer(i);
			return;
		}
	}
	tex[DTid.xy] = float4(0.0, 0.0, 0.0, 1.0);
}