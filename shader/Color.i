// ���嶥����ɫ��������ṹ��
struct VinputType
{
	float4 position : POSITION;	// ����λ��
	float4 color : COLOR;	// ������ɫ
};

// ���嶥����ɫ��������ṹ��/������ɫ��������ṹ
struct PinputType
{
	float4 position : SV_POSITION;	// λ�á�SV_POSITION������ɫ���ڲ�����
	float4 color : COLOR;	// ��ɫ
};