//--name = app/compute_reaction
// Reaction-diffusion simulation (Gray-Scott model).
// Reference: http://mrob.com/pub/comp/xmorphia/

float feed;
float kill;
float diffuseA;
float diffuseB;
float timestep;
uint  size;

StructuredBuffer  <float2> input  : register(t1);
RWStructuredBuffer<float2> output : register(u2);

[[vk::image_format("rgba8")]]
RWTexture2D <float4> out_tex: register(u3);

float2 GetLaplacian(int2 pos) {
	int2 offsets[9] = {
		int2(-1, 1), int2( 0, 1), int2(1, 1),
		int2(-1, 0), int2( 0, 0), int2(1, 0),
		int2(-1,-1), int2( 0,-1), int2(1,-1) };
	float weights[9] = {
		0,  1, 0,
		1, -4, 1,
		0,  1, 0 };

	float2 result = float2(0,0);
	for (int i = 0; i < 9; i++) {
		int2 p = pos + offsets[i];
		if (p.x < 0)          p.x = size - 1;
		if (p.y < 0)          p.y = size - 1;
		if (p.x >= (int)size) p.x = 0;
		if (p.y >= (int)size) p.y = 0;

		result += input[p.x+p.y*size] * weights[i];
	}
	return result;
}

[numthreads(8, 8, 1)]
void cs(uint3 dispatchThreadID : SV_DispatchThreadID) {
	uint   id  = dispatchThreadID.x + dispatchThreadID.y*size;
	float2 lap = GetLaplacian(dispatchThreadID.xy);

	float A = input[id].x;
	float B = input[id].y;
	float reactionTerm = A * B * B;

	// Vary kill spatially (distance from center) so different
	// regions settle into different reaction regimes, producing
	// more interesting patterns across the image.
	float dist   = abs(dispatchThreadID.x / (float)size - 0.5) * 2;
	float kill_v = lerp(kill * 0.8, kill * 1.2, dist);

	output[id] = float2(
		A + ((diffuseA * lap.x - reactionTerm + feed * (1.0 - A))    * timestep),
		B + ((diffuseB * lap.y + reactionTerm - (kill_v + feed) * B) * timestep)
	);

	// Color mapping
	float  value  = output[id].x*0.9-0.1;
	float4 color1 = float4(1.00, 0.83, 0.00,  0  );
	float4 color2 = float4(1.00, 0.35, 0.03, 0.1 );
	float4 color3 = float4(1.00, 0.17, 0.10, 0.2 );
	float4 color4 = float4(0.53, 0.05, 0.24, 0.3 );
	float4 color5 = float4(0.22, 0.05, 0.26, 0.4 );
	float4 color6 = float4(0.00, 0.04, 0.18, 1.0 );

	float3 col;
	float  a;
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
	out_tex[dispatchThreadID.xy] = float4(col, 1);
}
