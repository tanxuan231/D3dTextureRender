#include "ComonType.i"

cbuffer MyCBuf
{
	matrix transform;
};

// 入口函数
PinputType ColorVsMain(VinputType input)
{
	PinputType output;

	// w分量不从CPU传入，而默认设置为1.0f
	input.position.w = 1.0f;

	output.position = mul(input.position, transform);
	output.color = input.color;

	return output;
}