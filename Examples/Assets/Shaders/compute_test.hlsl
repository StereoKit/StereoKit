// Reference and numbers are from here:
// http://mrob.com/pub/comp/xmorphia/

float feed;
float kill;
float diffuseA;
float diffuseB;
float timestep;
uint  size;

StructuredBuffer  <float2> input  : register(t0);
RWStructuredBuffer<float2> output : register(u0);
RWTexture2D       <float4> out_tex: register(u1);

float2 GetLaplacian(int2 pos) {
	int2 offsets[9] = { 
		int2(-1, 1), int2( 0, 1), int2(1, 1),
		int2(-1, 0), int2( 0, 0), int2(1, 0),
		int2(-1,-1), int2( 0,-1), int2(1,-1) };
	float weights[9] = { 
		0,  1, 0,
		1, -4, 1,
		0,  1, 0 };
	/*float weights[9] = { 
		0.05, 0.2, 0.05,
		0.2,  -1,  0.2,
		0.05, 0.2, 0.05 };*/

	float2 result = float2(0,0);
	for (int i = 0; i < 9; i++) {
		int2 p = pos + offsets[i];
		if (p.x < 0) p.x = size - 1;
		if (p.y < 0) p.y = size - 1;
		if (p.x >= (int)size) p.x = 0;
		if (p.y >= (int)size) p.y = 0;

		result += input[p.x+p.y*size] * weights[i];
	}
	return result;
}

[numthreads(32, 32, 1)]
void cs( uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint   id  = dispatchThreadID.x + dispatchThreadID.y*size;
	float2 lap = GetLaplacian(dispatchThreadID.xy);

	float A = input[id].x;
	float B = input[id].y;
	float reactionTerm = A * B * B;

	float px = (dispatchThreadID.x / (float)size);
	float py = (dispatchThreadID.y / (float)size);
	px = 1-abs(px - 0.5) * 2;
	py = 1-abs(py - 0.5) * 2;
	// Good!
	float feed2 = .02;
	float kill2 = lerp(.045, .065, abs(px-0.5) * 2);
	//float feed2 = feed;
	//float kill2 = kill;
	//float diffuseA2 = lerp(0.15, 0.25, px);
	//float diffuseB2 = lerp(0.05, 0.15, py);

	output[id] = float2(
		A + ((diffuseA * lap.x - reactionTerm + feed2 * (1.0 - A))   * timestep),
		B + ((diffuseB * lap.y + reactionTerm - (kill2 + feed2) * B)  * timestep)
	);

	float3 col;
	float  a;
	float  value  = output[id].x*0.9-0.1;
	float4 color1 = float4(1.00f, 0.83f, 0.00f,  0); //float4(1.00f, 0.92f, 0.00f,  0);
	float4 color2 = float4(1.00f, 0.35f, 0.03f, 0.1f);//float4(1.00f, 0.63f, 0.20f, 0.1f);
	float4 color3 = float4(1.00f, 0.17f, 0.10f, 0.2f);//float4(1.00f, 0.45f, 0.35f, 0.2f);
	float4 color4 = float4(0.53f, 0.05f, 0.24f, 0.3f);//float4(0.76f, 0.26f, 0.53f, 0.3f);
	float4 color5 = float4(0.22f, 0.05f, 0.26f, 0.4f);//float4(0.51f, 0.24f, 0.55f, 0.4f);
	float4 color6 = float4(0.00f, 0.04f, 0.18f, 1.0f);//float4(0.05f, 0.22f, 0.46f, 1.0f);
	if (value <= color1.a) {
		col = color1.rgb;
	} else if (value <= color2.a) {
		a = (value - color1.a)/(color2.a - color1.a);
		col = lerp(color1.rgb, color2.rgb, a);
	} else if (value <= color3.a) {
		a = (value - color2.a)/(color3.a - color2.a);
		col = lerp(color2.rgb, color3.rgb, a);
	} else if (value <= color4.a) {
		a = (value - color3.a)/(color4.a - color3.a);
		col = lerp(color3.rgb, color4.rgb, a);
	} else if (value <= color5.a) {
		a = (value - color4.a)/(color5.a - color4.a);
		col = lerp(color4.rgb, color5.rgb, a);
	} else if (value <= color6.a) {
		a = (value - color5.a)/(color6.a - color5.a);
		col = lerp(color5.rgb, color6.rgb, a);
	} else {
		col = color6.rgb;
	}
	out_tex[dispatchThreadID.xy] = float4(col,1);
}