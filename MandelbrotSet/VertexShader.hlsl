struct vs_in
{
	float3 positionLocal : POSITION;
};

float4 main(vs_in input) : SV_POSITION
{
	return float4(input.positionLocal, 1.0);
}