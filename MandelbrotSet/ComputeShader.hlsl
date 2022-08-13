cbuffer Inputs : register (b0)
{
	double topLeftX;
	double topLeftY;
	double width;
	double height;
	double windWidth;
	double windHeight;
};

Texture2D tex : register(u0)

[numthreads(1, 1, 1)]

double2 cmplxSquare(double2 input)
{
	double2 output;
	output.x = input.x * input.x - input.y * input.y;
	output.y = 2 * input.x * input.y;
	return output;
}

void main( uint3 DTid : SV_DispatchThreadID )
{
	double2 c;
	c.x = topLeftX + (double)(width * DTid.x) / windWidth;
	c.y = topLeftY + (double)(height * DTid.y) / windHeight;

	double2 z = c;

	for (int i = 0; i < 1000; i++)
	{
		z = cmplxSquare(z) + c;

		if (length(z) > 2)
		{
			tex[DTid.x, DTid.y] = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	tex[DTid.x, DTid.y] = float4(0.0f, 0.0f, 0.0f, 1.0f);
}