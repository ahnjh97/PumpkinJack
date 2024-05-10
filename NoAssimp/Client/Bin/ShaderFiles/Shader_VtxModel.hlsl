#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D   g_DiffuseTexture, g_WaterTexture, g_CausticTexture, g_NoiseTexture;
texture2D   g_TigerStripes, g_TopBottomFadeTex;
texture2D   g_FireNoise, g_VoronoiBlur;
texture2D   g_Circle[2];
texture2D   g_AttackEffect[6];

float       g_fOffset_Y;
float       g_fTime;
float       g_fAlpha;
float       g_fColorOffset;
float       g_fCamFar;
float       g_fPlayRatio;

uint        g_iWeaponIndex;
uint        g_iSpriteIndex;

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
    float3      vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
    //float4      vWorldPos : TEXCOORD1;
    float4      vProjPos : TEXCOORD2;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTexcoord = In.vTexcoord;
    //Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
    float3      vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
    //float4      vWorldPos : TEXCOORD1;
    float4      vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4      vColor : SV_TARGET0;
    float4      vNormal : SV_TARGET1;
    float4      vDepth : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT	Out = (PS_OUT)0;
    
	Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= Out.vColor.a)
        discard;
    
	return Out;
}

PS_OUT PS_SLIMEBOSSROOM(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    In.vTexcoord.y += g_fTime * 0.2f;
    In.vTexcoord.y = fmod(In.vTexcoord.y, 1.0);
    
    //float3 darkGreen = float3(0.0, 0.5, 0.0); // 어두운 누리끼리한 초록색
    //float3 lightGreen = float3(0.5, 1.0, 0.5); // 밝은 초록색
    //float3 slimeDarkGreen = float3(0.0, 0.8, 0.0); // 슬라임의 어두운 부분
    float3 slimeLightGreen = float3(0.4, 1.0, 0.4); // 슬라임의 밝은 부분
    
    vector vDiffuseColor = g_WaterTexture.Sample(LinearSampler, In.vTexcoord);
    vector vCausticColor = g_CausticTexture.Sample(LinearSampler, In.vTexcoord);
    float3 resultColor = lerp(vDiffuseColor, slimeLightGreen, vCausticColor);
    Out.vColor = float4(resultColor, 0.5);
    
    return Out;
}

PS_OUT PS_KNIFE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);

    Out.vColor.a = g_fAlpha;
    if (0.1f >= Out.vColor.a)
        discard;
    
    return Out;
}

PS_OUT PS_ARENAFIRE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    In.vTexcoord.y += g_fTime * 0.1f;
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord * 6.f);
    vector vNoiseColor = g_NoiseTexture.Sample(LinearSampler, In.vTexcoord * 4.f);
  
    float3 darkOrange = float3(0.6, 0.2, 0.2); 
    float3 redPurple = float3(0.9, 0.4, 0.3); 
    float3 color = lerp(darkOrange, redPurple, Out.vColor.xyz);
    Out.vColor.xyz = color;
    Out.vColor.a = lerp(Out.vColor.a * 0.5f, 0.01, vNoiseColor.r);

    return Out;
}

PS_OUT PS_TORNADO(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, (In.vTexcoord.x, In.vTexcoord.y - g_fTime * 0.6f) * 1.5f); // CloudTex
    vector vTigerStripes = g_TigerStripes.Sample(LinearSampler, In.vTexcoord - g_fTime * 0.6f);
    vector vTopBottomFade = g_TopBottomFadeTex.Sample(LinearSampler, In.vTexcoord);
  
    //Out.vColor = vDiffuse;
    float4 finalColor = lerp(vTigerStripes, vDiffuse, vDiffuse.r);
    Out.vColor = finalColor;
    Out.vColor.a = vTopBottomFade.r /** (1 - vTigerStripes.r) */* finalColor.g;
    if (Out.vColor.a <= 0.15f)
        discard;
    
    return Out;
}

PS_OUT PS_HOLEWALL(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 smokeUV = In.vTexcoord;
    float smokeValue = g_VoronoiBlur.Sample(LinearSampler, smokeUV).r;

    float2 fireUV = In.vTexcoord + g_fTime * 0.1;
    float fireNoise = g_FireNoise.Sample(LinearSampler, fireUV).r;

    // voronoiBlur의 값이 높을수록 fireNoise의 영향을 많이 줌
    float finalValue = lerp(1 - smokeValue + 0.2, fireNoise * 0.5, 1 - smokeValue + 0.2);

    Out.vColor = float4(1, 1, 1, 1 - finalValue - 0.2);

    if (Out.vColor.a <= 0.1f)
        discard;

    return Out;
}

PS_OUT PS_SPHERE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    Out.vColor.rgb = float3(2, 2, 2);
    Out.vColor.a = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord - g_fTime * 0.2).r;
    if (0.4 >= Out.vColor.a)
        discard;
    
    return Out;
}

PS_OUT PS_SLASH(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    if (g_fPlayRatio < In.vTexcoord.x * 0.25f)
        discard;
    if (g_fPlayRatio > 0.75f + In.vTexcoord.x * 0.25f)
        discard;
    
    float2 adjustedUV = In.vTexcoord;
    adjustedUV.y = (adjustedUV.y * 0.8) + 0.5; // Y 좌표 조정
    // 텍스처에서 색상 정보를 샘플링
    float4 color = float4(0, 0, 0, 0);
    //color = g_AttackEffect[0].Sample(LinearSampler, In.vTexcoord);
    if (g_iSpriteIndex == 0)
        color = g_AttackEffect[0].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 1)
        color = g_AttackEffect[1].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 2)
        color = g_AttackEffect[2].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 3)
        color = g_AttackEffect[3].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 4)
        color = g_AttackEffect[4].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 5)
        color = g_AttackEffect[5].Sample(LinearSampler, In.vTexcoord);
    
    if (0.1f >= color.a)
        discard;
    
    float4 vResult = float4(0, 0, 0, 0);
    
    if (g_iWeaponIndex == 0)
    {
        if (color.b > color.r && color.b > color.g)
        {
            // 파란색-> 초록색
            float avg = (color.r + color.b) * 0.5;
            vResult = vector(color.r * 0.8f, 1, color.g * 0.8f, color.a);
        }
    }
    else
    {
        if (color.b > color.r && color.b > color.g)
        {
            // 파란색-> 보라색
            float avg = (color.r + color.b) * 0.5;
            vResult = vector(avg, color.g * 0.2, avg, color.a);
        }
    }
    
    Out.vColor = vResult;
    if (0.3f >= Out.vColor.a)
        discard;
    
     return Out;
}

PS_OUT PS_SOLDIER_SLASH(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    if (g_fPlayRatio < In.vTexcoord.x * 0.25f)
        discard;
    if (g_fPlayRatio > 0.75f + In.vTexcoord.x * 0.25f)
        discard;
    
    float2 adjustedUV = In.vTexcoord;
    adjustedUV.y = (adjustedUV.y * 0.8) + 0.1; // Y 좌표 조정
    // 텍스처에서 색상 정보를 샘플링
    float4 color = float4(0, 0, 0, 0);
    if (g_iSpriteIndex == 0)
        color = g_AttackEffect[0].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 1)
        color = g_AttackEffect[1].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 2)
        color = g_AttackEffect[2].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 3)
        color = g_AttackEffect[3].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 4)
        color = g_AttackEffect[4].Sample(LinearSampler, In.vTexcoord);
    else if (g_iSpriteIndex == 5)
        color = g_AttackEffect[5].Sample(LinearSampler, In.vTexcoord);

    if (0.1f >= color.a)
        discard;

    Out.vColor = color;
    if (0.3f >= Out.vColor.a)
        discard;
        
    return Out;
}

PS_OUT PS_DAMAGABLE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float3 vDamageColor = float3(2.f, 0.45f, 0);
    
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= Out.vColor.a)
        discard;
    
    if (g_fPlayRatio < 0.5f)
        Out.vColor.rgb += vDamageColor * g_fPlayRatio;
    else if (g_fPlayRatio < 1.f)
        Out.vColor.rgb += vDamageColor * (1 - g_fPlayRatio);
    
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
        PixelShader = compile ps_5_0 PS_SLIMEBOSSROOM();
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
        PixelShader = compile ps_5_0 PS_KNIFE();
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
        PixelShader = compile ps_5_0 PS_ARENAFIRE();
    }

    pass SkySphere // 5
    {
        SetRasterizerState(RS_Sky);
        SetDepthStencilState(DSS_Sky, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Tornado // 6
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_TORNADO();
    }

    pass HoleWall // 7
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_HOLEWALL();
    }

    pass Sphere // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SPHERE();
    }

    pass Slash // 9
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SLASH();
    }

    pass SoldierSlash // 10
    {
        SetRasterizerState(RS_NoCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SOLDIER_SLASH();
    }

    pass Damagable // 11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_DAMAGABLE();
    }
}