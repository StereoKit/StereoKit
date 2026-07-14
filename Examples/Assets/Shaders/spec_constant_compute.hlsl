//--name = app/spec_constant_compute
// Test compute shader for specialization constants. Each thread writes a value
// derived entirely from the [[vk::constant_id]] constants into the output
// buffer, so overriding one (via Compute.SetInt/SetFloat/SetBool/SetUInt)
// changes the dispatched result without touching any bound resource.

[[vk::constant_id(0)]] const int   ADD_COUNT  = 3;
[[vk::constant_id(1)]] const float SCALE      = 2.0;
[[vk::constant_id(2)]] const bool  NEGATE     = false;
[[vk::constant_id(3)]] const uint  OFFSET     = 10;

RWStructuredBuffer<float> output : register(u0);

[numthreads(1, 1, 1)]
void cs(uint3 id : SV_DispatchThreadID) {
	float sum = 0;
	for (int i = 0; i < ADD_COUNT; i++)
		sum += 1.0;

	float result = sum * SCALE + (float)OFFSET;
	if (NEGATE)
		result = -result;

	output[id.x] = result;
}
