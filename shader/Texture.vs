#include "ComonType.i"

// ��ں���
tPinputtype TextureVsMain(tVinputType input)
{
	tPinputtype output;

	// w��������CPU���룬��Ĭ������Ϊ1.0f
	input.position.w = 1.0f;

	output.position = input.position;
	output.tex = input.tex;

	return output;
}