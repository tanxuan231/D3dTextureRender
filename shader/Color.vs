// ���嶥����ɫ��������ṹ��
struct VinputType
{
	float4 position : POSITION;	// ����λ��
	float4 color : COLOR;	// ������ɫ
};

// ���嶥����ɫ��������ṹ�壨����������ɫ��������ṹ��
struct PinputType
{
	float4 position : SV_POSITION;	// λ�á�SV_POSITION������ɫ���ڲ�����
	float4 color : COLOR;	// ��ɫ
};

// ��ں���
PinputType ColorVsMain(VinputType input)
{
	PinputType output;

	// w��������CPU���룬��Ĭ������Ϊ1.0f
	input.position.w = 1.0f;

	output.position = input.position;
	output.color = input.color;

	return output;
}