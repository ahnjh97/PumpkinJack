#include "Engine_Shader_Defines.hlsli"


/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture, g_WaterTexture, g_CausticTexture;
float g_fOffset_Y;
float g_fAlpha;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float3		vTangent : TANGENT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= Out.vColor.a)
        discard;
    
	return Out;
}

PS_OUT PS_MAIN_SLIMEBOSSROOM(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    In.vTexcoord.y += g_fOffset_Y;
    In.vTexcoord.y = fmod(In.vTexcoord.y, 1.0);
    
    //float3 darkGreen = float3(0.0, 0.5, 0.0); // 어두운 누리끼리한 초록색
    //float3 lightGreen = float3(0.5, 1.0, 0.5); // 밝은 초록색
    //float3 slimeDarkGreen = float3(0.0, 0.8, 0.0); // 슬라임의 어두운 부분
    float3 slimeLightGreen = float3(0.4, 1.0, 0.4); // 슬라임의 밝은 부분
    
    vector vDiffuseColor = g_WaterTexture.Sample(LinearSampler, In.vTexcoord);
    vector vCausticColor = g_CausticTexture.Sample(LinearSampler, In.vTexcoord);
    float3 resultColor = lerp(vDiffuseColor, slimeLightGreen, vCausticColor);
    Out.vColor = float4(resultColor, 0.5f);
    
    return Out;
}

PS_OUT PS_MAIN_KNIFE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= Out.vColor.a)
        discard;
    
    Out.vColor.a = g_fAlpha;
    
    return Out;
}

PS_OUT PS_MAIN_ARENAFIRE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    In.vTexcoord.y += g_fOffset_Y;
    
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord * 7.f);
    if (0.3f >= Out.vColor.a)
        discard;
    
    float3 darkOrange = float3(0.6, 0.3, 0.1); // 어두운 주황색
    float3 redPurple = float3(0.7, 0.2, 0.5); // 붉은 자주색
    float3 color = lerp(darkOrange, redPurple, Out.vColor.r);
    Out.vColor.xyz = color;
    Out.vColor.a = 0.3f;
    
    return Out;
}

technique11 DefaultTechnique
{
	pass Default // 0
	{
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

    pass NoCull // 1
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass SlimeBossRoom // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SLIMEBOSSROOM();
    }

    pass Knife // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_KNIFE();
    }

    pass ArenaFire // 4
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ARENAFIRE();
    }
}