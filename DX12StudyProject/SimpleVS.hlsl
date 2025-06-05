struct VSInput
{
    float3 Position : POSITION; //�ʒu���W
    float4 Color : COLOR; //�F
};

struct VSOutput
{
    float4 Position : SV_POSITION; //�ϊ���̈ʒu���W
    float4 Color : COLOR; //�F
};

//Tranform �萔�o�b�t�@
cbuffer Transform : register(b0)
{
    float4x4 World : packoffset(c0); //���[���h�ϊ��s��
    float4x4 View : packoffset(c4); // �r���[�ϊ��s��
    float4x4 Proj : packoffset(c8); // �v���W�F�N�V�����ϊ��s��
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    float4 localPos = float4(input.Position, 1.0f); //���[�J�����W��4�����x�N�g����
    float4 worldPos = mul(localPos, World); //���[���h�ϊ�
    float4 viewPos = mul(worldPos, View); //�r���[�ϊ�
    float4 projPos =   mul(viewPos, Proj); //�v���W�F�N�V�����ϊ�
    
    output.Position = projPos;
    output.Color = input.Color;

    

    return output;
}