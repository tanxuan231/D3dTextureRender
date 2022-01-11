#include "ComonType.i"

cbuffer MyCBuf
{
	matrix transform;
};

// ��ں���
PinputType ColorVsMain(VinputType input)
{
	PinputType output;

	// w��������CPU���룬��Ĭ������Ϊ1.0f
	input.position.w = 1.0f;

	output.position = mul(input.position, transform);
	output.color = input.color;

	return output;
}