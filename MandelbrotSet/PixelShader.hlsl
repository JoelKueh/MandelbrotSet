Texture2D<unorm float4> tex : register (t0);

unorm float4 main(float4 pixelPos : SV_Position) : SV_TARGET
{
	int texScale = 4;
	float4 output;
	for (int row = 0; row < texScale; row++)
	{
		for (int col = 0; col < texScale; col++)
		{
			int3 coords = { pixelPos.x * texScale + row, pixelPos.y * texScale + col, 0 };
			output += tex.Load(coords);
		}
	}
	output /= texScale * texScale;
	return output;
}