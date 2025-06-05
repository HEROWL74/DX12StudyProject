struct VSOutput
{
    float4 Position : SV_POSITION; //変換後の位置座標
    float4 Color : COLOR; //色
};

// ピクセルシェーダーの入力
struct PSOutput
{
    float4 Color : SV_TARGET0;
};

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    // 入力の色をそのまま出力
    output.Color = input.Color;

    return output;
}