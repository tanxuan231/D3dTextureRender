#include "Color.i"

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