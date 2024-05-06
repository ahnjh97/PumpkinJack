#include "Engine_Shader_Defines.hlsli"
/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_CirclePositions[2];

texture2D   g_DiffuseTexture;
texture2D   g_Circle[2];
texture2D   g_HighFreq;

float       g_fOffset_Y;
float       g_fTime;
float       g_fAlpha;
float       g_fColorOffset;
float       g_fCamFar;

float       g_fCircleTime0;
float       g_fCircleTime1;
float       g_fDissolveTime;

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
    float4      vWorldPos : TEXCOORD1;
    float4      vProjPos : TEXCOORD2;
    float4      vLocalPos : TEXCOORD3;
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
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    Out.vLocalPos = vector(In.vPosition, 1);
    
	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
    float3      vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
    float4      vWorldPos : TEXCOORD1;
    float4      vProjPos : TEXCOORD2;
    float4      vLocalPos : TEXCOORD3;
};

struct PS_OUT_ELECARENA
{
    float4 vColor : SV_TARGET0;
    //float4 vNormal : SV_TARGET1;
    //float4 vDepth : SV_TARGET2;
    //float4 vFieldDepth : SV_TARGET3;
    float4 vMagicCircle : SV_TARGET3;
    float4 vSkillDepth : SV_TARGET5;
};

float2 Compute_CircleUV(float4 vPos, float2 vCircleCoord, float fRange)
{
    return float2((vPos.x - (vCircleCoord.x - fRange * 0.5f)) / fRange, ((vCircleCoord.y + fRange * 0.5f) - vPos.z) / fRange);
}

PS_OUT_ELECARENA PS_ELECARENA(PS_IN In)
{
    PS_OUT_ELECARENA Out = (PS_OUT_ELECARENA) 0;
    
    float fOuterRadius = 0.5;
    float fInnerRadius = 0.4;
    float fLightningEndTime = 1.8;
    float fSecondCircleEndTime = 1;
    
    float3 upDirection = float3(0, 1, 0);
    float dotProduct = dot(normalize(In.vNormal), upDirection);
    
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= Out.vColor.a)
        discard;

    Out.vColor.rgb += g_fColorOffset;
    
    // 내적 값이 양수이면 노멀 벡터가 위를 향하고 있다고 판단
    if (dotProduct > -0.1f)
    {        
        float fFirstCircleTime = 0.4f;
        float fSecondRadius = 0.f;
        float2 vCenter = float2(0.5, 0.5);
        
        float fCircleRange = 22.f;
        float4 finalColor;
        
        float2 vCirclePos0[8] =
        {
            float2(g_CirclePositions[0]._11, g_CirclePositions[0]._12), float2(g_CirclePositions[0]._13, g_CirclePositions[0]._14),
            float2(g_CirclePositions[0]._21, g_CirclePositions[0]._22), float2(g_CirclePositions[0]._23, g_CirclePositions[0]._24),
            float2(g_CirclePositions[0]._31, g_CirclePositions[0]._32), float2(g_CirclePositions[0]._33, g_CirclePositions[0]._34),
            float2(g_CirclePositions[0]._41, g_CirclePositions[0]._42), float2(g_CirclePositions[0]._43, g_CirclePositions[0]._44)
        };
        
        float2 vCirclePos1[8] =
        {
            float2(g_CirclePositions[1]._11, g_CirclePositions[1]._12), float2(g_CirclePositions[1]._13, g_CirclePositions[1]._14),
            float2(g_CirclePositions[1]._21, g_CirclePositions[1]._22), float2(g_CirclePositions[1]._23, g_CirclePositions[1]._24),
            float2(g_CirclePositions[1]._31, g_CirclePositions[1]._32), float2(g_CirclePositions[1]._33, g_CirclePositions[1]._34),
            float2(g_CirclePositions[1]._41, g_CirclePositions[1]._42), float2(g_CirclePositions[1]._43, g_CirclePositions[1]._44)
        };
        
        for (int i = 0; i < 8; i++)
        {
            if (vCirclePos0[i].x - fCircleRange * 0.5f < In.vWorldPos.x && In.vWorldPos.x <= vCirclePos0[i].x + fCircleRange * 0.5f &&
		    vCirclePos0[i].y - fCircleRange * 0.5f < In.vWorldPos.z && In.vWorldPos.z <= vCirclePos0[i].y + fCircleRange * 0.5f)
            {
                if (Out.vMagicCircle.a >= 0.3 || vCirclePos0[i].x == 0 || In.vLocalPos.y < 7.2f)
                    continue;
                
                Out.vMagicCircle.xyz = float3(1, 1, 1);
                
                float2 vCircleUV0 = Compute_CircleUV(In.vWorldPos, vCirclePos0[i], fCircleRange);
                
                float fDis0 = distance(vCircleUV0, vCenter);
                float fCircle = g_Circle[1].Sample(LinearSampler, vCircleUV0).a;
                
                if (g_fCircleTime0 < fFirstCircleTime)
                {
                    if (fDis0 <= fOuterRadius && fDis0 >= fInnerRadius)
                        Out.vMagicCircle.a = smoothstep(0, 1, g_fCircleTime0 * 2.f);
                    
                    if (fDis0 < fInnerRadius)
                        Out.vMagicCircle.a = 0;
                    
                }
                else
                {
                    float fSecondCircle = 0;
        
                    if (g_fCircleTime0 <= 0.8f)
                    {
                        fSecondCircle = (g_fCircleTime0 - fFirstCircleTime) / (fSecondCircleEndTime - fFirstCircleTime);
                        fSecondRadius = smoothstep(0, 1, fSecondCircle) * 0.4;
                        if (fDis0 <= fOuterRadius && fDis0 >= fInnerRadius)
                            Out.vMagicCircle.a = 1;
                    }
            
                    if (g_fCircleTime0 > 0.8f)
                    {
                        fSecondCircle = (g_fCircleTime0 - 1) / (fLightningEndTime - 1);
                        fSecondRadius = smoothstep(1, 0, fSecondCircle * 1.5) * 0.4;
            
                        float fFade = (g_fCircleTime0 - fFirstCircleTime) / (fLightningEndTime - fFirstCircleTime);
                        if (fDis0 <= fOuterRadius)
                            Out.vMagicCircle.a = smoothstep(1, 0, fFade);
                    }
        
                    if (fDis0 <= fInnerRadius)
                        Out.vMagicCircle.a = fSecondRadius * 2.2;
                    
                }
                
                //if (fCircle > 0.95 && Out.vMagicCircle.a > 0.3)
                //{
                //    if (dotProduct < 0)
                //        Out.vMagicCircle.a = 0.3;
                //}
                
                if ((fCircle <= 0.1 || g_fCircleTime0 >= fLightningEndTime) && fDis0 <= fOuterRadius)
                    Out.vMagicCircle.a = 0;
                
                if (Out.vMagicCircle.a != 0)
                    Out.vSkillDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCamFar, 0.0f, 0.0f);
            }
        }
        
        for (int j = 0; j < 8; j++)
        {
            if (vCirclePos1[j].x - fCircleRange * 0.5f < In.vWorldPos.x && In.vWorldPos.x <= vCirclePos1[j].x + fCircleRange * 0.5f &&
		        vCirclePos1[j].y - fCircleRange * 0.5f < In.vWorldPos.z && In.vWorldPos.z <= vCirclePos1[j].y + fCircleRange * 0.5f)
            {
                if (Out.vMagicCircle.a >= 0.3 || vCirclePos1[j].x == 0 || In.vLocalPos.y < 7.2f)
                    continue;
                
                Out.vMagicCircle.xyz = float3(1, 1, 1);
                
                float2 vCircleUV1 = Compute_CircleUV(In.vWorldPos, vCirclePos1[j], fCircleRange);
                
                float fDis1 = distance(vCircleUV1, vCenter);
                float fCircle = g_Circle[1].Sample(LinearSampler, vCircleUV1).a;
                
                if (g_fCircleTime1 < fFirstCircleTime)
                {
                    if (fDis1 <= fOuterRadius && fDis1 >= fInnerRadius)
                        Out.vMagicCircle.a = smoothstep(0, 1, g_fCircleTime1 * 2.f);
                    
                    if (fDis1 < fInnerRadius)
                        Out.vMagicCircle.a = 0;
                    
                }
                else
                {
                    float fSecondCircle = 0;
        
                    if (g_fCircleTime1 <= 0.8f)
                    {
                        fSecondCircle = (g_fCircleTime1 - fFirstCircleTime) / (fSecondCircleEndTime - fFirstCircleTime);
                        fSecondRadius = smoothstep(0, 1, fSecondCircle) * 0.4;
                        if (fDis1 <= fOuterRadius && fDis1 >= fInnerRadius)
                            Out.vMagicCircle.a = 1;
                    }
            
                    if (g_fCircleTime1 > 0.8f)
                    {
                        fSecondCircle = (g_fCircleTime1 - 1) / (fLightningEndTime - 1);
                        fSecondRadius = smoothstep(1, 0, fSecondCircle * 1.5) * 0.4;
            
                        float fFade = (g_fCircleTime1 - fFirstCircleTime) / (fLightningEndTime - fFirstCircleTime);
                        if (fDis1 <= fOuterRadius)
                            Out.vMagicCircle.a = smoothstep(1, 0, fFade);
                    }
        
                    if (fDis1 <= fInnerRadius)
                        Out.vMagicCircle.a = fSecondRadius * 2.2;
            
                }
                
                //if (fCircle > 0.95 && Out.vMagicCircle.a > 0.3)
                //{
                //    if (dotProduct < 0)
                //        Out.vMagicCircle.a = 0.3;
                //}
                
                if ((fCircle <= 0.1 || g_fCircleTime1 >= fLightningEndTime) && fDis1 <= fOuterRadius)
                    Out.vMagicCircle.a = 0;
                
                if (Out.vMagicCircle.a != 0)
                    Out.vSkillDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCamFar, 0.0f, 0.0f);
            }
        }
    }
    
    float fDissolve = g_HighFreq.Sample(LinearSampler, In.vTexcoord).r;
      
    if (fDissolve < g_fDissolveTime * 1.3f)
        discard;
    
    //Out.vNormal = vector((In.vNormal * 0.5f + 0.5f).xyz, 0.f);
    //Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fCamFar, 0.0f, 0.0f);
    
    //Out.vFieldDepth = vector(In.vProjPos.z / In.vProjPos.w, 0, 0, 0); //  월드위치 추출하기 위한 깊이
    
    return Out;
}

technique11 DefaultTechnique
{
    pass ElecArena // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_ELECARENA();
    }
}