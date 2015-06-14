texture2D s_Texture;
SamplerState s_Sampler;

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : UV;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return s_Texture.Sample(s_Sampler, input.uv);
}