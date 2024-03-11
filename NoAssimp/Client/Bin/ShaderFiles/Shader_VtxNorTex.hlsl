

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

vector g_vLightDir = vector(1.f, -1.f, 1.f, 0.f);
vector g_vLightDiffuse = vector(1.f, 1.f, 1.f, 1.f);
vector g_vLightAmbient = vector(1.f, 1.f, 1.f, 1.f);
vector g_vLightSpecular = vector(1.f, 1.f, 1.f, 1.f);

texture2D g_DiffuseTexture[4];
texture2D g_MaskTexture;
texture2D g_BrushTexture;

vector g_vMtrlAmbient = vector(0.4f, 0.4f, 0.4f, 1.f);
vector g_vMtrlSpecular = vector(1.f, 1.f, 1.f, 1.f);

vector g_vCamPosition;

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
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vShade : COLOR0;
    float fSpecular : COLOR1;
    float2 vTexcoord : TEXCOORD0;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord * 10.f;

	return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vShade : COLOR0;
    float fSpecular : COLOR1;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

    vector vSourDiffuse = g_DiffuseTexture[3].Sample(LinearSampler, In.vTexcoord );
    vector vDestDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord );
    //vector vBrush = g_BrushTexture.Sample(LinearSampler, In.vTexcoord);
    vector vMask = g_MaskTexture.Sample(PointSampler, In.vTexcoord / 10);
    vector vDiffuse = vDestDiffuse * vMask + vSourDiffuse * (1.f - vMask);
	
    Out.vColor = vDiffuse;
	
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