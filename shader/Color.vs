// 定义顶点着色器的输入结构体
struct VinputType
{
	float4 position : POSITION;	// 顶点位置
	float4 color : COLOR;	// 顶点颜色
};

// 定义顶点着色器的输出结构体（就是像素着色器的输入结构）
struct PinputType
{
	float4 position : SV_POSITION;	// 位置。SV_POSITION像素着色器内部变量
	float4 color : COLOR;	// 颜色
};

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