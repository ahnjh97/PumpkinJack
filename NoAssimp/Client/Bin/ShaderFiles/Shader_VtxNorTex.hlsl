#include "Engine_Shader_Defines.hlsli"
/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture[4];
texture2D g_MaskTexture;
texture2D g_WaterTexture;

float g_fCamFar;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{   
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

    vector vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    vector vWorldNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    
	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vNormal = vWorldNormal;
	Out.vTexcoord = In.vTexcoord * 100.f;
    Out.vWorldPos = vWorldPos;
    Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

    vector vSourDiffuse = g_WaterTexture.Sample(LinearSampler, In.vTexcoord * 0.08f);
    vector vDestDiffuse = g_DiffuseTexture[3].Sample(LinearSampler, In.vTexcoord );
    vector vMask = g_MaskTexture.Sample(PointSampler, In.vTexcoord * 0.01f);
    vector vDiffuse = vDestDiffuse * vMask + vSourDiffuse * (1.f - vMask);
	
    Out.vDiffuse = vector(vDiffuse.xyz, 1.f);
    Out.vNormal = vector((In.vNormal * 0.5f + 0.5f).xyz, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCamFar, 0.0f, 0.0f);
    
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
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

    pass RS_Wireframe
    {
        SetRasterizerState(RS_Wireframe);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}