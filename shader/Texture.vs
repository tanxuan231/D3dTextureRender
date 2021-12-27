#include "ComonType.i"

// 入口函数
tPinputtype TextureVsMain(tVinputType input)
{
	tPinputtype output;

	// w分量不从CPU传入，而默认设置为1.0f
	input.position.w = 1.0f;

	output.position = input.position;
	output.tex = input.tex;

	return output;
}