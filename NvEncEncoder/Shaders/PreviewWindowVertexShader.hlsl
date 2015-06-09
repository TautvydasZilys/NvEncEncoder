cbuffer MatrixBuffer
{
	float2 scale;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

VertexToPixel main(float2 pos : POSITION)
{
	VertexToPixel output;

	output.position = float4(pos.x * scale.x, pos.y * scale.y, 0.5f, 1.0f);
	output.uv = (pos + float2(1.0f, 1.0f)) / 2.0f;

	return output;
}