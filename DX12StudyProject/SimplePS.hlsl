struct VSOutput
{
    float4 Position : SV_POSITION; //�ϊ���̈ʒu���W
    float4 Color : COLOR; //�F
};

// �s�N�Z���V�F�[�_�[�̓���
struct PSOutput
{
    float4 Color : SV_TARGET0;
};

PSOutput main(VSOutput input)
{
    PSOutput output = (PSOutput)0;

    // ���͂̐F�����̂܂܏o��
    output.Color = input.Color;

    return output;
}