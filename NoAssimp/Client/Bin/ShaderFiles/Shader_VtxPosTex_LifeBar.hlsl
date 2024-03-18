

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D	g_DiffuseTexture[2];
texture2D	g_MaskTexture;
float		g_Offset_Y;

sampler LinearSampler = sampler_state
{
	Filter = MIN_MAG_MIP_LINEAR;
    AddressU = wrap;
    AddressV = wrap;
};

sampler PointSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = wrap;
    AddressV = wrap;
};



struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
    float2		vTexcoord : TEXCOORD0;
    float2		vTexcoord2 : TEXCOORD1;
    float2      vTexcoord3 : TEXCOORD2;
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
    Out.vTexcoord2 = In.vTexcoord;
    Out.vTexcoord3 = In.vTexcoord;
	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
    float2		vTexcoord2 : TEXCOORD1;
    float2      vTexcoord3 : TEXCOORD2;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	vector vMask = g_MaskTexture.Sample(PointSampler, In.vTexcoord2);
    In.vTexcoord.y += g_Offset_Y * 3.f;
    In.vTexcoord3.y += g_Offset_Y * 1.6f;
    In.vTexcoord.y = fmod(In.vTexcoord.y / 3.f, 1.0);
    In.vTexcoord3.y = fmod(In.vTexcoord3.y / 3.f, 1.f);
    vector vDstDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord);
    vector vSrcDiffuse = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord3);
	
    float3 lightGreenColor = float3(0.3, 0.9, 0.3); // 밝은 초록색
    float3 darkGreenColor = float3(0, 0.5, 0); // 진한 초록색
    float lerpFactor = saturate(vDstDiffuse.r * 1.2f); // 색상 변환 가중치, 0 ~ 1 범위로 제한
    float lerpFactor2 = saturate(vSrcDiffuse.r * 0.7f);
    float3 newColor = lerp(lightGreenColor, darkGreenColor, lerpFactor);
    float3 newColor2 = lerp(lightGreenColor, darkGreenColor, lerpFactor2);

    vector vNewColor1 = vector(newColor, vDstDiffuse.a); // 알파 값은 변경하지 않음
    vector vNewColor2 = vector(newColor2, vSrcDiffuse.a);
    vector vDiffuse = vNewColor1 * vMask * vNewColor2;
    Out.vColor = vDiffuse;
    if (Out.vColor.g <= 0.1f)
        discard;

     return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
