#include "ComonType.i"

cbuffer ConstantBuffer : register( b0 )
{
    matrix World;
    matrix View;
    matrix Projection;
}

// ��ں���
PinputType ColorVsMain(VinputType input)
{
	PinputType output;

	// w��������CPU���룬��Ĭ������Ϊ1.0f
	input.position.w = 1.0f;

	//output.position = mul(input.position, transform);
	//output.position = input.position;

	output.position = mul( input.position, World );
    output.position = mul( output.position, View );
    output.position = mul( output.position, Projection );

	output.color = input.color;

	return output;
}