struct VSInput
{
    float3 Position : POSITION; //位置座標
    float4 Color : COLOR; //色
};

struct VSOutput
{
    float4 Position : SV_POSITION; //変換後の位置座標
    float4 Color : COLOR; //色
};

//Tranform 定数バッファ
cbuffer Transform : register(b0)
{
    float4x4 World : packoffset(c0); //ワールド変換行列
    float4x4 View : packoffset(c4); // ビュー変換行列
    float4x4 Proj : packoffset(c8); // プロジェクション変換行列
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    float4 localPos = float4(input.Position, 1.0f); //ローカル座標を4次元ベクトルに
    float4 worldPos = mul(localPos, World); //ワールド変換
    float4 viewPos = mul(worldPos, View); //ビュー変換
    float4 projPos =   mul(viewPos, Proj); //プロジェクション変換
    
    output.Position = projPos;
    output.Color = input.Color;

    

    return output;
}