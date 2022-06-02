double2 cmplxSquare(double2 input)
{
	double2 output;
	output.x = input.x * input.x - input.y * input.y;
	output.y = 2 * input.x * input.y;
	return output;
}

float4 main(float4 pixelPos : SV_Position) : SV_TARGET
{
	double2 c;
	c.x = (double)(pixelPos.x - 960.5) / 480.25;
	c.y = (double)(pixelPos.y - 540.5) / -480.25;
	
	double2 z = c;

	for (int i = 0; i < 1000; i++)
	{
		z = cmplxSquare(z) + c;

		if (length(z) > 2)
		{
			return float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}