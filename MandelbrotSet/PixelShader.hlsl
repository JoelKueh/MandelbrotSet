cbuffer Inputs : register (b0)
{
	float centerX;
	float centerY;
	float width;
	float windWidth;
	float windHeight;
	float ssLvl;
};

Texture2D<unorm float4> tex : register (t0);

unorm float4 main(float4 pixelPos : SV_Position) : SV_TARGET
{
	float4 output;
	for (int row = 0; row < ssLvl; row++)
	{
		for (int col = 0; col < ssLvl; col++)
		{
			int3 coords = { pixelPos.x * ssLvl + row, pixelPos.y * ssLvl + col, 0 };
			output += tex.Load(coords);
		}
	}
	output /= ssLvl * ssLvl;
	return output;
}