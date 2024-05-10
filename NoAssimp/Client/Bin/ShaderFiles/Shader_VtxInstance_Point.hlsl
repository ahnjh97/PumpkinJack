#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_Texture;
texture2D g_MaskTexture;
texture2D g_NoiseTexture;
texture2D g_AttackEffect[6];

vector	g_vCamPosition;
float   g_fTime;
bool    g_bFadeIn;

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

PS_OUT PS_ELECPARTS(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
   
    uint iX = In.iSpriteIndexOffset % 3;
    uint iY = In.iSpriteIndexOffset / 3;
    float2 vNewTexCoord = In.vTexcoord * 0.33f + float2(iX / 3.f, iY / 3.f);
    Out.vColor = g_Texture.Sample(LinearSampler, vNewTexCoord);
	 
    if (Out.vColor.a <= 0.33f || 1.f <= In.fLifeRatio || 0 > In.fLifeRatio)
        discard;

    Out.vColor.rgb = float3(1.f, 1.f, 1.f);

    if (true == g_bFadeIn)
    {
        if (In.fLifeRatio >= 0 && In.fLifeRatio < 0.5)  // 투명에서 불투명으로 
            Out.vColor.a = smoothstep(0, In.fLifeRatio * 2, 0);
        else if (In.fLifeRatio < 1) // 불투명에서 투명으로
            Out.vColor.a = 1.0 - smoothstep(0.5, 1.0, (In.fLifeRatio - 0.5) * 2.0);
    }
	else
    {
        if (In.fLifeRatio >= 0.5) // 불투명에서 투명으로 
            Out.vColor.a = 1.0 - smoothstep(0.5, 1.0, (In.fLifeRatio - 0.5) * 2.0);
		else
            Out.vColor.a = 1;
    }
	
    if (Out.vColor.a <= 0.05f)
        discard;
	
    return Out;
}

PS_OUT PS_ARROWEFFECT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    float fNoise = g_NoiseTexture.Sample(LinearSampler, In.vTexcoord * 0.2f).r;
    float2 vNewUV = In.vTexcoord + (fNoise - 0.5) * 0.05;
    
    Out.vColor = g_Texture.Sample(LinearSampler, vNewUV);

    if (Out.vColor.a <= 0.33f)
        discard;

    Out.vColor.rgb = float3(1.f, 0.6f, 0.f);
    
    return Out;
}

PS_OUT PS_EXPLOSION(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 vCenter = float2(0.5, 0.5);
    float fDistance = length(In.vTexcoord - vCenter);
    
    if (1.f <= In.fLifeRatio)
        discard;
    
    if (fDistance <= 0.5)
        Out.vColor = float4(1.f, 0.65f, 0.f, 1);
    else
        discard;
    
    float2 vUV = float2(In.vTexcoord.x * 0.15f, In.vTexcoord.y * 0.15f + In.iSpriteIndexOffset * 0.01f);
    float fNoise = g_NoiseTexture.Sample(LinearSampler, vUV).r;
    
    if (fNoise < 0.1)
        discard;
    
     return Out;
}

PS_OUT PS_JUMP(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float fNoise = g_NoiseTexture.Sample(LinearSampler, In.vTexcoord * 0.5f).r;
    
    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

    if (Out.vColor.a <= 0.33f)
        discard;

    if (fNoise < In.fLifeRatio)
        discard;
    
    Out.vColor.rgb = float3(0.8f, 0.8f, 0.8f);
    
    return Out;
}

PS_OUT PS_SLIMEPARTICLES(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    if (1.f <= In.fLifeRatio)
        discard;
    
    float fMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord).r;
    if (0.3f > fMask)
        discard;
    
    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);
    Out.vColor += float4(0.2, 0.28, 0.2, 0);
    
    return Out;
}

PS_OUT PS_SHOCKWAVE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    if (1.f <= In.fLifeRatio)
        discard;
    
    float fMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord).a;
    if (0.3f > fMask)
        discard;
    
    float2 vUV = float2(In.vTexcoord.x * 0.4f, In.vTexcoord.y * 0.4f + In.iSpriteIndexOffset * 0.01f + g_fTime * 0.6f);
    Out.vColor = g_Texture.Sample(LinearSampler, vUV);
    
    float4 color = Out.vColor;
    //float4 vResult = float4(0, 0, 0, 0);
    if ((color.r > color.g) && (color.r + color.g > color.b * 1.5))
    {
        float greenEnhanced = (color.r + color.g) * 0.5;
        float brightness = (color.r + color.g + color.b) / 3;
        float whiteComponent = brightness * 0.5; //
    
        Out.vColor = vector(whiteComponent, greenEnhanced, whiteComponent, color.a);
        Out.vColor.rgb *= 0.85f;
        Out.vColor.a *= 0.8f;
        
        if (Out.vColor.a <= 0.25f)
            discard;
    }
    
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
        PixelShader = compile ps_5_0 PS_ELECPARTS();
    }

    pass ArrowEffect // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_ARROWEFFECT();
    }

    pass Explosion // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_EXPLOSION();
    }

    pass JumpEffect // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_JUMP();
    }

    pass SlimePartices // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SLIMEPARTICLES();
    }

    pass ShockWave // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SHOCKWAVE();
    }
}