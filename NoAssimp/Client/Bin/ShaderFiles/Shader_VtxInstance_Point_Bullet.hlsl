#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_AttackEffect[6];

vector	g_vCamPosition;
float3  g_vPivot;
float   g_fTime;

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
    float3      vUp : COLOR2;
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
    Out.vUp = float3(In.TransformMatrix._31, In.TransformMatrix._32, In.TransformMatrix._33);
    
	return Out;
}

struct GS_IN
{
	float4		vPosition : POSITION;
	float2		vPSize : PSIZE;
    float		fLifeRatio : COLOR0;
    uint        iSpriteIndexOffset : COLOR1;
    float3      vUp : COLOR2;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
    float		fLifeRatio : COLOR0;
};

[maxvertexcount(6)]
void GS_BULLET(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4];

    float3 vPivot = mul(float4(g_vPivot, 1), g_WorldMatrix).xyz;
    float3 vUp = In[0].vPosition.xyz - vPivot;
    float3 vLook = normalize((g_vCamPosition - In[0].vPosition).xyz);
    float3 vRight = normalize(cross(vUp, vLook.xyz)) * In[0].vPSize.x * 0.5f;
    vUp = cross(vLook, vRight);
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = vector(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[0].vTexcoord = float2(0.0f, 0.0f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].fLifeRatio = In[0].fLifeRatio;

    Out[1].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[1].vTexcoord = float2(1.0f, 0.0f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].fLifeRatio = In[0].fLifeRatio;

    Out[2].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[2].vTexcoord = float2(1.0f, 1.0f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].fLifeRatio = In[0].fLifeRatio;

    Out[3].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[3].vTexcoord = float2(0.0f, 1.0f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].fLifeRatio = In[0].fLifeRatio;

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
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_BULLETS(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    if (In.fLifeRatio < 1 / 6.f)
        Out.vColor = g_AttackEffect[0].Sample(LinearSampler, In.vTexcoord);
    else if (In.fLifeRatio < 2 / 6.f)
        Out.vColor = g_AttackEffect[1].Sample(LinearSampler, In.vTexcoord);
    else if (In.fLifeRatio < 3 / 6.f)
        Out.vColor = g_AttackEffect[2].Sample(LinearSampler, In.vTexcoord);
    else if (In.fLifeRatio < 4 / 6.f)
        Out.vColor = g_AttackEffect[3].Sample(LinearSampler, In.vTexcoord);
    else if (In.fLifeRatio < 5 / 6.f)
        Out.vColor = g_AttackEffect[4].Sample(LinearSampler, In.vTexcoord);
    else if (In.fLifeRatio < 1.f)
        Out.vColor = g_AttackEffect[5].Sample(LinearSampler, In.vTexcoord);
    
    if (Out.vColor.a <= 0.3f || 1.f <= In.fLifeRatio)
        discard;

    return Out;
}

technique11 DefaultTechnique
{
    pass Bullets // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_BULLET();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_BULLETS();
    }
}