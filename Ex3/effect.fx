
RasterizerState CullNone
{
	CullMode = None;
};

DepthStencilState DepthDefault
{
};

BlendState BlendDisable
{
};



cbuffer cb
{
	float4x4 g_worldViewProj;
}



struct PSIn
{
	float4 pos   : SV_Position;
	float4 color : COLOR;
};


void vsTriangle(uint vertexID : SV_VertexID, out PSIn output)
{
	switch(vertexID)
	{
		case 0:
			output.pos = float4(-0.5, -0.5, 0.0, 1.0);
			output.color = float4(1.0, 0.0, 0.0, 1.0);
			break;
		case 1:
			output.pos = float4(0.0, 0.5, 0.0, 1.0);
			output.color = float4(0.0, 1.0, 0.0, 1.0);
			break;
		default:
			output.pos = float4(0.5, -0.5, 0.0, 1.0);
			output.color = float4(0.0, 0.0, 1.0, 1.0);
			break;
	}
	output.pos = mul(output.pos, g_worldViewProj);
}

float4 psSimple(PSIn input) : SV_Target
{
	return input.color;
}



technique11 Simple
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, vsTriangle()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, psSimple()));
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthDefault, 0);
		SetBlendState(BlendDisable, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
	}
}
