#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_Texture;
texture2D	g_NoiseTexture, g_NoiseMaskTexture;

vector	g_vCamPosition;

struct VS_IN
{
	float3				vPosition : POSITION;
	row_major float4x4	TransformMatrix : WORLD;
	float				fLifeRatio : COLOR0;
    uint				iSpriteIndexOffset : COLOR1;
};

struct VS_OUT
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
    float		fLifeRatio : COLOR0;
    uint		iSpriteIndexOffset : COLOR1;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	vector		vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);	

	Out.vPosition = mul(vPosition, g_WorldMatrix);
	Out.vPSize = float2(In.TransformMatrix._11, In.TransformMatrix._22);
    Out.fLifeRatio = In.fLifeRatio;
    Out.iSpriteIndexOffset = In.iSpriteIndexOffset;
	
	return Out;
}

struct GS_IN
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
    float		fLifeRatio : COLOR0;
    uint		iSpriteIndexOffset : COLOR1;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
    float		fLifeRatio : COLOR0;
    uint		iSpriteIndexOffset : COLOR1;
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
    Out[0].fLifeRatio = In[0].fLifeRatio;
    Out[0].iSpriteIndexOffset = In[0].iSpriteIndexOffset;

	Out[1].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
	Out[1].vTexcoord = float2(1.0f, 0.0f);
	Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].fLifeRatio = In[0].fLifeRatio;
    Out[1].iSpriteIndexOffset = In[0].iSpriteIndexOffset;


	Out[2].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
	Out[2].vTexcoord = float2(1.0f, 1.0f);
	Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].fLifeRatio = In[0].fLifeRatio;
    Out[2].iSpriteIndexOffset = In[0].iSpriteIndexOffset;


	Out[3].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
	Out[3].vTexcoord = float2(0.0f, 1.0f);
	Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].fLifeRatio = In[0].fLifeRatio;
    Out[3].iSpriteIndexOffset = In[0].iSpriteIndexOffset;


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
    float		fLifeRatio : COLOR0;
    uint		iSpriteIndexOffset : COLOR1;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	
    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

    if (Out.vColor.a <= 0.0f || 1.f <= In.fLifeRatio)
		discard;

    Out.vColor.rgb = float3(1.f, 1.f, 1.f);
	
	return Out;
}

PS_OUT PS_MAIN_ELECPARTS(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    uint iX = In.iSpriteIndexOffset % 3;
    uint iY = In.iSpriteIndexOffset / 3;
    float2 vNewTexcoord = In.vTexcoord * 0.33f + float2(iX / 3.f, iY / 3.f);
    Out.vColor = g_Texture.Sample(LinearSampler, vNewTexcoord);
    float fNoise = g_NoiseMaskTexture.Sample(LinearSampler, In.vTexcoord).r;
	 
    if (Out.vColor.a <= 0.33f || 1.f <= In.fLifeRatio)
        discard;

    Out.vColor.rgb = float3(1.f, 1.f, 1.f);
	
    if (In.fLifeRatio < 0.5)  // 투명에서 불투명으로 (noiseValue가 더 크면 알파값 증가)
        Out.vColor.a = smoothstep(0.0, In.fLifeRatio * 2.0, fNoise);
    else // 불투명에서 투명으로 (noiseValue가 더 작으면 알파값 감소)
        Out.vColor.a = 1.0 - smoothstep(0.5, 1.0, (In.fLifeRatio - 0.5) * 2.0) * (1.0 - fNoise);
	
    if (Out.vColor.a <= 0.05f)
        discard;
	
    return Out;
}

technique11 DefaultTechnique
{
	pass Default // 0
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

    pass ElecParts // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ELECPARTS();
    }
}