// 颜色顶点结构体
// 定义顶点着色器的输入结构体
struct VinputType
{
	float4 position : POSITION;	// 顶点位置
	float4 color : COLOR1;		// 顶点颜色，1为自定义的语义索引
};

// 定义顶点着色器的输出结构体/像素着色器的输入结构
struct PinputType
{
	float4 position : SV_POSITION;	// 位置。SV_POSITION像素着色器内部变量
	float4 color : COLOR;			// 颜色
};

// 纹理顶点结构体
// 顶点着色器输入结构
struct tVinputType
{
	float4 position : myPOSITION;	// 顶点位置(x/y/z/w)
	float2 tex : myTEXCOORD;		// 纹理坐标(u/v)
};

// 像素着色器输入结构
struct tPinputtype
{
	float4 position : SV_POSITION;	// 位置。SV_POSITION像素着色器内部变量
	float2 tex : myTEXCOORD;			// 纹理坐标
};
