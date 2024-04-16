#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_Texture;

vector	g_vCamPosition;

struct VS_IN
{
	float3				vPosition : POSITION;
	row_major float4x4	TransformMatrix : WORLD;
	bool				isLived : COLOR0;
};


struct VS_OUT
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
	bool		isLived : COLOR0;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	vector		vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);	

	Out.vPosition = mul(vPosition, g_WorldMatrix);
	Out.vPSize = float2(In.TransformMatrix._11, In.TransformMatrix._22);
	Out.isLived = In.isLived;

	return Out;
}

struct GS_IN
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
	bool		isLived : COLOR0;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	bool		isLived : COLOR0;
};

/* 정점을 생성한다. */
[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
	GS_OUT		Out[4];

	float3		vLook = normalize((g_vCamPosition - In[0].vPosition).xyz);
	float3		vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook.xyz)) * In[0].vPSize.x * 0.5f;
	float3		vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;

	matrix		matVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = vector(In[0].vPosition.xyz + vRight + vUp, 1.f);
	Out[0].vTexcoord = float2(0.0f, 0.0f);
	Out[0].vPosition = mul(Out[0].vPosition, matVP);
	Out[0].isLived = In[0].isLived;

	Out[1].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
	Out[1].vTexcoord = float2(1.0f, 0.0f);
	Out[1].vPosition = mul(Out[1].vPosition, matVP);
	Out[1].isLived = In[0].isLived;

	Out[2].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
	Out[2].vTexcoord = float2(1.0f, 1.0f);
	Out[2].vPosition = mul(Out[2].vPosition, matVP);
	Out[2].isLived = In[0].isLived;

	Out[3].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
	Out[3].vTexcoord = float2(0.0f, 1.0f);
	Out[3].vPosition = mul(Out[3].vPosition, matVP);
	Out[3].isLived = In[0].isLived;

	Vertices.Append(Out[0]);
	Vertices.Append(Out[1]);
	Vertices.Append(Out[2]);
	Vertices.RestartStrip();

	Vertices.Append(Out[0]);
	Vertices.Append(Out[2]);
	Vertices.Append(Out[3]);
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	bool		isLived : COLOR0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	if (Out.vColor.a <= 0.3f || 
		false == In.isLived)
		discard;

	Out.vColor = vector(1.f, 0.f, 0.f, 1.f);
	
	return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}