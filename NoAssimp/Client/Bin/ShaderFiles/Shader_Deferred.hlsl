#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;

texture2D	g_Texture;
texture2D	g_NormalTexture;	
texture2D	g_DiffuseTexture;
texture2D	g_ShadeTexture;
texture2D	g_DepthTexture;
texture2D	g_SpecularTexture;
texture2D	g_FieldDepthTexture;
texture2D	g_MagicCircleTexture;
texture2D	g_SkillEffectTexture;
texture2D	g_PlayerDepthTexture;
texture2D	g_SkillDepthTexture;

float4		g_vLightDir;
float4		g_vLightPos;
float		g_fLightRange;

float4		g_vLightDiffuse;
float4		g_vLightAmbient;
float4		g_vLightSpecular;

float4		g_vMtrlAmbient = float4(1.f, 1.f, 1.f, 1.f);
float4		g_vMtrlSpecular = float4(1.f, 1.f, 1.f, 1.f);

float4		g_vCamPosition;

float		g_fCamFar;

struct VS_IN
{
	float3		vPosition : POSITION;	
	float2		vTexcoord : TEXCOORD;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;	
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
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
	float2		vTexcoord : TEXCOORD;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

	return Out;
}

struct PS_OUT_LIGHT
{
	float4		vShade : SV_TARGET0;
	float4		vSpecular : SV_TARGET1;
};

struct PS_OUT_SSD
{
    float4 vSkillEffect : SV_TARGET0;
    //float fDepth : SV_Depth;
};

/* 빛 하나당 480000 수행되는 쉐이더. */
PS_OUT_LIGHT PS_DIRECTIONAL(PS_IN In)
{
	PS_OUT_LIGHT			Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	float4		vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
	float		fViewZ = vDepthDesc.y * g_fCamFar;
	
	Out.vShade = g_vLightDiffuse * saturate(max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + g_vLightAmbient * g_vMtrlAmbient);

	float4		vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	float4		vLook = vWorldPos - g_vCamPosition;
	float4		vReflect = reflect(normalize(g_vLightDir), vNormal);	

	float		fSpecular = pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;

	return Out;
}

PS_OUT_LIGHT PS_POINT(PS_IN In)
{
	PS_OUT_LIGHT			Out = (PS_OUT_LIGHT)0;

	vector		vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
	float4		vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

	vector		vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    float		fViewZ = vDepthDesc.y * g_fCamFar;

	float4		vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
	vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
	vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
	vWorldPos.z = vDepthDesc.x;
	vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
	vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
	vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
	vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

	vector		vLightDir = vWorldPos - g_vLightPos;
	float		fDistance = length(vLightDir);

	float		fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);

	Out.vShade = g_vLightDiffuse * saturate(max(dot(normalize(vLightDir) * -1.f, vNormal), 0.f) + g_vLightAmbient * g_vMtrlAmbient);
	Out.vShade *= fAtt;
	
	float4		vLook = vWorldPos - g_vCamPosition;
	float4		vReflect = reflect(normalize(vLightDir), vNormal);

	float		fSpecular = pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 30.f);

	Out.vSpecular = (g_vLightSpecular * g_vMtrlSpecular) * fSpecular;
	Out.vSpecular *= fAtt;

	return Out;
}

PS_OUT_SSD PS_SSD(PS_IN In)
{
    PS_OUT_SSD Out = (PS_OUT_SSD) 0;

    Out.vSkillEffect = g_MagicCircleTexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vSkillEffect.a < 0.2)
        Out.vSkillEffect.a = 0;
    
	//float depth = In.vPosition.z / In.vPosition.w;
	
    //if (Out.vSkillEffect.a ==0)
    //    discard;
    //Out.fDepth = g_SkillDepthTexture.Sample(LinearSampler, In.vTexcoord).r + 0.01; // ;
    ////;
	
    //vector vSkillDepth = g_SkillDepthTexture.Sample(PointSampler, In.vTexcoord);
    //vector vPlayerDepth = g_PlayerDepthTexture.Sample(PointSampler, In.vTexcoord);
    //if (Out.vSkillEffect.a > 0.1 && vPlayerDepth.r > vSkillDepth.r)
    //    Out.vSkillEffect.a = 0.f;
    //if (vPlayerDepth.x != 0 || vPlayerDepth.y != 0 && Out.vSkillEffect.a > 0.3)
    //    Out.vSkillEffect.a -= 0.5;
    //Out.fDepth = g_FieldDepthTexture.Sample(LinearSampler, In.vTexcoord);
	
        return Out;
}

/* 최종적ㅇ으로 480000 수행되는 쉐이더. */
PS_OUT PS_FINAL(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector		vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (0.0f == vDiffuse.a)
		discard;
	vector	vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
	vector	vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    float4	vSkillEffect = g_SkillEffectTexture.Sample(LinearSampler, In.vTexcoord);
    
	
	//vector	vSkillDepth = g_SkillDepthTexture.Sample(PointSampler, In.vTexcoord);
    vector vPlayerDepth = g_PlayerDepthTexture.Sample(PointSampler, In.vTexcoord);
    if (vPlayerDepth.r != 0)
        vSkillEffect.a = 0;
    //if (vPlayerDepth.r < vSkillDepth.r)
    //    vSkillEffect.a = 0.f;
	
    //vSkillEffect.a = saturate(vSkillEffect.a);
	
	//Out.vColor = colorLighted;
        vector colorLighted = vDiffuse * vShade + vSpecular;
	Out.vColor = (1.0f - vSkillEffect.a) * colorLighted + vSkillEffect.a * vSkillEffect;

	return Out;
}

technique11 DefaultTechnique
{
	pass Debug  // 0
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass Light_Directional  // 1
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_DIRECTIONAL();
	}

	pass Light_Point  // 2
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_POINT();
	}

	pass Final  // 3
	{
        SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_FINAL();
	}

    pass SSD // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SSD();
    }
}