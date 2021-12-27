#include "Color.i"

// 入口函数
PinputType ColorVsMain(VinputType input)
{
	PinputType output;

	// w分量不从CPU传入，而默认设置为1.0f
	input.position.w = 1.0f;

	output.position = input.position;
	output.color = input.color;

	return output;
}