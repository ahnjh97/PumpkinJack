#include "Engine_Shader_Defines.hlsli"
/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_matViewInverse, g_matProjInverse;

texture2D g_DiffuseTexture;
texture2D g_Circle[2];
texture2D g_Lightning[3];
float g_fAlpha;
float g_fTime;
float fLightningEndTime = 1.8f;
float fSecondCircleEndTime = 1;

texture2D g_ElecNoise;
texture2D g_FieldDepth;
uint g_iSpriteIndex, g_iLightningIndex;

texture2D g_ElecChain[8];

float2 ScreenSize = float2(1280, 720);

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
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

struct PS_OUT_SkillEffect
{
    float4 vSkillEffect : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
 
	return Out;
}

PS_OUT PS_CROWLIFEBAR(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vColor.a == 0.0f)
        discard;
	
    return Out;
}

PS_OUT PS_INVEN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vColor.a *= saturate(g_fAlpha);
    
    return Out;
}

float fOuterRadius = 0.5;
float fInnerRadius = 0.4;

float Compute_HeightFromFieldDepthTexture(float4 vProjPosition)
{
    // Normalized Device Coordinate로 변환
    float3 ndcPos = vProjPosition.xyz / vProjPosition.w;
    
    // NDC 좌표를 뷰포트/스크린 좌표로 변환
    float2 vViewportPos;
    vViewportPos.x = (ndcPos.x + 1.0) * 0.5 * ScreenSize.x;
    vViewportPos.y = (1.0 - ndcPos.y) * 0.5 * ScreenSize.y;
    
    // 뷰포트 좌표를 [0, 1] 범위의 UV 좌표로 변환
    float2 uv = float2(vViewportPos.x / ScreenSize.x, vViewportPos.y / ScreenSize.y);
    
    // 깊이 텍스처에서 깊이 값 샘플링
    float fZ = g_FieldDepth.Sample(PointSampler, uv).r;
    
    // 프로젝션 좌표 계산
    float3 vProjPos = float3(ndcPos.x, ndcPos.y, fZ);
    
    // 프로젝션 좌표를 월드 좌표로 역변환
    float4 worldPosWithDepth = mul(float4(vProjPos, 1.0), g_matProjInverse);
    worldPosWithDepth /= worldPosWithDepth.w; // 원근 나누기
    worldPosWithDepth = mul(worldPosWithDepth, g_matViewInverse);
    
    // 월드 좌표에서 y값 반환
    return worldPosWithDepth.y;
}

PS_OUT PS_CIRCLE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = 1.f;
    
    float fFirstCircleTime = 0.4f;
    
    float2 vCenter = float2(0.5, 0.5);
    float fDis = distance(In.vTexcoord, vCenter);
    float fSecondRadius = 0.f;
    
    if (g_fTime < fFirstCircleTime)
    {
        Out.vColor.a = smoothstep(0, 1, g_fTime * 2.f);
        if (fDis >= fOuterRadius || fDis <= fInnerRadius)
            discard;    
    }
    else
    {
        float fSecondCircle = 0; 
        
        if (g_fTime <= 0.8f)
        {
            fSecondCircle = (g_fTime - fFirstCircleTime) / (fSecondCircleEndTime - fFirstCircleTime);
            fSecondRadius = smoothstep(0, 1, fSecondCircle) * 0.4;
        }
            
        if (g_fTime > 0.8f)
        {
            fSecondCircle = (g_fTime - 1) / (fLightningEndTime - 1);
            fSecondRadius = smoothstep(1, 0, fSecondCircle * 1.5) * 0.4;
            
            float fFade = (g_fTime - fFirstCircleTime) / (fLightningEndTime - fFirstCircleTime);
            Out.vColor.a = smoothstep(1, 0, fFade);
        }
        
        if (fDis >= fOuterRadius)
            discard;
        
        if (fDis <= fInnerRadius)
            Out.vColor.a = fSecondRadius * 2.2;
    }
    
    Out.vColor.a *= g_Circle[1].Sample(LinearSampler, In.vTexcoord).a;
    
    if (Out.vColor.a == 0 || g_fTime > fLightningEndTime)
        discard;
    
    return Out;
}

PS_OUT PS_LIGHTNING(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    int iNumX, iNumY;
    if (g_iLightningIndex == 0 || g_iLightningIndex == 2)
        iNumX = iNumY = 3;
    else if (g_iLightningIndex == 1)    {
        iNumX = 4;
        iNumY = 5;
    }

    float2 vTexCoordOffset = float2(g_iSpriteIndex % iNumX, g_iSpriteIndex / iNumY);
	
    float2 vNewTexCoord = float2((In.vTexcoord.x + vTexCoordOffset.x) / iNumX, (In.vTexcoord.y + vTexCoordOffset.y) / iNumY);

    if (g_iLightningIndex == 0)
        Out.vColor = g_Lightning[0].Sample(LinearSampler, vNewTexCoord);
    else if (g_iLightningIndex == 1)
        Out.vColor = g_Lightning[1].Sample(LinearSampler, vNewTexCoord);
    else if (g_iLightningIndex == 2)
        Out.vColor = g_Lightning[2].Sample(LinearSampler, vNewTexCoord);
    
    if (Out.vColor.a <= 0.3)
        discard;
	
    return Out;
}

PS_OUT PS_ELECCHAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_ElecChain[1].Sample(LinearSampler, In.vTexcoord);
    //Out.vColor.xyz = 1.f;
    Out.vColor.a = g_ElecChain[1].Sample(LinearSampler, In.vTexcoord).r;
    if (0.7 > Out.vColor.a)
        discard;
    
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

    pass CrowLifeBar // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_CROWLIFEBAR();
    }

    pass Inven // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_INVEN();
    }

    pass Portal // 3
    {
        SetRasterizerState(RS_Sky);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_CROWLIFEBAR();
    }

    pass Circle // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_CIRCLE();
    }

    pass Lightning // 5
    {
        SetRasterizerState(RS_Sky);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_LIGHTNING();
    }

    pass ElecChain // 6
    {
        SetRasterizerState(RS_Sky);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_ELECCHAIN();
    }
}