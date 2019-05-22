#define HLSL
#include "CommonRT.hlsl"

struct GSOut {
	float4 position : SV_POSITION;
	float3 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

struct VSOut {
	float4 position : SV_POSITION;
	float3 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 worldPos : WORLDPOS;
};

[maxvertexcount(3)]
void GSMain(triangle VSOut input[3] : SV_POSITION, inout TriangleStream< VSOut > output) {
	float3 v1 = input[1].worldPos.xyz - input[0].worldPos.xyz;
	float3 v2 = input[2].worldPos.xyz - input[0].worldPos.xyz;

	float3 norm = cross(v2, v1);
	norm = normalize(norm);

	for (uint i = 0; i < 3; i++) {
		VSOut element;
		element.position = input[i].position;
		element.normal = norm;
		element.texCoord = input[i].texCoord;
		element.worldPos = input[i].worldPos;
		output.Append(element);
	}
}
